// A market maker trading agent.
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
// Reference: An Agent-Based Model for Market Impact
//

#ifndef STRATEGIES_MARKET_MAKER_HPP
#define STRATEGIES_MARKET_MAKER_HPP

#include "data_feed/receiver.hpp"
#include "order_entry/client.hpp"
#include "maths/probability.hpp"
#include "maths/exponential_moving_average.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <iostream>

/// Direct market access trading strategies.
namespace Strategies {

/// @brief The market maker strategy logic.
/// @details
/// \image html market-maker/market-maker.svg "Market Maker Agent Algorithm"
/// \image latex market-maker/market-maker.pdf "Market Maker Agent Algorithm"
///
class MarketMaker {
 private:
    /// a type for the receiver using this class as an event handler
    typedef DataFeed::Receiver<MarketMaker> DataFeedReceiver;
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

    /// the minimal order size to sample
    OrderEntry::Quantity minimum_size;
    /// the maximal order size to sample
    OrderEntry::Quantity maximum_size;
    /// the number of shares to hedge with
    OrderEntry::Quantity hedge_size;
    /// the decision boundary for when to place an order
    double decision_boundary;

    /// the exponential moving average of trade side
    Maths::ExponentialMovingAverage<double> trade_side;

    /// @brief start the strategy
    inline void start_strategy() {
        timer.expires_after(std::chrono::milliseconds(sleep_time));
        timer.async_wait([this](const std::error_code& error) {
            if (error) {  // TODO: handle error code
                std::cout << "MarketMaker::start_strategy - " << error << std::endl;
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
        // compare the average to the decision boundary
        if (trade_side.get_average() > decision_boundary) {  // buy order likely
            // place sell
            client.send<OrderEntry::Messages::OrderRequest>(
                receiver.get_book().last_best_sell(),
                Maths::Probability::uniform_int(minimum_size, maximum_size),
                OrderEntry::Side::Sell
            );
            // hedge buy
            client.send<OrderEntry::Messages::OrderRequest>(
                receiver.get_book().last_best_buy(),
                hedge_size,
                OrderEntry::Side::Buy
            );
        } else if (trade_side.get_average() < -decision_boundary) {  // sell order likely
            // place buy
            client.send<OrderEntry::Messages::OrderRequest>(
                receiver.get_book().last_best_buy(),
                Maths::Probability::uniform_int(minimum_size, maximum_size),
                OrderEntry::Side::Buy
            );
            // hedge sell
            client.send<OrderEntry::Messages::OrderRequest>(
                receiver.get_book().last_best_sell(),
                hedge_size,
                OrderEntry::Side::Sell
            );
        }
    }

 public:
    /// @brief Initialize the strategy.
    ///
    /// @param feed_context the IO context to create the feed with
    /// @param context the IO context to create the strategy with
    /// @param options the JSON object with agent-dependent options
    ///
    MarketMaker(
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
        minimum_size(options["strategy"]["minimum_size"].get<OrderEntry::Quantity>()),
        maximum_size(options["strategy"]["maximum_size"].get<OrderEntry::Quantity>()),
        hedge_size(options["strategy"]["hedge_size"].get<OrderEntry::Quantity>()),
        decision_boundary(options["strategy"]["decision_boundary"].get<double>()),
        trade_side(options["strategy"]["weight"].get<double>(), options["strategy"]["average"].get<double>()) {
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
        // clear the average trade direction
        trade_side.reset(Maths::Probability::uniform_real(-1.0, 1.0));
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
    inline void did_receive(DataFeedReceiver* receiver, const DataFeed::Messages::Trade& message) {
        trade_side.process(DataFeed::side_to_double(message.side));
    }
};

}  // namespace Strategies

#endif  // STRATEGIES_MARKET_MAKER_HPP
