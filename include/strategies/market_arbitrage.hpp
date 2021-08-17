// A market arbitrage (latency) trading agent.
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

#ifndef STRATEGIES_MARKET_ARBITRAGE_HPP
#define STRATEGIES_MARKET_ARBITRAGE_HPP

#include "data_feed/receiver.hpp"
#include "order_entry/client.hpp"
#include "maths/probability.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <iostream>

/// Direct market access trading strategies.
namespace Strategies {

/// @brief The market arbitrage strategy logic.
/// @details
/// \image html market-arbitrage/market-arbitrage.svg "Market Arbitrage Agent Algorithm"
/// \image latex market-arbitrage/market-arbitrage.pdf "Market Arbitrage Agent Algorithm"
///
class MarketArbitrage {
 private:
    /// a type for the receiver using this class as an event handler
    typedef DataFeed::Receiver<MarketArbitrage> DataFeedReceiver;
    /// the receiver for rebuilding the instrument A's book
    DataFeedReceiver receiverA;
    /// the receiver for rebuilding the instrument B's book
    DataFeedReceiver receiverB;
    /// the client for managing orders on the market A
    OrderEntry::Client clientA;
    /// the client for managing orders on the market B
    OrderEntry::Client clientB;
    /// the timer for waiting
    asio::steady_timer timer;
    /// the amount of time to sleep between actions
    uint32_t sleep_time;
    /// the probability of running the strategy when acting
    float P_act;
    /// a flag determining if the strategy is running
    std::atomic<bool> is_running;

    /// the size of the market orders to place
    OrderEntry::Quantity size;
    /// the minimal spread to trade on
    DataFeed::Quantity spread;

    /// @brief start the strategy
    inline void start_strategy() {
        timer.expires_after(std::chrono::milliseconds(sleep_time));
        timer.async_wait([this](const std::error_code& error) {
            if (error) {  // TODO: handle error code
                std::cout << "MarketArbitrage::start_strategy - " << error << std::endl;
                return;
            }
            if (Maths::Probability::boolean(P_act)) do_strategy();
            start_strategy();
        });
    }

    /// @brief do the strategy.
    void do_strategy() {
        // check if A is selling for less than B is buying
        if (receiverA.get_book().does_cross(receiverB.get_book(), spread)) {
            // buy on A and sell on B
            // std::cout << "buy on A and sell on B" << std::endl;
            clientA.send<OrderEntry::Messages::OrderRequest>(
                OrderEntry::Messages::ORDER_PRICE_MARKET,
                size,
                OrderEntry::Side::Buy
            );
            clientB.send<OrderEntry::Messages::OrderRequest>(
                OrderEntry::Messages::ORDER_PRICE_MARKET,
                size,
                OrderEntry::Side::Sell
            );
        // check if B is selling for less than A is buying
        } else if (receiverB.get_book().does_cross(receiverA.get_book(), spread)) {
            // buy on B and sell on A
            // std::cout << "buy on B and sell on A" << std::endl;
            clientB.send<OrderEntry::Messages::OrderRequest>(
                OrderEntry::Messages::ORDER_PRICE_MARKET,
                size,
                OrderEntry::Side::Buy
            );
            clientA.send<OrderEntry::Messages::OrderRequest>(
                OrderEntry::Messages::ORDER_PRICE_MARKET,
                size,
                OrderEntry::Side::Sell
            );
        }
    }

 public:
    /// @brief Initialize the strategy.
    ///
    /// @param feedA_context the IO context to create a feed for A with
    /// @param feedB_context the IO context to create a feed for B with
    /// @param context the IO context to create the strategy with
    /// @param options the JSON object with agent-dependent options
    ///
    MarketArbitrage(
        asio::io_context& feedA_context,
        asio::io_context& feedB_context,
        asio::io_context& context,
        nlohmann::json options
    ) :
        receiverA(
            feedA_context,
            asio::ip::make_address(options["data_feed"]["A"]["listen"].get<std::string>()),
            asio::ip::make_address(options["data_feed"]["A"]["group"].get<std::string>()),
            options["data_feed"]["A"]["port"].get<uint16_t>(),
            *this
        ),
        receiverB(
            feedB_context,
            asio::ip::make_address(options["data_feed"]["B"]["listen"].get<std::string>()),
            asio::ip::make_address(options["data_feed"]["B"]["group"].get<std::string>()),
            options["data_feed"]["B"]["port"].get<uint16_t>(),
            *this
        ),
        clientA(
            context,
            options["order_entry"]["A"]["host"].get<std::string>(),
            std::to_string(options["order_entry"]["A"]["port"].get<uint16_t>())
        ),
        clientB(
            context,
            options["order_entry"]["B"]["host"].get<std::string>(),
            std::to_string(options["order_entry"]["B"]["port"].get<uint16_t>())
        ),
        timer(context),
        sleep_time(options["strategy"]["sleep_time"].get<uint32_t>()),
        P_act(options["strategy"]["P_act"].get<double>()),
        is_running(false),
        size(options["strategy"]["size"].get<OrderEntry::Quantity>()),
        spread(options["strategy"]["spread"].get<DataFeed::Quantity>()) {
            // login to client A
            auto usernameA = OrderEntry::make_username(options["order_entry"]["A"]["username"].get<std::string>());
            auto passwordA = OrderEntry::make_password(options["order_entry"]["A"]["password"].get<std::string>());
            clientA.send<OrderEntry::Messages::LoginRequest>(usernameA, passwordA);
            // login to client B
            auto usernameB = OrderEntry::make_username(options["order_entry"]["B"]["username"].get<std::string>());
            auto passwordB = OrderEntry::make_password(options["order_entry"]["B"]["password"].get<std::string>());
            clientB.send<OrderEntry::Messages::LoginRequest>(usernameB, passwordB);
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
        // check if both markets are active
        if (receiverA.is_session_active() and receiverB.is_session_active()) {
            // start the strategy
            start_strategy();
            // set the running flag to true
            is_running = true;
        }
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
        // check if either market has stopped the trading session
        if (not receiverA.is_session_active() or not receiverB.is_session_active()) {
            // stop the strategy
            timer.cancel();
            is_running = false;
        }
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

#endif  // STRATEGIES_MARKET_ARBITRAGE_HPP
