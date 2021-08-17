// A mean reversion trading strategy.
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
// Reference: High frequency trading strategies, market fragility and price
//            spikes: an agent based model perspective
// Reference: TODO
//

#ifndef STRATEGIES_MEAN_REVERSION_HPP
#define STRATEGIES_MEAN_REVERSION_HPP

#include "data_feed/receiver.hpp"
#include "order_entry/client.hpp"
#include "maths/probability.hpp"
#include "maths/exponential_moving_variance.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <iostream>

/// Direct market access trading strategies.
namespace Strategies {

/// @brief The mean reversion trader strategy logic.
/// @details
/// \image html mean-reversion/mean-reversion.svg "Mean Reversion Agent Algorithm"
/// \image latex mean-reversion/mean-reversion.pdf "Mean Reversion Agent Algorithm"
///
class MeanReversion {
 private:
    /// a type for the receiver using this class as an event handler
    typedef DataFeed::Receiver<MeanReversion> DataFeedReceiver;
    /// the receiver for rebuilding the instrument's book
    DataFeedReceiver receiver;
    /// the client for managing orders on the market
    OrderEntry::Client client;
    /// the timer for waiting
    asio::steady_timer timer;
    /// the amount of time to sleep between actions
    uint32_t sleep_time;
    /// the probability of running the strategy when acting
    float P_act;
    /// a flag determining if the strategy is running
    std::atomic<bool> is_running;

    /// the size of the orders to place
    OrderEntry::Quantity size;
    /// the factor k to multiply the standard deviation by to determine decision
    double deviations;
    /// the exponential moving variance of the midpoint price
    Maths::ExponentialMovingVariance<double> midpoint;

    /// @brief start the strategy
    inline void start_strategy() {
        timer.expires_after(std::chrono::milliseconds(sleep_time));
        timer.async_wait([this](const std::error_code& error) {
            if (error) {  // TODO: handle error code
                std::cout << "MeanReversion::start_strategy - " << error << std::endl;
                return;
            }
            if (Maths::Probability::boolean(P_act)) do_strategy();
            start_strategy();
        });
    }

    /// @brief do the strategy.
    void do_strategy() {
        if (client.has_active_order())  // cancel existing orders
            client.send<OrderEntry::Messages::PurgeRequest>();
        // update the exponential moving variance
        auto change = midpoint.process(receiver.get_book().last_price());
        // calculate the decision boundary
        double decision_boundary = deviations * midpoint.get_stddev();
        // place an order (take a position)
        if (change >= decision_boundary) {  // price increasing
            // take a short position, i.e., sell
            auto best_sell = receiver.get_book().last_best_sell();
            // check for underflow
            if (best_sell <= std::numeric_limits<OrderEntry::Price>::min() + 1) return;
            // decrement the price
            auto price = best_sell - 1;
            // send the limit order
            client.send<OrderEntry::Messages::OrderRequest>(price, size, OrderEntry::Side::Sell);
        }
        else if (change <= -decision_boundary) {  // price decreasing
            // take a long position, i.e., buy
            auto best_buy = receiver.get_book().last_best_buy();
            // check for overflow
            if (best_buy >= std::numeric_limits<OrderEntry::Price>::max() - 1) return;
            // increment the price
            auto price = best_buy + 1;
            // send the order
            client.send<OrderEntry::Messages::OrderRequest>(price, size, OrderEntry::Side::Buy);
        }
    }

 public:
    /// @brief Initialize the strategy.
    ///
    /// @param feed_context the IO context to create the feed with
    /// @param context the IO context to create the strategy with
    /// @param options the JSON object with agent-dependent options
    ///
    MeanReversion(
        asio::io_context& feed_context,
        asio::io_context& context,
        nlohmann::json options
    ) :
        receiver(
            feed_context,
            asio::ip::make_address(options["data_feed"]["listen"].get<std::string>()),
            asio::ip::make_address(options["data_feed"]["group"].get<std::string>()),
            options["data_feed"]["port"].get<uint16_t>(),
            *this
        ),
        client(
            context,
            options["order_entry"]["host"].get<std::string>(),
            std::to_string(options["order_entry"]["port"].get<uint16_t>())
        ),
        timer(context),
        sleep_time(options["strategy"]["sleep_time"].get<uint32_t>()),
        P_act(options["strategy"]["P_act"].get<double>()),
        is_running(false),
        size(options["strategy"]["size"].get<OrderEntry::Quantity>()),
        deviations(options["strategy"]["deviations"].get<double>()),
        midpoint(options["strategy"]["weight"].get<double>(), options["strategy"]["average"].get<double>()) {
            // login to the order entry client
            auto username = OrderEntry::make_username(options["order_entry"]["username"].get<std::string>());
            auto password = OrderEntry::make_password(options["order_entry"]["password"].get<std::string>());
            client.send<OrderEntry::Messages::LoginRequest>(username, password);
        }

    /// @brief Handle a start of session message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(DataFeedReceiver* receiver, const DataFeed::Messages::StartOfSession& message) {
        if (is_running) {
            std::cerr << "received start of session when already running" << std::endl;
            return;
        }
        // start the strategy
        start_strategy();
        // set the running flag to true
        is_running = true;
    }

    /// @brief Handle an end of session message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(DataFeedReceiver* receiver, const DataFeed::Messages::EndOfSession& message) {
        if (not is_running) {
            std::cerr << "received end of session when not running" << std::endl;
            return;
        }
        // stop the strategy
        timer.cancel();
        is_running = false;
    }

    /// @brief Handle a clear book message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(DataFeedReceiver* receiver, const DataFeed::Messages::Clear& message) { }

    /// @brief Handle an add order message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(DataFeedReceiver* receiver, const DataFeed::Messages::AddOrder& message) { }

    /// @brief Handle a delete order message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(DataFeedReceiver* receiver, const DataFeed::Messages::DeleteOrder& message) { }

    /// @brief Handle a trade message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(DataFeedReceiver* receiver, const DataFeed::Messages::Trade& message) { }
};

}  // namespace Strategies

#endif  // STRATEGIES_MEAN_REVERSION_HPP
