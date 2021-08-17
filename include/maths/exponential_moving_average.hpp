// A structure for calculating an exponential moving average.
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

#ifndef MATHS_EXPONENTIAL_MOVING_AVERAGE
#define MATHS_EXPONENTIAL_MOVING_AVERAGE

#include <algorithm>
#include <atomic>

/// Functions for doing maths.
namespace Maths {

/// @brief A structure for calculating an exponential moving average.
///
/// @tparam T the type for the exponential moving average
/// @details
/// The exponential moving average \f${\mu}_t\f$ for event \f$t\f$, corresponding
/// observation \f$o_t\f$, initial observation \f$o_0\f$, and rate of change
/// parameter \f$\alpha\f$ is defined as:
///
/// \f[ {\mu}_t \gets \alpha \cdot o_t + (1 - \alpha) \cdot {\mu}_{t - 1} | {\mu}_0 = o_0\f]
///
/// where:
///
/// - \f$\alpha \in [0, 1]\f$
/// - \f${\mu}_t \in (-\infty, \infty)\f$
/// - \f$o_t \in (-\infty, \infty)\f$
///
template<typename T = float>
struct ExponentialMovingAverage {
 private:
    /// the alpha parameter of the exponential moving average
    std::atomic<T> alpha;
    /// the exponential moving average value
    std::atomic<T> average;

 public:
    /// the minimal value for alpha to take
    static constexpr T ALPHA_MIN = 0;
    /// the maximal value for alpha to take
    static constexpr T ALPHA_MAX = 1;

    /// @brief Initialize an exponential moving average with given
    /// \f$\alpha\f$ and \f$o_0\f$.
    ///
    /// @param alpha_ the \f$\alpha\f$ parameter, i.e., rate of change
    /// @param observation the initial observation \f$o_0\f$ for the average
    ///
    explicit ExponentialMovingAverage(T alpha_, T observation = 0) :
        alpha(std::clamp(alpha_, ALPHA_MIN, ALPHA_MAX)), average(observation) { }

    /// @brief Reset the average to initial state \f${\mu}_0\f$ with
    /// observation \f$o_0\f$.
    ///
    /// @param observation the initial observation \f$o_0\f$ for the average
    ///
    inline void reset(T observation = 0) { average = observation; }

    /// @brief Set the \f$\alpha\f$ parameter to a new value.
    ///
    /// @param alpha_ the \f$\alpha\f$ parameter, i.e., rate of change
    ///
    inline void set_alpha(T alpha_) {
        alpha = std::clamp(alpha_, ALPHA_MIN, ALPHA_MAX);
    }

    /// @brief Return the \f$\alpha\f$ parameter.
    ///
    /// @returns the \f$\alpha\f$ parameter for calculating the average
    ///
    inline T get_alpha() const { return alpha; }

    /// @brief Calculate the next average \f${\mu}_t\f$ based on observation
    /// \f$o_t\f$.
    ///
    /// @param observation the observation \f$o_t\f$ to integrate into the average
    ///
    inline void process(T observation) {
        average = average + alpha * (observation - average);
    }

    /// @brief Return the current value of the exponential moving average
    /// \f${\mu}_t\f$.
    ///
    /// @returns the current average \f${\mu}_t\f$
    ///
    inline T get_average() const { return average; }
};

}  // namespace Maths

#endif  // MATHS_EXPONENTIAL_MOVING_AVERAGE
