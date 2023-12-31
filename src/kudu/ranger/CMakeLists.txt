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

#######################################
# ranger_proto
#######################################

PROTOBUF_GENERATE_CPP(
  RANGER_PROTO_SRCS RANGER_PROTO_HDRS RANGER_PROTO_TGTS
  SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..
  BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../..
  PROTO_FILES ranger.proto)

add_library(ranger_proto
  ${RANGER_PROTO_SRCS}
  ${RANGER_PROTO_HDRS})
target_link_libraries(ranger_proto
  protobuf
)

#######################################
# kudu_ranger
#######################################

set(RANGER_SRCS
  ranger_client.cc)
set(RANGER_DEPS
  gflags
  kudu_subprocess
  ranger_proto
  security)

add_library(kudu_ranger ${RANGER_SRCS})
target_link_libraries(kudu_ranger ${RANGER_DEPS})

#######################################
# mini_ranger
#######################################

# The mini_ranger is used only for tests.
if (KUDU_CLI_TEST_TOOL_ENABLED)
  execute_process(COMMAND ln -nsf
    "${THIRDPARTY_DIR}/installed/common/opt/ranger"
    "${EXECUTABLE_OUTPUT_PATH}/ranger-home")
  execute_process(COMMAND ln -nsf
    "${THIRDPARTY_DIR}/installed/common/opt/hadoop"
    "${EXECUTABLE_OUTPUT_PATH}/hadoop-home")
  execute_process(COMMAND ln -nsf
    "${JAVA_HOME}"
    "${EXECUTABLE_OUTPUT_PATH}/java-home")

  set(MINI_RANGER_SRCS
    mini_ranger.cc)
  set(MINI_RANGER_DEPS
    kudu_curl_util
    kudu_test_util
    kudu_ranger
    kudu_util
    mini_postgres)
  add_library(mini_ranger ${MINI_RANGER_SRCS})
  target_link_libraries(mini_ranger ${MINI_RANGER_DEPS})
endif()

#######################################
# Unit tests
#######################################

SET_KUDU_TEST_LINK_LIBS(
  kudu_ranger
  mini_postgres
  mini_ranger)

ADD_KUDU_TEST(mini_ranger-test)
ADD_KUDU_TEST(ranger_client-test)
