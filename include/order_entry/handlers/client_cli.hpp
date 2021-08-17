// A command line interface to the client service.
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

#ifndef HANDLERS_CLIENT_CLI_HPP
#define HANDLERS_CLIENT_CLI_HPP

#include "order_entry/client.hpp"
#include <regex>
#include <sstream>
#include <iostream>

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// Logic for handling CLI messages for the Client / Server.
namespace Handlers {

/// A CLI handler for interacting a Client.
class ClientCLI {
 private:
    /// the client the CLI is interacting with
    Client& client;

    /// a regular expression for the login command
    const std::regex LOGIN = std::regex("^login ([a-zA-Z0-9]*) ([a-zA-Z0-9]*)\n$");

    /// @brief Handle a login command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void login(const std::string& line) {
        std::smatch match;
        std::regex_search(line, match, LOGIN);
        // get the username and password
        auto username = make_username(match.str(1));
        auto password = make_password(match.str(2));
        // send the request
        client.send<Messages::LoginRequest>(username, password);
    }

    /// a regular expression for the logout command
    const std::regex LOGOUT = std::regex("^logout\n$");

    /// @brief Handle a logout command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void logout(const std::string& line) {
        client.send<Messages::LogoutRequest>();
    }

    /// a regular expression for the market order command
    const std::regex MARKET = std::regex("^market (buy|sell) ([0-9]*)\n$");

    /// @brief Handle a market order command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void market_order(const std::string& line) {
        std::smatch match;
        std::regex_search(line, match, MARKET);
        // determine the side of the order
        auto side = bool_to_side(match.str(1).compare("sell"));
        // determine the size of the order
        Quantity size;
        std::istringstream(match.str(2)) >> size;
        // place the order
        client.send<Messages::OrderRequest>(Messages::ORDER_PRICE_MARKET, size, side);
    }

    /// a regular expression for the limit order command
    const std::regex LIMIT = std::regex("^limit (buy|sell) ([0-9]*) ([0-9]*)\n$");

    /// @brief Handle a limit order command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void limit_order(const std::string& line) {
        std::smatch match;
        std::regex_search(line, match, LIMIT);
        // determine the side of the order
        auto side = bool_to_side(match.str(1).compare("sell"));
        // determine the size of the order
        Quantity size;
        std::istringstream(match.str(2)) >> size;
        // determine the price of the order
        Price price;
        std::istringstream(match.str(3)) >> price;
        // place the order
        client.send<Messages::OrderRequest>(price, size, side);
    }

    /// a regular expression for the cancel limit order command
    const std::regex CANCEL = std::regex("^cancel ([0-9]*)\n$");

    /// @brief Handle a cancel order command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void cancel_order(const std::string& line) {
        std::smatch match;
        std::regex_search(line, match, CANCEL);
        // determine the order ID of the order
        OrderID order_id;
        std::istringstream(match.str(1)) >> order_id;
        client.send<Messages::CancelRequest>(order_id);
    }

    /// a regular expression for the replace limit order command
    const std::regex REPLACE = std::regex("^replace ([0-9]*) (buy|sell) ([0-9]*) ([0-9]*)\n$");

    /// @brief Handle a replace order command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void replace_order(const std::string& line) {
        std::smatch match;
        std::regex_search(line, match, REPLACE);
        // determine the order ID of the order
        OrderID order_id;
        std::istringstream(match.str(1)) >> order_id;
        // determine the side of the order
        auto side = bool_to_side(match.str(2).compare("sell"));
        // determine the size of the order
        Quantity size;
        std::istringstream(match.str(3)) >> size;
        // determine the price of the order
        Price price;
        std::istringstream(match.str(4)) >> price;
        // place the order
        client.send<Messages::ReplaceRequest>(order_id, price, size, side);
    }

    /// a regular expression for the purge command
    const std::regex PURGE = std::regex("^purge\n$");

    /// @brief Handle a purge orders command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void purge_orders(const std::string& line) {
        // std::smatch match;
        // std::regex_search(line, match, PURGE);
        client.send<Messages::PurgeRequest>();
    }

    /// a regular expression for the print account information command
    const std::regex ACCOUNT = std::regex("^account\n$");

    /// @brief Handle a print account command.
    ///
    /// @param line the string containing the command from the shell
    ///
    inline void print_account(const std::string& line) {
        std::cout << "Account Information:" << '\n'
            << "logged in: " << client.is_logged_in() << '\n'
            << "active:    {";
        for (auto const& i : client.get_active_orders())
            std::cout << i << ",";
        std::cout << '}' << '\n'
            << "shares:    " << client.get_shares() << '\n'
            << "capital:   " << client.get_capital() << '\n';
    }

 public:
    /// @brief Initialize a new client handler for a CLI.
    ///
    /// @param client_ the client to send orders through
    ///
    explicit ClientCLI(Client& client_) : client(client_) { }

    /// @brief Process a command from the console.
    ///
    /// @param line the command to parse
    ///
    void parse(const std::string& line) {
        if (std::regex_match(line, LOGIN))
            login(line);
        else if (std::regex_match(line, LOGOUT))
            logout(line);
        else if (std::regex_match(line, MARKET))
            market_order(line);
        else if (std::regex_match(line, LIMIT))
            limit_order(line);
        else if (std::regex_match(line, CANCEL))
            cancel_order(line);
        else if (std::regex_match(line, REPLACE))
            replace_order(line);
        else if (std::regex_match(line, PURGE))
            purge_orders(line);
        else if (std::regex_match(line, ACCOUNT))
            print_account(line);
        else
            std::cout << "invalid command" << std::endl;
    }
};

}  // namespace Handlers

}  // namespace OrderEntry

#endif  // HANDLERS_CLIENT_CLI_HPP
