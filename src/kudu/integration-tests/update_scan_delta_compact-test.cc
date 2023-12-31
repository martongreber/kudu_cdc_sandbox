// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <ostream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "kudu/client/callbacks.h"
#include "kudu/client/client.h"
#include "kudu/client/row_result.h"
#include "kudu/client/scan_batch.h"
#include "kudu/client/schema.h"
#include "kudu/client/shared_ptr.h" // IWYU pragma: keep
#include "kudu/client/write_op.h"
#include "kudu/common/partial_row.h"
#include "kudu/master/mini_master.h"
#include "kudu/mini-cluster/internal_mini_cluster.h"
#include "kudu/tserver/mini_tablet_server.h"
#include "kudu/util/async_util.h"
#include "kudu/util/countdown_latch.h"
#include "kudu/util/curl_util.h"
#include "kudu/util/faststring.h"
#include "kudu/util/monotime.h"
#include "kudu/util/net/sockaddr.h"
#include "kudu/util/status.h"
#include "kudu/util/stopwatch.h"
#include "kudu/util/test_macros.h"
#include "kudu/util/test_util.h"

DECLARE_int32(flush_threshold_mb);
DECLARE_int32(log_segment_size_mb);
DECLARE_int32(maintenance_manager_polling_interval_ms);
DEFINE_int32(mbs_for_flushes_and_rolls, 1, "How many MBs are needed to flush and roll");
DEFINE_int32(row_count, 2000, "How many rows will be used in this test for the base data");
DEFINE_int32(seconds_to_run, 4,
             "How long this test runs for, after inserting the base data, in seconds");

using kudu::client::KuduInsert;
using kudu::client::KuduClient;
using kudu::client::KuduClientBuilder;
using kudu::client::KuduColumnSchema;
using kudu::client::KuduRowResult;
using kudu::client::KuduScanBatch;
using kudu::client::KuduScanner;
using kudu::client::KuduSchema;
using kudu::client::KuduSchemaBuilder;
using kudu::client::KuduSession;
using kudu::client::KuduStatusCallback;
using kudu::client::KuduStatusMemberCallback;
using kudu::client::KuduTable;
using kudu::client::KuduTableCreator;
using kudu::client::KuduUpdate;
using kudu::client::sp::shared_ptr;
using kudu::cluster::InternalMiniCluster;
using kudu::cluster::InternalMiniClusterOptions;
using std::string;
using std::thread;
using std::unique_ptr;
using std::vector;

namespace kudu {
namespace tablet {

// This integration test tries to trigger all the update-related bits while also serving as a
// foundation for benchmarking. It first inserts 'row_count' rows and then starts two threads,
// one that continuously updates all the rows sequentially and one that scans them all, until
// it's been running for 'seconds_to_run'. It doesn't test for correctness, unless something
// FATALs.
class UpdateScanDeltaCompactionTest : public KuduTest {
 protected:
  UpdateScanDeltaCompactionTest() {
    KuduSchemaBuilder b;
    b.AddColumn("key")->Type(KuduColumnSchema::INT64)->NotNull()->PrimaryKey();
    b.AddColumn("string")->Type(KuduColumnSchema::STRING)->NotNull();
    b.AddColumn("int64")->Type(KuduColumnSchema::INT64)->NotNull();
    CHECK_OK(b.Build(&schema_));
  }

  void SetUp() override {
    KuduTest::SetUp();
  }

  void CreateTable() {
    NO_FATALS(InitCluster());
    unique_ptr<KuduTableCreator> table_creator(client_->NewTableCreator());
    ASSERT_OK(table_creator->table_name(kTableName)
             .schema(&schema_)
             .set_range_partition_columns({ "key" })
             .num_replicas(1)
             .Create());
    ASSERT_OK(client_->OpenTable(kTableName, &table_));
  }

  void TearDown() override {
    if (cluster_) {
      cluster_->Shutdown();
    }
    KuduTest::TearDown();
  }

  // Inserts row_count rows sequentially.
  void InsertBaseData();

  // Starts the update and scan threads then stops them after seconds_to_run.
  void RunThreads();

 private:
  enum {
    kKeyCol,
    kStrCol,
    kInt64Col
  };
  static const char* const kTableName;

  void InitCluster() {
    // Start mini-cluster with 1 tserver.
    cluster_.reset(new InternalMiniCluster(env_, InternalMiniClusterOptions()));
    ASSERT_OK(cluster_->Start());
    KuduClientBuilder client_builder;
    client_builder.add_master_server_addr(
        cluster_->mini_master()->bound_rpc_addr_str());
    ASSERT_OK(client_builder.Build(&client_));
  }

  shared_ptr<KuduSession> CreateSession() {
    shared_ptr<KuduSession> session = client_->NewSession();
    session->SetTimeoutMillis(30000);
    CHECK_OK(session->SetFlushMode(KuduSession::MANUAL_FLUSH));
    return session;
  }

  // Continuously updates the existing data until 'stop_latch' drops to 0.
  void UpdateRows(CountDownLatch* stop_latch);

  // Continuously scans the data until 'stop_latch' drops to 0.
  void ScanRows(CountDownLatch* stop_latch) const;

  // Continuously fetch various web pages on the TS.
  void CurlWebPages(CountDownLatch* stop_latch) const;

  // Sets the passed values on the row.
  // TODO randomize the string column.
  void MakeRow(int64_t key, int64_t val, KuduPartialRow* row) const;

  // If 'key' is a multiple of kSessionBatchSize, it uses 'last_s' to wait for the previous batch
  // to finish and then flushes the current one.
  Status WaitForLastBatchAndFlush(int64_t key,
                                  Synchronizer* last_s,
                                  KuduStatusCallback* last_s_cb,
                                  shared_ptr<KuduSession> session);

  KuduSchema schema_;
  std::shared_ptr<InternalMiniCluster> cluster_;
  shared_ptr<KuduTable> table_;
  shared_ptr<KuduClient> client_;
};

const char* const UpdateScanDeltaCompactionTest::kTableName = "update-scan-delta-compact-tbl";
const int kSessionBatchSize = 1000;

TEST_F(UpdateScanDeltaCompactionTest, TestAll) {
  OverrideFlagForSlowTests("seconds_to_run", "100");
  OverrideFlagForSlowTests("row_count", "1000000");
  OverrideFlagForSlowTests("mbs_for_flushes_and_rolls", "8");
  // Setting this high enough that we see the effects of flushes and compactions.
  OverrideFlagForSlowTests("maintenance_manager_polling_interval_ms", "2000");
  FLAGS_flush_threshold_mb = FLAGS_mbs_for_flushes_and_rolls;
  FLAGS_log_segment_size_mb = FLAGS_mbs_for_flushes_and_rolls;
  if (!AllowSlowTests()) {
    // Make it run more often since it's not a long test.
    FLAGS_maintenance_manager_polling_interval_ms = 50;
  }

  NO_FATALS(CreateTable());
  NO_FATALS(InsertBaseData());
  NO_FATALS(RunThreads());
}

void UpdateScanDeltaCompactionTest::InsertBaseData() {
  shared_ptr<KuduSession> session = CreateSession();
  Synchronizer last_s;
  KuduStatusMemberCallback<Synchronizer> last_s_cb(&last_s,
                                                   &Synchronizer::StatusCB);
  last_s_cb.Run(Status::OK());

  LOG_TIMING(INFO, "Insert") {
    for (int64_t key = 0; key < FLAGS_row_count; key++) {
      unique_ptr<KuduInsert> insert(table_->NewInsert());
      MakeRow(key, 0, insert->mutable_row());
      ASSERT_OK(session->Apply(insert.release()));
      ASSERT_OK(WaitForLastBatchAndFlush(key, &last_s, &last_s_cb, session));
    }
    ASSERT_OK(WaitForLastBatchAndFlush(kSessionBatchSize, &last_s, &last_s_cb, session));
    ASSERT_OK(last_s.Wait());
  }
}

void UpdateScanDeltaCompactionTest::RunThreads() {
  vector<thread> threads;
  CountDownLatch stop_latch(1);

  threads.emplace_back([this, &stop_latch]() { this->UpdateRows(&stop_latch); });
  threads.emplace_back([this, &stop_latch]() { this->ScanRows(&stop_latch); });
  threads.emplace_back([this, &stop_latch]() { this->CurlWebPages(&stop_latch); });

  SleepFor(MonoDelta::FromSeconds(FLAGS_seconds_to_run * 1.0));
  stop_latch.CountDown();
  for (auto& t : threads) {
    t.join();
  }
}

void UpdateScanDeltaCompactionTest::UpdateRows(CountDownLatch* stop_latch) {
  shared_ptr<KuduSession> session = CreateSession();
  Synchronizer last_s;
  KuduStatusMemberCallback<Synchronizer> last_s_cb(&last_s,
                                                   &Synchronizer::StatusCB);

  for (int64_t iteration = 1; stop_latch->count() > 0; iteration++) {
    last_s_cb.Run(Status::OK());
    LOG_TIMING(INFO, "Update") {
      for (int64_t key = 0; key < FLAGS_row_count && stop_latch->count() > 0; key++) {
        unique_ptr<KuduUpdate> update(table_->NewUpdate());
        MakeRow(key, iteration, update->mutable_row());
        CHECK_OK(session->Apply(update.release()));
        CHECK_OK(WaitForLastBatchAndFlush(key, &last_s, &last_s_cb, session));
      }
      CHECK_OK(WaitForLastBatchAndFlush(kSessionBatchSize, &last_s, &last_s_cb, session));
      CHECK_OK(last_s.Wait());
    }
  }
}

void UpdateScanDeltaCompactionTest::ScanRows(CountDownLatch* stop_latch) const {
  while (stop_latch->count() > 0) {
    KuduScanner scanner(table_.get());
    // Sometimes use fault-tolerant scans, to get more coverage of the
    // MergeIterator code paths.
    if (rand() % 2 == 1) {
      CHECK_OK(scanner.SetFaultTolerant());
    }
    LOG_TIMING(INFO, "Scan") {
      CHECK_OK(scanner.Open());
      KuduScanBatch batch;
      while (scanner.HasMoreRows()) {
        CHECK_OK(scanner.NextBatch(&batch));
      }
    }
  }
}

void UpdateScanDeltaCompactionTest::CurlWebPages(CountDownLatch* stop_latch) const {
  vector<string> urls;
  string base_url = cluster_->mini_tablet_server(0)->bound_http_addr().ToString();
  urls.push_back(base_url + "/scans");
  urls.push_back(base_url + "/transactions");

  EasyCurl curl;
  faststring dst;
  while (stop_latch->count() > 0) {
    for (const string& url : urls) {
      VLOG(1) << "Curling URL " << url;
      Status status = curl.FetchURL(url, &dst);
      if (status.ok()) {
        CHECK_GT(dst.length(), 0);
      }
    }
  }
}

void UpdateScanDeltaCompactionTest::MakeRow(int64_t key,
                                            int64_t val,
                                            KuduPartialRow* row) const {
  CHECK_OK(row->SetInt64(kKeyCol, key));
  CHECK_OK(row->SetStringCopy(kStrCol, "TODO random string"));
  CHECK_OK(row->SetInt64(kInt64Col, val));
}

Status UpdateScanDeltaCompactionTest::WaitForLastBatchAndFlush(int64_t key,
                                                               Synchronizer* last_s,
                                                               KuduStatusCallback* last_s_cb,
                                                               shared_ptr<KuduSession> session) {
  if (key % kSessionBatchSize == 0) {
    RETURN_NOT_OK(last_s->Wait());
    last_s->Reset();
    session->FlushAsync(last_s_cb);
  }
  return Status::OK();
}

} // namespace tablet
} // namespace kudu
