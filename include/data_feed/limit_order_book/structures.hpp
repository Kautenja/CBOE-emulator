// Types and structures for the LimitOrderBook: order, limit
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

#ifndef DATA_FEED_LIMIT_ORDER_BOOK_STRUCTURES_HPP
#define DATA_FEED_LIMIT_ORDER_BOOK_STRUCTURES_HPP

#include "doubly_linked_list.hpp"
#include "binary_search_tree.hpp"
#include <cstdint>

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// Logic for maintaining a continuous double auction via a limit-order book.
namespace LOB {

/// the possible sides for the LimitTree
enum class Side : bool { Sell = false, Buy = true };

/// @brief Return the opposite side using the invert operator.
///
/// @param side the side to invert
/// @returns the opposite of the given side
///
inline Side operator!(Side side) {
    return static_cast<Side>(!static_cast<bool>(side));
}

/// a type for order IDs
typedef uint64_t UID;
/// a type for order quantities
typedef uint32_t Quantity;
/// a type for order prices
typedef uint64_t Price;

// forward declare the `Limit` structure so `Order` can reference it
struct Limit;

/// A single order in the LimitOrderBook.
struct Order : DLL::Node {
    /// the day-unique ID for this particular order
    const UID uid = 0;
    /// a boolean determining whether the order id a buy (true) or sell (false)
    const Side side = Side::Sell;
    /// the number of shares in the order
    Quantity quantity = 0;
    /// the limit price for the order (market price if market order)
    const Price price = 0;
    /// the limit this order falls under
    Limit* limit = nullptr;

    /// Initialize a new order data.
    Order() : DLL::Node() { }

    /// @brief Initialize a new order data.
    ///
    /// @param uid_ the unique identifier for the order
    /// @param side_ the side of the order: buy=true, sell=false
    /// @param quantity_ the number of shares to buy or sell
    /// @param price_ the limit/market price for the order
    ///
    Order(UID uid_, Side side_, Quantity quantity_, Price price_) :
        DLL::Node(),
        uid(uid_),
        side(side_),
        quantity(quantity_),
        price(price_) { }
};

/// a type for limit price order counts
typedef uint32_t Count;
/// a type for limit total volume
typedef uint64_t Volume;

/// A price limit containing a FIFO queue of Order objects.
struct Limit : BST::Node<Price> {
    /// the number of orders at this limit price
    Count count = 0;
    /// padding for byte alignment
    const uint32_t _padding = 0;
    /// the total amount of volume at this limit price (sum of order shares)
    Volume volume = 0;
    /// the first order in the queue (next to execute)
    Order* order_head = nullptr;
    /// the last order in the queue (last to execute)
    Order* order_tail = nullptr;

    /// @brief Initialize a new limit data.
    Limit() : BST::Node<Price>() { }

    /// @brief Initialize a new limit data.
    ///
    /// @param order the initial order to create the limit with
    ///
    explicit Limit(Order* order) :
        BST::Node<Price>(order->price),
        count(1),
        volume(order->quantity),
        order_head(order),
        order_tail(order) { }
};

}  // namespace LOB

}  // namespace DataFeed

#endif  // DATA_FEED_LIMIT_ORDER_BOOK_STRUCTURES_HPP
