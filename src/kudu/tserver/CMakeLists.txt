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

#########################################
# tablet_copy_proto
#########################################

KRPC_GENERATE(
  TABLET_COPY_KRPC_SRCS TABLET_COPY_KRPC_HDRS TABLET_COPY_KRPC_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES tablet_copy.proto)
set(TABLET_COPY_KRPC_LIBS
  consensus_proto
  krpc
  protobuf
  rpc_header_proto
  tablet_proto
  wire_protocol_proto)
ADD_EXPORTABLE_LIBRARY(tablet_copy_proto
  SRCS ${TABLET_COPY_KRPC_SRCS}
  DEPS ${TABLET_COPY_KRPC_LIBS}
  NONLINK_DEPS ${TABLET_COPY_KRPC_TGTS})

#########################################
# tserver_proto
#########################################

PROTOBUF_GENERATE_CPP(
  TSERVER_PROTO_SRCS TSERVER_PROTO_HDRS TSERVER_PROTO_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES tserver.proto)
set(TSERVER_PROTO_LIBS
  kudu_common_proto
  krpc
  consensus_metadata_proto
  tablet_proto
  wire_protocol_proto)
ADD_EXPORTABLE_LIBRARY(tserver_proto
  SRCS ${TSERVER_PROTO_SRCS}
  DEPS ${TSERVER_PROTO_LIBS}
  NONLINK_DEPS ${TSERVER_PROTO_TGTS})

#########################################
# tserver_admin_proto
#########################################

KRPC_GENERATE(
  TSERVER_ADMIN_KRPC_SRCS TSERVER_ADMIN_KRPC_HDRS TSERVER_ADMIN_KRPC_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES tserver_admin.proto)
set(TSERVER_ADMIN_KRPC_LIBS
  krpc
  protobuf
  rpc_header_proto
  transactions_proto
  tserver_proto
  wire_protocol_proto)
ADD_EXPORTABLE_LIBRARY(tserver_admin_proto
  SRCS ${TSERVER_ADMIN_KRPC_SRCS}
  DEPS ${TSERVER_ADMIN_KRPC_LIBS}
  NONLINK_DEPS ${TSERVER_ADMIN_KRPC_TGTS})

#########################################
# tserver_service_proto
#########################################

KRPC_GENERATE(
  TSERVER_KRPC_SRCS TSERVER_KRPC_HDRS TSERVER_KRPC_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES tserver_service.proto)
set(TSERVER_KRPC_LIBS
  krpc
  kudu_common_proto
  protobuf
  rpc_header_proto
  tablet_copy_proto
  tserver_proto
  wire_protocol_proto)
ADD_EXPORTABLE_LIBRARY(tserver_service_proto
  SRCS ${TSERVER_KRPC_SRCS}
  DEPS ${TSERVER_KRPC_LIBS}
  NONLINK_DEPS ${TSERVER_KRPC_TGTS})

#########################################
# tserver
#########################################

set(TSERVER_SRCS
  heartbeater.cc
  scanner_metrics.cc
  scanners.cc
  tablet_copy_client.cc
  tablet_copy_service.cc
  tablet_copy_source_session.cc
  tablet_server.cc
  tablet_server_options.cc
  tablet_server_runner.cc
  tablet_service.cc
  ts_tablet_manager.cc
  tserver_path_handlers.cc
)

# The mini_tablet_server is used only for tests.
if (KUDU_CLI_TEST_TOOL_ENABLED)
  set(TSERVER_SRCS ${TSERVER_SRCS} mini_tablet_server.cc)
endif()

add_library(tserver ${TSERVER_SRCS})
target_link_libraries(tserver
  clock
  consensus
  consensus_proto
  krpc
  kserver
  log
  log_proto
  master_proto
  protobuf
  security
  server_process
  tablet
  tablet_copy_proto
  transactions
  tserver_admin_proto
  tserver_proto
  tserver_service_proto)

#########################################
# kudu-tserver
#########################################

add_executable(kudu-tserver tablet_server_main.cc)
target_link_libraries(kudu-tserver
  ${SANITIZER_OPTIONS_OVERRIDE}
  ${KRB5_REALM_OVERRIDE}
  tserver
  ${KUDU_BASE_LIBS})

option(KUDU_TSERVER_INSTALL "Whether to install the Kudu Tablet Server executable" ON)
if(KUDU_TSERVER_INSTALL)
  install(TARGETS kudu-tserver RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})
else()
  message(STATUS "Skipping install rule for the Kudu Tablet Server executable")
endif()

#########################################
# tserver_test_util
#########################################

if (NOT NO_TESTS)
  set(TSERVER_TEST_UTIL_SRCS
    tablet_server_test_util.cc
    tablet_server-test-base.cc
  )

  add_library(tserver_test_util ${TSERVER_TEST_UTIL_SRCS})
  target_link_libraries(tserver_test_util
    ${KUDU_MIN_TEST_LIBS}
    tserver)
endif()

#########################################
# Unit tests
#########################################

SET_KUDU_TEST_LINK_LIBS(
  kudu_curl_util
  tserver
  tserver_test_util)
ADD_KUDU_TEST(mini_tablet_server-test)
ADD_KUDU_TEST(tablet_copy_client-test)
ADD_KUDU_TEST(tablet_copy_source_session-test)
ADD_KUDU_TEST(tablet_copy_service-test)
ADD_KUDU_TEST(tablet_server-test PROCESSORS 3 NUM_SHARDS 4)
ADD_KUDU_TEST(tablet_server-stress-test RUN_SERIAL true)
ADD_KUDU_TEST(tablet_server_authorization-test NUM_SHARDS 2)
ADD_KUDU_TEST(scanners-test)
ADD_KUDU_TEST(ts_tablet_manager-test)
