# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

PROTOBUF_GENERATE_CPP(
  COMMON_PROTO_SRCS COMMON_PROTO_HDRS COMMON_PROTO_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES common.proto)
ADD_EXPORTABLE_LIBRARY(kudu_common_proto
  SRCS ${COMMON_PROTO_SRCS}
  DEPS block_bloom_filter_proto hash_proto protobuf row_operations_proto util_compression_proto
  NONLINK_DEPS ${COMMON_PROTO_TGTS})

PROTOBUF_GENERATE_CPP(
  ROW_OPERATIONS_PROTO_SRCS ROW_OPERATIONS_PROTO_HDRS ROW_OPERATIONS_PROTO_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES row_operations.proto)
ADD_EXPORTABLE_LIBRARY(row_operations_proto
  SRCS ${ROW_OPERATIONS_PROTO_SRCS}
  DEPS pb_util_proto protobuf
  NONLINK_DEPS ${ROW_OPERATIONS_PROTO_TGTS})

PROTOBUF_GENERATE_CPP(
  WIRE_PROTOCOL_PROTO_SRCS WIRE_PROTOCOL_PROTO_HDRS WIRE_PROTOCOL_PROTO_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES wire_protocol.proto)
set(WIRE_PROTOCOL_PROTO_LIBS
  kudu_common_proto
  consensus_metadata_proto
  protobuf)
ADD_EXPORTABLE_LIBRARY(wire_protocol_proto
  SRCS ${WIRE_PROTOCOL_PROTO_SRCS}
  DEPS ${WIRE_PROTOCOL_PROTO_LIBS}
  NONLINK_DEPS ${WIRE_PROTOCOL_PROTO_TGTS})

set(COMMON_SRCS
  columnblock.cc
  column_predicate.cc
  columnar_serialization.cc
  encoded_key.cc
  generic_iterators.cc
  id_mapping.cc
  iterator_stats.cc
  key_encoder.cc
  key_range.cc
  key_util.cc
  partial_row.cc
  partition.cc
  partition_pruner.cc
  predicate_effectiveness.cc
  rowblock.cc
  row_changelist.cc
  row_operations.cc
  scan_spec.cc
  schema.cc
  table_util.cc
  timestamp.cc
  txn_id.cc
  types.cc
  wire_protocol.cc
  zp7.cc)

# Workaround for clang bug https://llvm.org/bugs/show_bug.cgi?id=23757
# in which it incorrectly optimizes key_util.cc and causes incorrect results.
if ("${COMPILER_FAMILY}" STREQUAL "clang")
  set_source_files_properties(key_util.cc PROPERTIES COMPILE_FLAGS -fwrapv)
endif()

set(COMMON_LIBS
  consensus_metadata_proto
  gutil
  kudu_common_proto
  kudu_util
  row_operations_proto
  wire_protocol_proto)

ADD_EXPORTABLE_LIBRARY(kudu_common
  SRCS ${COMMON_SRCS}
  DEPS ${COMMON_LIBS})

#######################################
# Unit tests
#######################################

SET_KUDU_TEST_LINK_LIBS(kudu_common)
ADD_KUDU_TEST(columnar_serialization-test)
ADD_KUDU_TEST(columnblock-test)
ADD_KUDU_TEST(column_predicate-test NUM_SHARDS 4)
ADD_KUDU_TEST(encoded_key-test)
ADD_KUDU_TEST(generic_iterators-test)
ADD_KUDU_TEST(id_mapping-test)
ADD_KUDU_TEST(key_util-test)
ADD_KUDU_TEST(partial_row-test)
ADD_KUDU_TEST(partition-test)
ADD_KUDU_TEST(partition_pruner-test)
ADD_KUDU_TEST(rowblock-test)
ADD_KUDU_TEST(row_changelist-test)
ADD_KUDU_TEST(row_operations-test)
ADD_KUDU_TEST(scan_spec-test)
ADD_KUDU_TEST(schema-test)
ADD_KUDU_TEST(table_util-test)
ADD_KUDU_TEST(txn_id-test)
ADD_KUDU_TEST(types-test)
ADD_KUDU_TEST(wire_protocol-test NUM_SHARDS 10)
