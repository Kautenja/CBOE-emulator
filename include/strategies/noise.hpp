// A complex noise trading strategy.
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

#ifndef STRATEGIES_NOISE_HPP
#define STRATEGIES_NOISE_HPP

#include "data_feed/receiver.hpp"
#include "order_entry/client.hpp"
#include "maths/probability.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <iostream>

/// Direct market access trading strategies.
namespace Strategies {

/// @brief The noise trader strategy logic.
/// @details
/// \image html noise/noise.svg "Noise Agent Algorithm"
/// \image latex noise/noise.pdf "Noise Agent Algorithm"
///
class Noise {
 private:
    /// a type for the receiver using this class as an event handler
    typedef DataFeed::Receiver<Noise> DataFeedReceiver;
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

    /// the side of the book the strategy is currently trading on
    OrderEntry::Side side;
    /// the possible order actions to perform
    enum class OrderType : int {Market, Limit, Cancel};
    /// a distribution to draw discrete order actions from
    std::discrete_distribution<int> order_distribution;
    /// the possible limit order actions to perform
    enum LimitType : int {Cross, Inside, AtBest, Outside};
    /// a discrete distribution to draw limit order actions from
    std::discrete_distribution<int> limit_distribution;
    /// the mean size (market)
    double size_market_mean;
    /// the standard deviation in size (market)
    double size_market_std;
    /// the mean size (limit)
    double size_limit_mean;
    /// the standard deviation in size (limit)
    double size_limit_std;
    /// the x min outside parameter of the power law distribution to sample
    /// deep limit order prices from
    double x_min_outside;
    /// the beta parameter of the power law distribution to sample
    /// deep limit order prices from
    double beta_exp;

    /// @brief Return the size for a market order.
    inline OrderEntry::Quantity size(double mean, double std) {
        return static_cast<int>(Maths::Probability::lognormal(mean, std));
    }

    /// @brief Return how far outside the spread a limit price should go.
    inline OrderEntry::Price outside() {
        return static_cast<int>(Maths::Probability::power_law(x_min_outside, beta_exp));
    }

    /// @brief Submit a new limit order.
    void limit_order() {
        switch (static_cast<LimitType>(limit_distribution(Maths::Probability::generator))) {
            case LimitType::Cross: {  // cross the spread
                switch (side) {
                    case OrderEntry::Side::Sell: {
                        client.send<OrderEntry::Messages::OrderRequest>(
                            receiver.get_book().last_best_buy(),
                            size(size_limit_mean, size_limit_std),
                            side
                        );
                        break;
                    }
                    case OrderEntry::Side::Buy: {
                        client.send<OrderEntry::Messages::OrderRequest>(
                            receiver.get_book().last_best_sell(),
                            size(size_limit_mean, size_limit_std),
                            side
                        );
                        break;
                    }
                }
                break;
            }
            case LimitType::Inside: {  // inside the spread
                // get the best buy price
                auto bid = receiver.get_book().last_best_buy();
                // check for overflow
                if (bid >= std::numeric_limits<OrderEntry::Price>::max() - 1)
                    bid = std::numeric_limits<OrderEntry::Price>::max();
                else  // safe to increase the bid
                    bid += 1;

                // get the best sell price
                auto ask = receiver.get_book().last_best_sell();
                // check for underflow
                if (ask <= std::numeric_limits<OrderEntry::Price>::min() + 1)
                    ask = std::numeric_limits<OrderEntry::Price>::min();
                else  // safe to decrease the bid
                    ask -= 1;

                // return if there is no spread (the "inside" doesn't exist)
                if (bid >= ask) return;
                // place the order with a random size and price inside the spread
                client.send<OrderEntry::Messages::OrderRequest>(
                    Maths::Probability::uniform_int(bid, ask),
                    size(size_limit_mean, size_limit_std),
                    side
                );
                break;
            }
            case LimitType::AtBest: {  // at the best price
                switch (side) {
                    case OrderEntry::Side::Sell: {
                        client.send<OrderEntry::Messages::OrderRequest>(
                            receiver.get_book().last_best_sell(),
                            size(size_limit_mean, size_limit_std),
                            side
                        );
                        break;

                    }
                    case OrderEntry::Side::Buy: {
                        client.send<OrderEntry::Messages::OrderRequest>(
                            receiver.get_book().last_best_buy(),
                            size(size_limit_mean, size_limit_std),
                            side
                        );
                        break;
                    }
                }
                break;
            }
            case LimitType::Outside: {  // outside the spread (deep in the book)
                // create some noise for the price
                auto noise = outside();
                switch (side) {
                    case OrderEntry::Side::Sell: {
                        // get the best sell
                        auto price = receiver.get_book().last_best_sell();
                        // check if the noise causes overflow
                        if (price >= std::numeric_limits<OrderEntry::Price>::max() - noise)
                            price = std::numeric_limits<OrderEntry::Price>::max();
                        else  // add the noise
                            price += noise;
                        client.send<OrderEntry::Messages::OrderRequest>(
                            price,
                            size(size_limit_mean, size_limit_std),
                            side
                        );
                        break;
                    }
                    case OrderEntry::Side::Buy:  {
                        // get the best buy
                        auto price = receiver.get_book().last_best_buy();
                        // check if the noise causes underflow
                        if (price <= std::numeric_limits<OrderEntry::Price>::min() + noise)
                            price = std::numeric_limits<OrderEntry::Price>::min();
                        else  // reduce by the noise
                            price -= noise;
                        client.send<OrderEntry::Messages::OrderRequest>(
                            price,
                            size(size_limit_mean, size_limit_std),
                            side
                        );
                        break;
                    }
                }
                break;
            }
        }
    }

    /// @brief start the strategy
    inline void start_strategy() {
        timer.expires_after(std::chrono::milliseconds(sleep_time));
        timer.async_wait([this](const std::error_code& error) {
            if (error) {  // TODO: handle error code
                std::cout << "Noise::start_strategy - " << error << std::endl;
                return;
            }
            if (Maths::Probability::boolean(P_act)) do_strategy();
            start_strategy();
        });
    }

    /// @brief do the strategy.
    void do_strategy() {
        side = Maths::Probability::boolean() ? OrderEntry::Side::Buy : OrderEntry::Side::Sell;
        // select an action to perform
        switch (static_cast<OrderType>(order_distribution(Maths::Probability::generator))) {
            case OrderType::Market: {  // make a market order
                client.send<OrderEntry::Messages::OrderRequest>(
                    OrderEntry::Messages::ORDER_PRICE_MARKET,
                    size(size_market_mean, size_market_std),
                    side
                );
                break;
            }
            case OrderType::Limit: {  // make a limit order
                limit_order();
                break;
            }
            case OrderType::Cancel: {  // cancel the oldest order
                if (client.has_active_order())  // an order exists
                    client.send<OrderEntry::Messages::PurgeRequest>();
                break;
            }
        }
    }

 public:
    /// @brief Initialize the strategy.
    ///
    /// @param feed_context the IO context to create the feed with
    /// @param context the IO context to create the strategy with
    /// @param options the JSON object with strategy-dependent options
    ///
    Noise(
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
        order_distribution{
            options["strategy"]["order_distribution"]["P_market"].get<double>(),
            options["strategy"]["order_distribution"]["P_limit"].get<double>(),
            options["strategy"]["order_distribution"]["P_cancel"].get<double>()
        },
        limit_distribution{
            options["strategy"]["limit_distribution"]["P_cross"].get<double>(),
            options["strategy"]["limit_distribution"]["P_inside"].get<double>(),
            options["strategy"]["limit_distribution"]["P_best"].get<double>(),
            options["strategy"]["limit_distribution"]["P_outside"].get<double>()
        },
        size_market_mean(options["strategy"]["size_market_mean"].get<double>() + 2),
        size_market_std(options["strategy"]["size_market_std"].get<double>()),
        size_limit_mean(options["strategy"]["size_limit_mean"].get<double>() + 2),
        size_limit_std(options["strategy"]["size_limit_std"].get<double>()),
        x_min_outside(options["strategy"]["x_min_outside"].get<double>()),
        beta_exp(options["strategy"]["beta_exp"].get<double>()) {
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

#endif  // STRATEGIES_NOISE_HPP
