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
#ifndef KUDU_UTIL_MONOTIME_H
#define KUDU_UTIL_MONOTIME_H

// NOTE: using stdint.h instead of cstdint because this file is supposed
//       to be processed by a compiler lacking C++11 support.
#include <stdint.h>

#include <iosfwd>
#include <string>

#ifdef KUDU_HEADERS_NO_STUBS
#include <gtest/gtest_prod.h>

#include "kudu/gutil/port.h"
#else
// This is a poor module interdependency, but the stubs are header-only and
// it's only for exported header builds, so we'll make an exception.
#include "kudu/client/stubs.h"
#endif

#include "kudu/util/kudu_export.h"


// The 'noexcept' specifier is recognized by a C++11-capable compiler, but this
// file is targeted to compile by C++-98 compiler as well. As it turns out,
// adding 'noexcept' doesn't affect the generated symbols in the exported
// MonoTime class, so it's safe to turn it on when compiling in the C++11 mode.
// The 'noexcept' specified is useful in cases when wrapping MonoTime into
// std::atomic<> and the standard C++ library explicitly requires that.
#ifdef LANG_CXX11
#define KUDU_MONOTIME_NOEXCEPT noexcept
#else
#define KUDU_MONOTIME_NOEXCEPT
#endif // #ifdef LANG_CXX11 ... #else ...

namespace kudu {

/// @brief A representation of a time interval.
///
/// The MonoDelta class represents an elapsed duration of time -- i.e.
/// the delta between two MonoTime instances.
class KUDU_EXPORT MonoDelta {
 public:
  /// @name Converters from seconds representation (and ubiquitous SI prefixes).
  ///@{

  /// @param [in] seconds
  ///   Time interval representation in seconds (with ubiquitous SI prefixes).
  /// @return The resulting MonoDelta object initialized in accordance with
  ///   the specified parameter.
  static MonoDelta FromSeconds(double seconds);

  /// @param [in] ms
  ///   Time interval representation in seconds (with ubiquitous SI prefixes).
  /// @return The resulting MonoDelta object initialized in accordance with
  ///   the specified parameter.
  static MonoDelta FromMilliseconds(int64_t ms);

  /// @param [in] us
  ///   Time interval representation in seconds (with ubiquitous SI prefixes).
  /// @return The resulting MonoDelta object initialized in accordance with
  ///   the specified parameter.
  static MonoDelta FromMicroseconds(int64_t us);

  /// @param [in] ns
  ///   Time interval representation in seconds (with ubiquitous SI prefixes).
  /// @return The resulting MonoDelta object initialized in accordance with
  ///   the specified parameter.
  static MonoDelta FromNanoseconds(int64_t ns);
  ///@}

  /// Build a MonoDelta object.
  ///
  /// @note A MonoDelta instance built with the this default constructor is
  ///   "uninitialized" and may not be used for any operation.
  MonoDelta();

  /// @return @c true iff this object is initialized.
  bool Initialized() const;

  /// @deprecated Use operator<(const MonoDelta&, const MonoDelta&) instead.
  ///
  /// Check whether this time interval is shorter than the specified one.
  ///
  /// @param [in] rhs
  ///   A time interval for comparison.
  /// @return @c true iff this time interval is strictly shorter
  ///   than the specified one.
  bool LessThan(const MonoDelta& rhs) const
      ATTRIBUTE_DEPRECATED("use operator<(const MonoDelta&, const MonoDelta&) instead");

  /// @deprecated Use operator>(const MonoDelta&, const MonoDelta&) instead.
  ///
  /// Check whether this time interval is longer than the specified one.
  ///
  /// @param [in] rhs
  ///   A time interval for comparison.
  /// @return @c true iff this time interval is strictly longer
  ///   than the specified one.
  bool MoreThan(const MonoDelta& rhs) const
      ATTRIBUTE_DEPRECATED("use operator>(const MonoDelta&, const MonoDelta&) instead");

  /// @deprecated Use operator==(const MonoDelta&, const MonoDelta&) instead.
  ///
  /// Check whether this time interval has the same duration
  /// as the specified one.
  ///
  /// @param [in] rhs
  ///   A time interval for comparison.
  /// @return @c true iff this time interval has the same duration as the
  ///   the specified one.
  bool Equals(const MonoDelta& rhs) const ATTRIBUTE_DEPRECATED(
      "use operator==(const MonoDelta&, const MonoDelta&) instead");

  /// @return String representation of this interval's duration (in seconds).
  std::string ToString() const;

  /// @name Converters into seconds representation (and ubiquitous SI prefixes).
  ///@{

  /// @return Representation of the time interval in appropriate SI units.
  double ToSeconds() const;
  int64_t ToMilliseconds() const;
  int64_t ToMicroseconds() const;
  int64_t ToNanoseconds() const;
  ///@}

  /// Represent this time interval as a timeval structure, with microsecond
  /// accuracy.
  ///
  /// @param [out] tv
  ///   Placeholder for the result value.
  void ToTimeVal(struct timeval* tv) const;

  /// Represent this time interval as a timespec structure, with nanosecond
  /// accuracy.
  ///
  /// @param [out] ts
  ///   Placeholder for the result value.
  void ToTimeSpec(struct timespec* ts) const;

  /// Convert a nanosecond value to a timespec.
  ///
  /// @param [in] nanos
  ///   Representation of a relative point in time in nanoseconds.
  /// @param [out] ts
  ///   Placeholder for the resulting timespec representation.
  static void NanosToTimeSpec(int64_t nanos, struct timespec* ts);

  /// @name Syntactic sugar: increment/decrement operators for MonoDelta.
  ///@{

  /// Add a delta to current time interval.
  ///
  /// @param [in] delta
  ///   The delta to add.
  /// @return Reference to the modified object.
  MonoDelta& operator+=(const MonoDelta& delta);

  /// Substract a delta from current time interval.
  ///
  /// @param [in] delta
  ///   The delta to substract.
  /// @return Reference to the modified object.
  MonoDelta& operator-=(const MonoDelta& delta);
  ///@}

 private:
  static const int64_t kUninitialized;

  friend class MonoTime;

  friend MonoDelta operator-(const class MonoTime&, const class MonoTime&);
  friend MonoDelta operator-(const MonoDelta&, const MonoDelta&);
  friend MonoDelta operator+(const MonoDelta&, const MonoDelta&);
  friend class MonoTime operator-(const class MonoTime&, const class MonoDelta&);
  friend class MonoTime operator+(const class MonoTime&, const class MonoDelta&);
  friend bool operator==(const MonoDelta&, const MonoDelta&);
  friend bool operator<(const MonoDelta&, const MonoDelta&);
  friend bool operator>(const MonoDelta&, const MonoDelta&);

  FRIEND_TEST(TestMonoTime, TestDeltaConversions);

  explicit MonoDelta(int64_t delta);
  int64_t nano_delta_;
};

/// @brief Representation of a particular point in time.
///
/// The MonoTime class represents a particular point in time,
/// relative to some fixed but unspecified reference point.
///
/// This time is monotonic, meaning that if the user changes his or her system
/// clock, the monotime does not change.
class KUDU_EXPORT MonoTime {
 public:
  /// @name Conversion constants for ubiquitous time units.
  ///@{

  /// Nanoseconds per second
  static const int64_t kNanosecondsPerSecond = 1000000000L;

  /// Nanoseconds per millisecond
  static const int64_t kNanosecondsPerMillisecond = 1000000L;

  /// Nanoseconds per microseconds
  static const int64_t kNanosecondsPerMicrosecond = 1000L;

  /// Microseconds per second
  static const int64_t kMicrosecondsPerSecond = 1000000L;
  ///@}

  /// Get current time in MonoTime representation.
  ///
  /// @return Time specification for the moment of the method's invocation.
  static MonoTime Now();

  /// @return MonoTime equal to farthest possible time into the future.
  static MonoTime Max();

  /// @return MonoTime equal to farthest possible time into the past.
  static MonoTime Min();

  /// @deprecated Use std::min() instead.
  ///
  /// Select the earliest between the specified time points.
  ///
  /// @param [in] a
  ///   The first MonoTime object to select from.
  /// @param [in] b
  ///   The second MonoTime object to select from.
  /// @return The earliest (minimum) of the two monotimes.
  static const MonoTime& Earliest(const MonoTime& a, const MonoTime& b)
      ATTRIBUTE_DEPRECATED("use std::min() instead");

  /// Build a MonoTime object. The resulting object is not initialized
  /// and not ready to use.
  MonoTime() KUDU_MONOTIME_NOEXCEPT;

  /// @return @c true iff the object is initialized.
  bool Initialized() const;

  /// @deprecated Use @c kudu::operator-(const MonoTime&, const MonoTime&)
  ///   instead.
  ///
  /// Compute time interval between the point in time specified by this
  /// and the specified object.
  ///
  /// @param [in] rhs
  ///   The object that corresponds to the left boundary of the time interval,
  ///   where this object corresponds to the right boundary of the interval.
  /// @return The resulting time interval represented as a MonoDelta object.
  MonoDelta GetDeltaSince(const MonoTime& rhs) const ATTRIBUTE_DEPRECATED(
      "use kudu::operator-(const MonoTime&, const MonoTime&) instead");

  /// @deprecated Use @c operator+=(const MonoDelta&) instead.
  ///
  /// Advance this object's time specification by the specified interval.
  ///
  /// @param [in] delta
  ///   The time interval to add.
  void AddDelta(const MonoDelta& delta) ATTRIBUTE_DEPRECATED(
      "use operator+=(const MonoDelta&) instead");

  /// @deprecated Use @c operator<(const MonoTime&, const MonoTime&) instead.
  ///
  /// Check whether the point in time specified by this object is earlier
  /// than the specified one.
  ///
  /// @param [in] rhs
  ///   The other MonoTime object to compare with.
  /// @return @c true iff the point in time represented by this MonoTime object
  ///   is earlier then the point in time represented by the parameter.
  bool ComesBefore(const MonoTime& rhs) const ATTRIBUTE_DEPRECATED(
      "use operator<(const MonoTime&, const MonoTime&) instead");

  /// @return String representation of the object (in seconds).
  std::string ToString() const;

  /// Represent this point in time as a timespec structure, with nanosecond
  /// accuracy.
  ///
  /// @param [out] ts
  ///   Placeholder for the result value.
  void ToTimeSpec(struct timespec* ts) const;

  /// @deprecated Use operator==(const MonoTime&, const MonoTime&) instead.
  ///
  /// Check whether this object represents the same point in time as the other.
  ///
  /// @param [in] other
  ///   The other MonoTime object to compare.
  /// @return @c true iff the point in time represented by this MonoTime object
  ///   is the same as the one represented by the other.
  bool Equals(const MonoTime& other) const ATTRIBUTE_DEPRECATED(
      "use operator==(const MonoTime&, const MonoTime&) instead");

  /// Add a delta to the point in time represented by the object.
  ///
  /// @param [in] delta
  ///   The delta to add.
  /// @return Reference to the modified object.
  MonoTime& operator+=(const MonoDelta& delta);

  /// Substract a delta from the point in time represented by the object.
  ///
  /// @param [in] delta
  ///   The delta to substract.
  /// @return Reference to the modified object.
  MonoTime& operator-=(const MonoDelta& delta);

 private:
  friend class MonoDelta;
  friend MonoDelta operator-(const MonoTime&, const MonoTime&);
  friend MonoTime operator-(const MonoTime&, const MonoDelta&);
  friend MonoTime operator+(const MonoTime&, const MonoDelta&);
  friend bool operator==(const MonoTime&, const MonoTime&);
  friend bool operator<(const MonoTime&, const MonoTime&);

  FRIEND_TEST(TestMonoTime, TestTimeSpec);
  FRIEND_TEST(TestMonoTime, TestDeltaConversions);

  explicit MonoTime(const struct timespec& ts) KUDU_MONOTIME_NOEXCEPT;
  explicit MonoTime(int64_t nanos) KUDU_MONOTIME_NOEXCEPT;
  double ToSeconds() const;
  int64_t nanos_;
};

/// Sleep for an interval specified by a MonoDelta instance.
///
/// This is preferred over sleep(3), usleep(3), and nanosleep(3).
/// It's less prone to mixups with units since it uses the MonoDelta for
/// interval specification.
/// Besides, it ignores signals/EINTR, so will reliably sleep at least for the
/// MonoDelta duration.
///
/// @param [in] delta
///   The time interval to sleep for.
void KUDU_EXPORT SleepFor(const MonoDelta& delta);

/// @name Syntactic sugar: binary operators for MonoDelta.
///@{

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is equal
///   to the time interval represented by @c rhs.
bool KUDU_EXPORT operator==(const MonoDelta& lhs, const MonoDelta& rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is not equal
///   to the time interval represented by @c rhs.
bool KUDU_EXPORT operator!=(const MonoDelta& lhs, const MonoDelta& rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is shorter
///   than the time interval represented by @c rhs.
bool KUDU_EXPORT operator<(const MonoDelta& lhs, const MonoDelta& rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is shorter
///   than or equal to the time interval represented by @c rhs.
bool KUDU_EXPORT operator<=(const MonoDelta& lhs, const MonoDelta& rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is longer
///   than the time interval represented by @c rhs.
bool KUDU_EXPORT operator>(const MonoDelta& lhs, const MonoDelta& rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is longer
///   than or equal to the time interval represented by @c rhs.
bool KUDU_EXPORT operator>=(const MonoDelta& lhs, const MonoDelta& rhs);

/// @param [in] lhs
///   A time interval for substraction: the left-hand operand.
/// @param [in] rhs
///   A time interval for substraction: the right-hand operand.
/// @return A MonoDelta object representing the result time interval.
MonoDelta KUDU_EXPORT operator-(const MonoDelta& lhs, const MonoDelta& rhs);

/// @param [in] lhs
///   A time interval for addition: the left-hand operand.
/// @param [in] rhs
///   A time interval for addition: the right-hand operand.
/// @return A MonoDelta object representing the result time interval.
MonoDelta KUDU_EXPORT operator+(const MonoDelta& lhs, const MonoDelta& rhs);
///@}

/// @name Syntactic sugar: binary operators for MonoTime.
///@{

/// Check if the specified objects represent the same point in time.
///
/// This is a handy operator which is semantically equivalent to
/// MonoTime::Equals().
///
/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the given objects represent the same point in time.
bool KUDU_EXPORT operator==(const MonoTime& lhs, const MonoTime& rhs);

/// Check if the specified objects represent different points in time.
///
/// This is a handy operator which is semantically equivalent to the negation of
/// MonoTime::Equals().
///
/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the given object represents a different point in time
///   than the specified one.
bool KUDU_EXPORT operator!=(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents an earlier point in time
///   than the @c rhs object.
bool KUDU_EXPORT operator<(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents an earlier than or
///   the same point in time as the @c rhs object.
bool KUDU_EXPORT operator<=(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents a later point in time
///   than the @c rhs object.
bool KUDU_EXPORT operator>(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents a later than or
///   the same point in time as the @c rhs object.
bool KUDU_EXPORT operator>=(const MonoTime& lhs, const MonoTime& rhs);
///@}

/// @name Syntactic sugar: mixed binary operators for MonoTime/MonoDelta.
///@{

/// Add the specified time interval to the given point in time.
///
/// @param [in] t
///   A MonoTime object representing the given point in time.
/// @param [in] delta
///   A MonoDelta object representing the specified time interval.
/// @return A MonoTime object representing the resulting point in time.
MonoTime KUDU_EXPORT operator+(const MonoTime& t, const MonoDelta& delta);

/// Subtract the specified time interval from the given point in time.
///
/// @param [in] t
///   A MonoTime object representing the given point in time.
/// @param [in] delta
///   A MonoDelta object representing the specified time interval.
/// @return A MonoTime object representing the resulting point in time.
MonoTime KUDU_EXPORT operator-(const MonoTime& t, const MonoDelta& delta);

/// Compute the time interval between the specified points in time.
///
/// @param [in] t_end
///   The second point in time.  Semantically corresponds to the end
///   of the resulting time interval.
/// @param [in] t_beg
///   The first point in time.  Semantically corresponds to the beginning
///   of the resulting time interval.
/// @return A MonoDelta object representing the time interval between the
///   specified points in time.
MonoDelta KUDU_EXPORT operator-(const MonoTime& t_end, const MonoTime& t_begin);
///@}

/// @cond PRIVATE_API

/// Allow the use of MonoTime with DCHECK_XX.
///
/// Private API.
///
/// @param [out] os
///   An ostream output object.
/// @param [in] time
///   A MonoTime object to output.
/// @return An ostream object containing the nanosecond point in time value
///   of the Monotime object.
std::ostream& operator<<(std::ostream& os, const kudu::MonoTime& time);
/// @endcond

} // namespace kudu

#endif
