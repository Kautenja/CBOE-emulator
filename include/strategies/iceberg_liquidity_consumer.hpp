// A liquidity consumer agent based on an iceberg trading scheme.
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

#ifndef STRATEGIES_ICEBERG_TRADER_HPP
#define STRATEGIES_ICEBERG_TRADER_HPP

#include "data_feed/receiver.hpp"
#include "order_entry/client.hpp"
#include "maths/probability.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <iostream>

/// Direct market access trading strategies.
namespace Strategies {

/// @brief The iceberg trader strategy logic.
/// @details
/// \image html iceberg-liquidity-consumer/iceberg-liquidity-consumer.svg "Iceberg Liquidity Consumer Agent Algorithm"
/// \image latex iceberg-liquidity-consumer/iceberg-liquidity-consumer.pdf "Iceberg Liquidity Consumer Agent Algorithm"
///
class IcebergLiquidityConsumer {
 private:
    /// a type for the receiver using this class as an event handler
    typedef DataFeed::Receiver<IcebergLiquidityConsumer> DataFeedReceiver;
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

    /// the minimal size order to sample
    OrderEntry::Quantity minimum_size;
    /// the maximal size order to sample
    OrderEntry::Quantity maximum_size;
    /// the size of the iceberg orders to place
    OrderEntry::Quantity iceberg_size;
    /// the side the liquidity consumer is trading on
    OrderEntry::Side side = OrderEntry::Side::Sell;
    /// the size of the order to place
    OrderEntry::Quantity size = 0;

    /// @brief Place a sell-side order.
    inline void sell_order() {
        auto volume = receiver.get_book().volume_buy_best();
        if (volume == 0) return;
        auto order_size = std::min(size, iceberg_size);
        // std::cout << "placing sell order for " << order_size << " shares" << std::endl;
        client.send<OrderEntry::Messages::OrderRequest>(
            OrderEntry::Messages::ORDER_PRICE_MARKET,
            order_size,
            OrderEntry::Side::Sell
        );
        // remove the amount ordered from the size for the day
        size -= order_size;
        // std::cout << size << " shares left in todays order quantity" << std::endl;
    }

    /// @brief Place a buy-side order.
    inline void buy_order() {
        auto volume = receiver.get_book().volume_sell_best();
        if (volume == 0) return;
        auto order_size = std::min(size, iceberg_size);
        // std::cout << "placing buy order for " << order_size << " shares" << std::endl;
        client.send<OrderEntry::Messages::OrderRequest>(
            OrderEntry::Messages::ORDER_PRICE_MARKET,
            order_size,
            OrderEntry::Side::Buy
        );
        // remove the amount ordered from the size for the day
        size -= order_size;
        // std::cout << size << " shares left in todays order quantity" << std::endl;
    }

    /// @brief start the strategy
    inline void start_strategy() {
        timer.expires_after(std::chrono::milliseconds(sleep_time));
        timer.async_wait([this](const std::error_code& error) {
            if (error) {  // TODO: handle error code
                std::cout << "IcebergLiquidityConsumer::start_strategy - " << error << std::endl;
                return;
            }
            if (Maths::Probability::boolean(P_act)) do_strategy();
            start_strategy();
        });
    }

    /// @brief do the strategy.
    void do_strategy() {
        // make an order based on the agent's side for the day
        switch (side) {
            case OrderEntry::Side::Sell: { sell_order(); break; }
            case OrderEntry::Side::Buy:  { buy_order();  break; }
        }
    }

 public:
    /// @brief Initialize the strategy.
    ///
    /// @param feed_context the IO context to create the data feed with
    /// @param context the IO context to create the strategy with
    /// @param options the JSON object with agent-dependent options
    ///
    IcebergLiquidityConsumer(
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
        minimum_size(options["strategy"]["minimum_size"].get<double>()),
        maximum_size(options["strategy"]["maximum_size"].get<double>()),
        iceberg_size(options["strategy"]["iceberg_size"].get<double>()) {
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
        // std::cout << "trading session started" << std::endl;
        // select a random side for the day's orders
        side = Maths::Probability::boolean() ? OrderEntry::Side::Buy : OrderEntry::Side::Sell;
        // std::cout << "placing " << side << " orders today" << std::endl;
        size = Maths::Probability::uniform_int(minimum_size, maximum_size);
        // std::cout << "ordering " << size << " shares today" << std::endl;
        // start the strategy
        start_strategy();
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
        // std::cout << "trading session ended" << std::endl;
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

#endif  // STRATEGIES_ICEBERG_TRADER_HPP
