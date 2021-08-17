// Types and structures for the LimitOrderBook: order, limit, account
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

#ifndef ORDER_ENTRY_LIMIT_ORDER_BOOK_STRUCTURES_HPP
#define ORDER_ENTRY_LIMIT_ORDER_BOOK_STRUCTURES_HPP

#include "doubly_linked_list.hpp"
#include "binary_search_tree.hpp"
#include <cstdint>
#include <set>

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// Logic for maintaining a continuous double auction via a limit-order book.
namespace LOB {

// ---------------------------------------------------------------------------
// MARK: Types
// ---------------------------------------------------------------------------

/// the possible sides for the LimitTree
enum class Side : bool { Sell = false, Buy = true };

/// @brief Return the opposite side using the invert operator
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

// ---------------------------------------------------------------------------
// MARK: Order
// ---------------------------------------------------------------------------

// forward declare the `Account` structure so `Order` can reference it
struct Account;
// forward declare the `Limit` structure so `Order` can reference it
struct Limit;

/// A single order in the LimitOrderBook.
struct Order : DLL::Node {
    /// the day-unique ID for this particular order
    const UID uid = 0;
    /// a boolean determining whether the order id a buy (true) or sell (false)
    const Side side = Side::Sell;
    /// the quantity of the order, i.e., the number of shares
    Quantity quantity = 0;
    /// the limit price for the order (market price if market order)
    const Price price = 0;
    /// the limit this order falls under
    Limit* limit = nullptr;
    /// the account this order belongs to
    Account* account = nullptr;

    /// @brief Initialize a new order data.
    Order() : DLL::Node() { }

    /// @brief Initialize a new order data.
    ///
    /// @param uid_ the unique identifier for the order
    /// @param side_ the side of the order: buy=true, sell=false
    /// @param quantity_ the number of shares to buy or sell
    /// @param price_ the limit/market price for the order
    /// @param limit_ the limit to create the order with
    /// @param account_ the account associated with this order
    ///
    Order(
        UID uid_,
        Side side_,
        Quantity quantity_,
        Price price_,
        Limit* limit_ = nullptr,
        Account* account_ = nullptr
    ) :
        DLL::Node(),
        uid(uid_),
        side(side_),
        quantity(quantity_),
        price(price_),
        limit(limit_),
        account(account_) { }
};

// ---------------------------------------------------------------------------
// MARK: Limit
// ---------------------------------------------------------------------------

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
    /// the total amount of volume at this limit price
    Volume volume = 0;
    /// the first order in the queue (next to execute)
    Order* order_head = nullptr;
    /// the last order in the queue (last to execute)
    Order* order_tail = nullptr;

    /// @brief Initialize a new limit.
    Limit() : BST::Node<Price>() { }

    /// @brief Initialize a new limit.
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

// ---------------------------------------------------------------------------
// MARK: Account
// ---------------------------------------------------------------------------

/// the number of shares in a user account
typedef int64_t Shares;
/// the amount of capital in a user account
typedef int64_t Capital;

/// A trading account for a LimitOrderBook client.
struct Account {
    /// the number of shares owned by the account
    Shares shares = 0;
    /// the total capital the account has (funds)
    Capital capital = 0;
    /// the set of active orders for the account
    std::set<Order*> orders= {};

    /// @brief Create an account with given values.
    ///
    /// @param shares_ the number of shares owned by the account
    /// @param capital_ the total capital the account has (funds)
    ///
    Account(Shares shares_ = 0, Capital capital_ = 0) :
        shares(shares_),
        capital(capital_) { }

    /// @brief Place a limit order.
    ///
    /// @param order the limit order to place
    ///
    inline void limit(Order* order) { orders.insert(order); }

    /// @brief Cancel a limit order.
    ///
    /// @param order the order to cancel
    ///
    inline void cancel(Order* order) { orders.erase(order); }

    /// @brief Fill a market order on given side with quantity and price.
    ///
    /// @param side the side of the trade
    /// @param quantity the quantity of the trade
    /// @param price the price the trade occurred at
    ///
    inline void fill(Side side, Quantity quantity, Price price) {
        switch (side) {
            case Side::Sell: {
                shares -= quantity;
                capital += quantity * price;
                break;
            }
            case Side::Buy: {
                shares += quantity;
                capital -= quantity * price;
                break;
            }
        }
    }

    /// @brief Fill a limit order.
    ///
    /// @param limit the limit order that is being filled
    /// @param market the market order that fills the limit order
    ///
    inline virtual void limit_fill(Order* limit, Order* market) {
        cancel(limit);
        fill(limit->side, limit->quantity, limit->price);
    }

    /// @brief Partially fill a limit order.
    ///
    /// @param limit the limit order that is being partially filled
    /// @param market the market order that partially fills the limit order
    ///
    inline virtual void limit_partial(Order* limit, Order* market) {
        fill(limit->side, market->quantity, limit->price);
    }

    /// @brief Fill a market order.
    ///
    /// @param limit the limit order that fills the market order
    /// @param market the market order being filled
    ///
    inline virtual void market_fill(Order* limit, Order* market) {
        fill(market->side, market->quantity, limit->price);
    }

    /// @brief Partially fill a market order.
    ///
    /// @param limit the limit order that partially fills the market order
    /// @param market the market order being partially filled
    ///
    inline virtual void market_partial(Order* limit, Order* market) {
        fill(market->side, limit->quantity, limit->price);
    }
};

}  // namespace LOB

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_LIMIT_ORDER_BOOK_STRUCTURES_HPP
