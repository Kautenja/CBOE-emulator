// An account on the server.
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

#ifndef ORDER_ENTRY_SYSTEM_ACCOUNT_HPP
#define ORDER_ENTRY_SYSTEM_ACCOUNT_HPP

#include "limit_order_book/limit_order_book.hpp"
#include "messages.hpp"
#include <string>
#include <ostream>
#include <iomanip>

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// @brief A subclass of the LOB::Account that manages client state on the
///        market server.
/// @tparam Handler a callback handler type for passing trade event data to
///
template<typename Handler>
struct SystemAccount : public LOB::Account {
    /// the user name for the account
    std::string username = "";
    /// the password for the account
    std::string password = "";
    /// the connection for this account
    Handler* handler = nullptr;
    /// whether the account is currently logged in
    bool is_connected = false;

    /// @brief Initialize a new system account.
    SystemAccount() : LOB::Account() { }

    /// @brief Initialize a new system account with given handler.
    ///
    /// @param username_ the username for the account
    /// @param password_ the password for the account
    ///
    SystemAccount(const std::string& username_, const std::string& password_) :
        LOB::Account(),
        username(username_),
        password(password_) { }

    /// @brief Initialize a new system account with given handler.
    ///
    /// @param handler_ a pointer to the handler for the system account
    ///
    explicit SystemAccount(Handler* handler_) :
        LOB::Account(),
        handler(handler_) { }

    /// @brief Return the header for the account table.
    ///
    /// @returns the header value for the system account table
    ///
    inline static const std::string table_header() {
        return "\
| UN   | Password     | Auth | Shares    | Capital   | Orders \n\
|:-----|:-------------|:-----|:----------|:----------|:----------|\
";
    }

    /// @brief Write the data from the account to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param account the account to write to the stream
    /// @returns the stream with data from the account written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const SystemAccount& account
    ) {
        // write the basic scalar values
        stream
            << "| " << std::setw(5)  << std::left << account.username
            << "| " << std::setw(13) << std::left << account.password
            << "| " << std::setw(5)  << std::left << account.is_connected
            << "| " << std::setw(10) << std::left << account.shares
            << "| " << std::setw(10) << std::left << account.capital
            << "| " << std::setw(10) << std::left << account.orders.size()
            << "|";
        return stream;
    }

    /// @brief Fill a limit order.
    ///
    /// @param limit the limit order that is being filled
    /// @param market the market order that fills the limit order
    ///
    inline void limit_fill(LOB::Order* limit, LOB::Order* market) override {
        LOB::Account::limit_fill(limit, market);
        if (handler == nullptr) return;
        handler->trade(
            limit->uid,
            limit->price,
            limit->quantity,
            0,  // filled, no quantity left
            bool_to_side(static_cast<bool>(limit->side))
        );
    }

    /// @brief Partially fill a limit order.
    ///
    /// @param limit the limit order that is being partially filled
    /// @param market the market order that partially fills the limit order
    ///
    inline void limit_partial(LOB::Order* limit, LOB::Order* market) override {
        LOB::Account::limit_partial(limit, market);
        if (handler == nullptr) return;
        handler->trade(
            limit->uid,
            limit->price,
            market->quantity,
            limit->quantity,
            bool_to_side(static_cast<bool>(limit->side))
        );
    }

    /// @brief Fill a market order.
    ///
    /// @param limit the limit order that fills the market order
    /// @param market the market order being filled
    ///
    inline void market_fill(LOB::Order* limit, LOB::Order* market) override {
        LOB::Account::market_fill(limit, market);
        if (handler == nullptr) return;
        handler->trade(
            0,  // market->uid,
            limit->price,
            market->quantity,
            0,  // filled, no quantity left
            bool_to_side(static_cast<bool>(market->side))
        );
    }

    /// @brief Partially fill a market order.
    ///
    /// @param limit the limit order that partially fills the market order
    /// @param market the market order being partially filled
    ///
    inline void market_partial(LOB::Order* limit, LOB::Order* market) override {
        LOB::Account::market_partial(limit, market);
        if (handler == nullptr) return;
        handler->trade(
            0,  // market->uid,
            limit->price,
            limit->quantity,
            market->quantity,
            bool_to_side(static_cast<bool>(market->side))
        );
    }
};

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_SYSTEM_ACCOUNT_HPP
