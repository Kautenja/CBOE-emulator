// An order book for managing Limit / Order objects in a continuous double auction.
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

#ifndef DATA_FEED_LIMIT_ORDER_BOOK_LIMIT_ORDER_BOOK_HPP
#define DATA_FEED_LIMIT_ORDER_BOOK_LIMIT_ORDER_BOOK_HPP

#include "limit_tree.hpp"
#include <unordered_map>
#include <ostream>
#include <iomanip>
#include <tuple>

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// Logic for maintaining a continuous double auction via a limit-order book.
namespace LOB {

/// a map of order IDs to orders
typedef std::unordered_map<UID, Order> UIDOrderMap;

/// @brief An order book for managing Limit / Order objects in a continuous double auction.
/// @details
/// \image html img/limit-order-book.svg "Abstract Domain Model of the Limit Order Book"
/// \image latex img/limit-order-book.pdf "Abstract Domain Model of the Limit Order Book"
///
class LimitOrderBook {
 private:
    /// the tree of sell order in the book.
    LimitTree<Side::Sell> sells;
    /// the tree of buy orders in the book.
    LimitTree<Side::Buy> buys;
    /// a mapping of order IDs to orders (for cancellation).
    UIDOrderMap orders;

 public:
    /// @brief Initialize a new limit order book object.
    LimitOrderBook() : sells(), buys(), orders() { }

    /// @brief Clear all the orders in the book.
    inline void clear() {
        sells.clear();
        buys.clear();
        orders.clear();
    }

    /// @brief Add a new sell limit order to the book.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the number of shares to sell
    /// @param price the limit price for the order
    /// @return the order ID for the order added to the book
    ///
    inline void limit_sell(UID order_id, Quantity quantity, Price price) {
        // put the order into the map
        orders.emplace(std::piecewise_construct,
            std::forward_as_tuple(order_id),
            std::forward_as_tuple(order_id, Side::Sell, quantity, price)
        );
        if (buys.best != nullptr && price <= buys.best->key) {  // crosses
            // place a market order with the limit price
            buys.market(&orders.at(order_id), [&](UID uid) { orders.erase(uid); });
            if (orders.at(order_id).quantity == 0) {  // order filled
                orders.erase(order_id);
                return;
            }
        }
        sells.limit(&orders.at(order_id));
    }

    /// @brief Add a new buy limit order to the book.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the number of shares to buy
    /// @param price the limit price for the order
    /// @return the order ID for the order added to the book
    ///
    inline void limit_buy(UID order_id, Quantity quantity, Price price) {
        // put the order into the map
        orders.emplace(std::piecewise_construct,
            std::forward_as_tuple(order_id),
            std::forward_as_tuple(order_id, Side::Buy, quantity, price)
        );
        if (sells.best != nullptr && price >= sells.best->key) {  // crosses
            // place a market order with the limit price
            sells.market(&orders.at(order_id), [&](UID uid) { orders.erase(uid); });
            if (orders.at(order_id).quantity == 0) {  // order filled
                orders.erase(order_id);
                return;
            }
        }
        buys.limit(&orders.at(order_id));
    }

    /// @brief Add a new order to the book.
    ///
    /// @param side whether the order is a buy (true) or sell (false)
    /// @param order_id the ID for the order
    /// @param quantity the number of shares to buy
    /// @param price the limit/market price for the order
    /// @returns the order ID for the order added to the book
    ///
    inline void limit(Side side, UID order_id, Quantity quantity, Price price) {
        switch (side) {  // send the order to the appropriate side
            case Side::Sell: return limit_sell(order_id, quantity, price);
            case Side::Buy:  return limit_buy(order_id, quantity, price);
        }
    }

    /// @brief Return true if the book has an order with given ID, false otherwise.
    ///
    /// @param order_id the order ID of the order to check for existence of
    /// @returns true if the book has an order with given ID, false otherwise
    ///
    inline bool has(UID order_id) const { return orders.count(order_id); }

    /// @brief Get the order with given ID.
    ///
    /// @param order_id the order ID of the order to get
    /// @returns a pointer to the order with given order ID
    ///
    inline const Order& get(UID order_id) { return orders.at(order_id); }

    /// @brief Cancel an existing order in the book.
    ///
    /// @param order_id the ID of the order to cancel
    ///
    inline void cancel(UID order_id) {
        auto order = &orders.at(order_id);
        switch (order->side) {  // remove the order from the appropriate side
            case Side::Sell: { sells.cancel(order); break; }
            case Side::Buy:  { buys.cancel(order); break; }
        }
        orders.erase(order_id);
    }

    /// @brief Reduce the quantity of the order with given ID.
    ///
    /// @param order_id the id of the order to reduce the quantity of
    /// @param quantity the amount to remove from the order
    ///
    inline void reduce(UID order_id, Quantity quantity) {
        auto order = &orders.at(order_id);
        if (quantity > order->quantity) {  // trying to remove more than available
            throw "trying to remove " +
                std::to_string(quantity) +
                " from order with " +
                std::to_string(order->quantity) +
                " available!";
        }
        // remove the quantity from the order and limit
        order->quantity -= quantity;
        order->limit->volume -= quantity;
        switch (order->side) {  // remove the order from the appropriate side
            case Side::Sell: { sells.volume -= quantity; break; }
            case Side::Buy:  { buys.volume -= quantity; break; }
        }
        if (order->quantity == 0) {  // remove the order if there is no quantity left
            switch (order->side) {  // remove the order from the appropriate side
                case Side::Sell: { sells.cancel(order); break; }
                case Side::Buy:  { buys.cancel(order); break; }
            }
            orders.erase(order_id);
        }
    }

    /// @brief Execute a sell market order.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the quantity in the market order
    ///
    inline void market_sell(UID order_id, Quantity quantity) {
        Order order{order_id, Side::Sell, quantity, 0};
        buys.market(&order, [&](UID uid) { orders.erase(uid); });
    }

    /// @brief Execute a buy market order.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the quantity of the market order
    ///
    inline void market_buy(UID order_id, Quantity quantity) {
        Order order{order_id, Side::Buy, quantity, 0};
        sells.market(&order, [&](UID uid) { orders.erase(uid); });
    }

    /// @brief Execute a market order.
    ///
    /// @param side whether the order is a sell or buy order
    /// @param order_id the ID for the order
    /// @param quantity the quantity of the market order
    ///
    inline void market(Side side, UID order_id, Quantity quantity) {
        switch (side) {  // send the market order to the appropriate side
            case Side::Sell: { market_sell(order_id, quantity); break; }
            case Side::Buy:  { market_buy(order_id, quantity); break; }
        }
    }

    /// @brief Return the best sell price.
    ///
    /// @return the best bid price in the book
    ///
    inline Price best_sell() const {
        if (sells.best == nullptr)
            return 0;
        return sells.best->key;
    }

    /// @brief Return the best buy price.
    ///
    /// @return the best bid price in the book
    ///
    inline Price best_buy() const {
        if (buys.best == nullptr)
            return 0;
        return buys.best->key;
    }

    /// @brief Return the best price for the given side.
    ///
    /// @param side the side to get the best price from
    /// @returns the best price on the given side of the book
    ///
    inline Price best(Side side) const {
        switch (side) {
            case Side::Sell: { return best_sell(); }
            case Side::Buy:  { return best_buy();  }
        }
    }

    /// @brief Return the current price of the asset.
    inline Price price() const {
        if (sells.best == nullptr && buys.best == nullptr)  // empty book
            return 0;
        if (sells.best == nullptr)  // no sell orders
            return buys.best->key;
        if (buys.best == nullptr)  // no buy orders
            return sells.best->key;
        // return the midpoint
        return (sells.best->key + buys.best->key) / 2;
    }

    /// @brief Return the last best sell price.
    ///
    /// @return the best bid price in the book
    ///
    inline Price last_best_sell() const { return sells.last_best_price; }

    /// @brief Return the last best buy price.
    ///
    /// @return the best bid price in the book
    ///
    inline Price last_best_buy() const { return buys.last_best_price; }

    /// @brief Return the best price for the given side.
    ///
    /// @param side the side to get the best price from
    /// @returns the best price on the given side of the book
    ///
    inline Price last_best(Side side) const {
        switch (side) {
            case Side::Sell: { return last_best_sell(); }
            case Side::Buy:  { return last_best_buy();  }
        }
    }

    /// @brief Return the current price of the asset using last prices.
    inline Price last_price() const {
        return (sells.last_best_price + buys.last_best_price) / 2;
    }

    /// @brief Return the total volume for the sell side of the book.
    ///
    /// @param price the limit price to get the volume for
    /// @return the volume for the given limit price
    ///
    inline Volume volume_sell(Price price) const {
        return sells.volume_at(price);
    }

    /// @brief Return the total volume for the sell side of the book.
    inline Volume volume_sell() const { return sells.volume; }

    /// @brief Return the volume at the best sell price.
    inline Volume volume_sell_best() const {
        if (sells.best == nullptr) return 0;
        return sells.best->volume;
    }

    /// @brief Return the total volume for the buy side of the book.
    ///
    /// @param price the limit price to get the volume for
    /// @return the volume for the given limit price
    ///
    inline Volume volume_buy(Price price) const {
        return buys.volume_at(price);
    }

    /// @brief Return the total volume for the buy side of the book.
    inline Volume volume_buy() const { return buys.volume; }

    /// @brief Return the volume at the best sell price.
    inline Volume volume_buy_best() const {
        if (buys.best == nullptr) return 0;
        return buys.best->volume;
    }

    /// @brief Return the volume at the given limit price.
    ///
    /// @param price the limit price to get the volume for
    /// @return the volume for the given limit price
    ///
    inline Volume volume(Price price) const {
        return buys.volume_at(price) + sells.volume_at(price);
    }

    /// @brief Return the total volume for the book.
    inline Volume volume() const { return sells.volume + buys.volume; }

    /// @brief Return the total number of orders at the given limit price.
    ///
    /// @param price the limit price to get the volume for
    /// @return the volume for the given limit price
    ///
    inline Count count_at(Price price) const {
        return buys.count_at(price) + sells.count_at(price);
    }

    /// @brief Return the total number of orders on the sell-side of the book.
    inline Count count_sell() const { return sells.count; }

    /// @brief Return the total number of orders on the buy-side of the book.
    inline Count count_buy() const { return buys.count; }

    /// @brief Return the total number of orders in the book.
    inline Count count() const { return sells.count + buys.count; }

    /// @brief Check if this book crosses another book.
    ///
    /// @param other the other market to compare against.
    /// @param spread the minimal spread to consider the markets crossed
    /// @returns true if the best sell on this market is lower than the best
    /// buy on the other market
    ///
    inline bool does_cross(const LimitOrderBook& other, Quantity spread = 0) const {
        // check if A has a sell price
        if (best_sell() == 0)
            return false;
        // check for overflow
        if (best_sell() > std::numeric_limits<Price>::max() - spread)
            return false;
        // check if A is selling for less than B is buying
        if (best_sell() + spread < other.best_buy())
            return true;
        return false;
    }

    /// @brief Write the limit order book to the given output stream.
    ///
    /// @param stream the stream to write the limit order book metadata to
    /// @param book the limit order book to write to the stream
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const LimitOrderBook& book
    ) {
        stream << "| Buy                     | Sell                    |" << std::endl;
        stream << "|-------------------------|-------------------------|" << std::endl;
        stream << "| Count | Volume  | Price | Price | Volume  | Count |" << std::endl;
        stream << "|:------|:--------|:------|:------|:--------|:------|" << std::endl;
        stream
            << "| " << std::setw(6) << std::left << book.count_buy()
            << "| " << std::setw(8) << std::left << book.volume_buy()
            << "| " << std::setw(6) << std::left << book.last_best_buy()
            << "| " << std::setw(6) << std::left << book.last_best_sell()
            << "| " << std::setw(8) << std::left << book.volume_sell()
            << "| " << std::setw(6) << std::left << book.count_sell()
            << "|";
        return stream;
    }
};

}  // namespace LOB

}  // namespace DataFeed

#endif  // DATA_FEED_LIMIT_ORDER_BOOK_LIMIT_ORDER_BOOK_HPP
