// A structure for calculating the Rate of Change (ROC) over a rolling window.
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

#ifndef MATHS_RATE_OF_CHANGE
#define MATHS_RATE_OF_CHANGE

#include <algorithm>
#include <atomic>
#include <limits>
#include <vector>

/// Functions for doing maths.
namespace Maths {

/// @brief A structure for calculating the Rate of Change (ROC) over a rolling window.
///
/// @tparam Observation the type for the observations
/// @tparam RoC the type for the rate of change to be calculated in
/// @details
/// The Rate of Change (ROC), i.e., momentum, is a technical analysis indicator
/// measuring the difference between values in a time series window. A moving
/// window of values, i.e., history vector \f$\boldsymbol{h}^n\f$, stores
/// of the last \f$n\f$ observations. An indexing variable \f$i\f$ determines
/// the head of the circular moving window. For a new observation \f$o_t\f$ at
/// time \f$t\f$, the history vector is updated with the observation
/// \f$\boldsymbol{h_i} \gets o_t\f$. The RoC is calculated from the current
/// observation \f$\boldsymbol{h}_i\f$ and the oldest observation
/// \f$\boldsymbol{h}_{(i + 1) \bmod n}\f$ as:
///
/// \f[ r_t = \frac{\boldsymbol{h}_i - \boldsymbol{h}_{(i + 1) \bmod n}}{\boldsymbol{h}_{(i + 1) \bmod n}}\f]
///
/// The index is updated as \f$i \gets (i + 1) \bmod n\f$.
///
template<typename Observation = double, typename RoC = double>
struct RateOfChange {
 private:
    /// the history of values
    std::vector<Observation> history;
    /// the current price
    std::size_t index = 0;
    /// the rate of change of the asset price
    RoC rate_of_change = 0;

 public:
    /// the inclusive minimal size for the length of the history
    static constexpr std::size_t LENGTH_MIN = 2;
    /// the inclusive maximal size for the length of the history
    static constexpr std::size_t LENGTH_MAX = std::numeric_limits<uint16_t>::max() - 1;

    /// @brief Initialize a new RoC with given history length \f$n\f$.
    ///
    /// @param length the length of the history window \f$n\f$
    ///
    explicit RateOfChange(std::size_t length) {
        if (length < LENGTH_MIN)
            throw "length must be >= " + std::to_string(LENGTH_MIN);
        if (length > LENGTH_MAX)
            throw "length must be <= " + std::to_string(LENGTH_MAX);
        history.resize(length, 0);
    }

    /// @brief Return the length of the rate of change history.
    ///
    /// @returns the length of the history window \f$n\f$
    ///
    std::size_t get_length() const { return history.size(); }

    /// @brief Return the current rate of change (RoC).
    ///
    /// @returns the current RoC \f$r_t\f$
    ///
    inline RoC get_rate_of_change() const { return rate_of_change; }

    /// @brief Reset the history to its initial state.
    inline void reset() {
        std::fill(history.begin(), history.end(), 0);
        index = 0;
        rate_of_change = 0;
    }

    /// @brief Calculate the next RoC based on observation \f$o_t\f$.
    ///
    /// @param observation the observation \f$o_t\f$ to integrate into the RoC
    ///
    inline void process(Observation observation) {
        // add the current observation to the history
        history[index] = observation;
        auto next_index = (index + 1) % history.size();
        // get the price from the beginning of the history
        auto value = history[next_index];
        index = next_index;
        // return if still learning value information (prevent divide by 0)
        if (value == 0) return;
        // set the rate of change
        rate_of_change = (observation - value) / value;
    }
};

}  // namespace Maths

#endif  // MATHS_RATE_OF_CHANGE
