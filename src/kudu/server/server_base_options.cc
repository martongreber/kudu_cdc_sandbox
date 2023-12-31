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

#include "kudu/server/server_base_options.h"

#include <gflags/gflags.h>

#include "kudu/gutil/macros.h"
#include "kudu/util/env.h"
#include "kudu/util/flag_tags.h"

DEFINE_string(server_dump_info_path, "",
              "Path into which the server information will be "
              "dumped after startup. The dumped data is described by "
              "ServerStatusPB in server_base.proto. The dump format is "
              "determined by --server_dump_info_format");
DEFINE_string(server_dump_info_format, "json",
              "Format for --server_dump_info_path. This may be either "
              "'pb' or 'json'.");
TAG_FLAG(server_dump_info_path, hidden);
TAG_FLAG(server_dump_info_format, hidden);

DEFINE_int32(metrics_log_interval_ms, 60000,
             "Interval (in milliseconds) at which the server will dump its "
             "metrics to a local log file. The log files are located in the same "
             "directory as specified by the -log_dir flag. If this is not a positive "
             "value, then metrics logging will be disabled.");
TAG_FLAG(metrics_log_interval_ms, advanced);

DEFINE_string(test_server_key, "",
              "Server key in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_server_key, hidden);


DEFINE_string(test_server_key_iv, "",
              "Server key IV in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_server_key_iv, hidden);


DEFINE_string(test_server_key_version, "",
              "Server key version in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_server_key_version, hidden);

DEFINE_string(test_tenant_name, "",
              "Tenant name in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_tenant_name, hidden);

DEFINE_string(test_tenant_id, "",
              "Tenant id in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_tenant_id, hidden);

DEFINE_string(test_tenant_key, "",
              "Tenant key in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_tenant_key, hidden);

DEFINE_string(test_tenant_key_iv, "",
              "Tenant key IV in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_tenant_key_iv, hidden);


DEFINE_string(test_tenant_key_version, "",
              "Tenant key version in plain-text to be persisted into the instance file. "
              "It is only used when creating the file system, it's disregarded on "
              "consecutive startups. It should only be used in tests.");
TAG_FLAG(test_tenant_key_version, hidden);

namespace kudu {
namespace server {

ServerKeyInfo::ServerKeyInfo()
  : server_key(FLAGS_test_server_key),
    server_key_iv(FLAGS_test_server_key_iv),
    server_key_version(FLAGS_test_server_key_version) {
}

TenantKeyInfo::TenantKeyInfo()
  : tenant_name(FLAGS_test_tenant_name),
    tenant_id(FLAGS_test_tenant_id),
    tenant_key(FLAGS_test_tenant_key),
    tenant_key_iv(FLAGS_test_tenant_key_iv),
    tenant_key_version(FLAGS_test_tenant_key_version) {
}

ServerBaseOptions::ServerBaseOptions()
  : env(Env::Default()),
    dump_info_path(FLAGS_server_dump_info_path),
    dump_info_format(FLAGS_server_dump_info_format),
    metrics_log_interval_ms(FLAGS_metrics_log_interval_ms) {
}

} // namespace server
} // namespace kudu
