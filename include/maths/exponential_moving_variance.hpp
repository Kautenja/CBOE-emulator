// A structure for calculating an exponential moving variance.
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

#ifndef MATHS_EXPONENTIAL_MOVING_VARIANCE
#define MATHS_EXPONENTIAL_MOVING_VARIANCE

#include <algorithm>
#include <atomic>
#include <cmath>

/// Functions for doing maths.
namespace Maths {

/// @brief A structure for calculating an exponential moving variance.
///
/// @tparam T the type for the exponential moving average/variance
/// @details
/// The change in value between the current average \f${\mu}_{t-1}\f$ and
/// next observation \f$o_t\f$ for event \f$t\f$ is defined as\f$\delta_t\f$:
///
/// \f[ \delta_t \gets o_t - {\mu}_{t-1} \f]
///
/// The exponential moving average \f${\mu}_t\f$ for event \f$t\f$, corresponding
/// change \f$\delta_t\f$, initial observation \f$o_0\f$, and rate of change
/// parameter \f$\alpha\f$ is defined as:
///
/// \f[ {\mu}_t \gets {\mu}_{t - 1} + \alpha \cdot \delta_t | {\mu}_0 = o_0 \f]
///
/// The exponential moving variance \f${\sigma}^2_t\f$ for event \f$t\f$,
/// corresponding change \f$\delta_t\f$, and rate of change parameter
/// \f$\alpha\f$ is defined as:
///
/// \f[ {\sigma^2}_t \gets (1 - \alpha) \cdot ({\sigma^2}_{t-1} + \alpha \cdot \delta^2_t) | {\sigma^2_0} = 0 \f]
///
/// where:
///
/// - \f$\alpha \in [0, 1]\f$
/// - \f${\mu}_t \in (-\infty, \infty)\f$
/// - \f${\sigma^2}_t \in (-\infty, \infty)\f$
/// - \f$o_t \in (-\infty, \infty)\f$
///
template<typename T = float>
struct ExponentialMovingVariance {
 private:
    /// the alpha parameter of the exponential moving variance
    std::atomic<T> alpha;
    /// the exponential moving average value
    std::atomic<T> average;
    /// the exponential moving variance value
    std::atomic<T> variance = 0.f;

 public:
    /// the minimal value for alpha to take
    static constexpr T ALPHA_MIN = 0;
    /// the maximal value for alpha to take
    static constexpr T ALPHA_MAX = 1;

    /// @brief Initialize an exponential moving variance with given
    /// \f$\alpha\f$ and \f$o_0\f$.
    ///
    /// @param alpha_ the \f$\alpha\f$ parameter, i.e., rate of change
    /// @param observation the initial observation \f$o_0\f$ for the average
    ///
    explicit ExponentialMovingVariance(T alpha_, T observation = 0) :
        alpha(std::clamp(alpha_, ALPHA_MIN, ALPHA_MAX)), average(observation) { }

    /// @brief Reset the average to initial state \f${\mu}_0\f$ with observation
    /// \f$o_0\f$. The initial variance \f${\sigma^2}_0\f$ is 0.
    ///
    /// @param observation the initial observation \f$o_0\f$ for the average
    ///
    inline void reset(T observation = 0) {
        average = observation;
        variance = 0;
    }

    /// @brief Set the \f$\alpha\f$ parameter to a new value.
    ///
    /// @param alpha_ the \f$\alpha\f$ parameter, i.e., rate of change
    ///
    inline void set_alpha(T alpha_) {
        alpha = std::clamp(alpha_, ALPHA_MIN, ALPHA_MAX);
    }

    /// @brief Return the \f$\alpha\f$ parameter.
    ///
    /// @returns the \f$\alpha\f$ parameter for calculating the average and
    /// variance
    ///
    inline T get_alpha() const { return alpha; }

    /// @brief Calculate the next average \f${\mu}_t\f$ and variance
    /// \f${\sigma^2}_t\f$ based on observation \f$o_t\f$.
    ///
    /// @param observation the observation \f$o_t\f$ to integrate into the
    ///        average and variance
    /// @returns the change in value between the observation and current average
    ///
    inline T process(T observation) {
        T delta = observation - average;
        average = average + alpha * delta;
        variance = (ALPHA_MAX - alpha) * (variance + alpha * delta * delta);
        return delta;
    }

    /// @brief Return the current value of the exponential moving average
    /// \f${\mu}_t\f$.
    ///
    /// @returns the current average \f${\mu}_t\f$
    ///
    inline T get_average() const { return average; }

    /// @brief Return the current value of the exponential moving variance
    /// \f${\sigma^2}_t\f$.
    ///
    /// @returns the current variance \f${\sigma^2}_t\f$
    ///
    inline T get_variance() const { return variance; }

    /// @brief Return the current value of the exponential moving standard
    /// deviation \f${\sigma}_t\f$.
    ///
    /// @returns the current standard deviation \f${\sigma}_t\f$
    ///
    inline T get_stddev() const { return sqrt(variance); }
};

}  // namespace Maths

#endif  // MATHS_EXPONENTIAL_MOVING_VARIANCE
