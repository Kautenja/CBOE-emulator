// Functions for dealing with probability.
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

#ifndef PROBABILITY_HPP
#define PROBABILITY_HPP

#include <random>
#include <unistd.h>

/// Functions for doing maths.
namespace Maths {

/// Functions for dealing with probability.
namespace Probability {

/// the global generator for the probability module.
std::mt19937 generator(getpid());

/// @brief Set the seed for the random number generator.
///
/// @tparam T the class of the seed value
/// @param value the seed for the random number generators
///
template<typename T>
inline void seed(T value) { generator.seed(value); }

/// @brief Sample a number from an integer-valued uniform distribution.
///
/// @tparam T the class of the number to sample
/// @param min the inclusive minimal value \f$v_{min}\f$ to sample
/// @param max the inclusive maximal value \f$v_{max}\f$ to sample
/// @returns \f$y \sim \mathcal{U}\{v_{min}, v_{max}\}\f$
///
template<typename T>
inline T uniform_int(T min, T max) {
    return std::uniform_int_distribution<T>(min, max)(generator);
}

/// @brief Sample a number from a real-valued uniform distribution.
///
/// @tparam T the class of the number to sample
/// @param min the inclusive minimal value \f$v_{min}\f$ to sample
/// @param max the inclusive maximal value \f$v_{max}\f$ to sample
/// @returns \f$y \sim \mathcal{U}(v_{min}, v_{max})\f$
///
template<typename T>
inline T uniform_real(T min, T max) {
    return std::uniform_real_distribution<T>(min, max)(generator);
}

/// @brief Sample a number from a normal distribution.
///
/// @tparam T the class of the number to sample
/// @param mean the mean value \f$\mu\f$ of the distribution
/// @param stddev the standard deviation \f$\sigma\f$ of the distribution
/// @returns \f$y \sim \mathcal{N}(\mu, \sigma)\f$
///
template<typename T>
inline T normal(T mean, T stddev) {
    return std::normal_distribution<T>(mean, stddev)(generator);
}

/// @brief Sample a number from a log-normal distribution.
///
/// @tparam T the class of the number to sample
/// @param mean the mean value \f$\mu\f$ of the distribution
/// @param stddev the standard deviation \f$\sigma\f$ of the distribution
/// @returns \f$y \sim e^{\mu + \sigma \mathcal{U}(0, 1)}\f$
///
template<typename T>
inline T lognormal(T mean, T stddev) {
    return std::exp(mean + stddev * std::uniform_real_distribution<T>(0, 1)(generator));
}

/// @brief Sample a number sample from a power law distribution.
///
/// @tparam T the class of the number to sample
/// @param constant the constant value \f$k\f$ of the distribution
/// @param exponent the exponent \f$\alpha\f$
/// @returns \f$y \sim k \cdot \mathcal{U}(0, 1)^{\alpha}\f$
///
template<typename T>
inline T power_law(T constant, T exponent) {
    return constant * std::pow(std::uniform_real_distribution<T>(0, 1)(generator), exponent);
}

/// @brief Return the result of a coin toss.
///
/// @returns \f$y \sim \mathcal{U}\{0, 1\}\f$
///
inline bool boolean() {
    return std::uniform_int_distribution<int>(0, 1)(generator);
}

/// @brief Return true with given probability.
///
/// @tparam T the class of the probability value
/// @param P the probability of sampling true, \f$p_{true}\f$
/// @returns true with \f$P(\texttt{true} = p_{true})\f$ and false with
///          \f$P(\texttt{false} = 1 - p_{true})\f$
///
template<typename T>
inline bool boolean(T P) {
    return std::uniform_real_distribution<double>(0, 1)(generator) < P;
}

}  // namespace Probability

}  // namespace Maths

#endif  // PROBABILITY_HPP
