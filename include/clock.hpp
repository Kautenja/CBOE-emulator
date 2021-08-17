// Functions for interacting with the system disciplined clock oscillator.
// Copyright 2020 Christian Kauten
//
// Author: Christian Kauten (kautenja@auburn.edu)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <chrono>
#include <cstdint>

/// Functions for interacting with the system real-time clock (RTC).
namespace Clock {

/// a type for timestamps
typedef uint64_t TimeStamp;
/// a type def for timestamp durations
typedef std::chrono::duration<TimeStamp, std::nano> TimeStampDuration;

/// @brief Return the time in nanoseconds as a 64-bit unsigned integer (Timestamp).
///
/// @returns the current time according to the system real-time clock (RTC)
///
inline TimeStamp get_time() {
    // get the time now
    auto now = std::chrono::system_clock::now();
    // convert the time now to nanosecond resolution
    auto now_ns = std::chrono::time_point_cast<TimeStampDuration>(now);
    // get the number of nanoseconds since the last epoch
    auto since_epoch = now_ns.time_since_epoch();
    // convert the time since to the epoch to nanosecond time
    auto nanos = std::chrono::duration_cast<TimeStampDuration>(since_epoch);
    // get the raw number of nanoseconds
    return nanos.count();
}

}  // namespace Clock

#endif  // CLOCK_HPP
