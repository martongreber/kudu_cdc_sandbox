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

#include "kudu/rpc/connection_id.h"

#include <cstddef>
#include <utility>

#include <boost/container_hash/extensions.hpp>
#include <glog/logging.h>

#include "kudu/gutil/strings/substitute.h"

using std::string;

namespace kudu {
namespace rpc {

ConnectionId::ConnectionId() {}

ConnectionId::ConnectionId(const Sockaddr& remote,
                           string hostname,
                           UserCredentials user_credentials)
    : remote_(remote),
      hostname_(std::move(hostname)),
      user_credentials_(std::move(user_credentials)) {
  CHECK(!hostname_.empty());
}

void ConnectionId::set_user_credentials(UserCredentials user_credentials) {
  DCHECK(user_credentials.has_real_user());
  user_credentials_ = std::move(user_credentials);
}

void ConnectionId::set_network_plane(string network_plane) {
  network_plane_ = std::move(network_plane);
}

string ConnectionId::ToString() const {
  string remote;
  if (remote_.is_initialized() && remote_.is_ip() && hostname_ != remote_.host()) {
    remote = strings::Substitute("$0 ($1)", remote_.ToString(), hostname_);
  } else {
    remote = remote_.ToString();
  }

  string network_plane;
  if (!network_plane_.empty()) {
    network_plane = strings::Substitute(", network_plane=$0", network_plane_);
  }

  return strings::Substitute("{remote=$0, user_credentials=$1$2}",
                             remote,
                             user_credentials_.ToString(),
                             network_plane);
}

size_t ConnectionId::HashCode() const {
  size_t seed = 0;
  boost::hash_combine(seed, remote_.HashCode());
  boost::hash_combine(seed, hostname_);
  boost::hash_combine(seed, user_credentials_.HashCode());
  boost::hash_combine(seed, network_plane_);
  return seed;
}

bool ConnectionId::operator==(const ConnectionId& other) const {
  return remote() == other.remote() &&
      hostname_ == other.hostname_ &&
      user_credentials() == other.user_credentials() &&
      network_plane_ == other.network_plane_;
}

size_t ConnectionIdHash::operator() (const ConnectionId& conn_id) const {
  return conn_id.HashCode();
}

bool ConnectionIdEqual::operator() (const ConnectionId& cid1, const ConnectionId& cid2) const {
  return cid1 == cid2;
}

} // namespace rpc
} // namespace kudu
