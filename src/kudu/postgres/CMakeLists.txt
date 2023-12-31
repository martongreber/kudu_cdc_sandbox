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
# mini_postgres
#######################################
# The mini_postgres is used only for tests.
if (KUDU_CLI_TEST_TOOL_ENABLED)
  execute_process(COMMAND ln -nsf
    "${THIRDPARTY_DIR}/installed/common/bin"
    "${EXECUTABLE_OUTPUT_PATH}/postgres")
  execute_process(COMMAND ln -nsf
    "${THIRDPARTY_DIR}/installed/common/lib"
    "${EXECUTABLE_OUTPUT_PATH}/postgres-lib")
  execute_process(COMMAND ln -nsf
    "${THIRDPARTY_DIR}/installed/common/share/postgresql"
    "${EXECUTABLE_OUTPUT_PATH}/postgres-share")
  execute_process(COMMAND ln -nsf
    "${THIRDPARTY_DIR}/installed/common/opt/jdbc/postgresql.jar"
    "${EXECUTABLE_OUTPUT_PATH}/postgresql.jar")

  add_library(mini_postgres
    mini_postgres.cc)
  target_link_libraries(mini_postgres
    gutil
    kudu_test_util
    kudu_util
  )
endif()

#######################################
# Unit tests
#######################################

SET_KUDU_TEST_LINK_LIBS(mini_postgres)

ADD_KUDU_TEST(mini_postgres-test)
