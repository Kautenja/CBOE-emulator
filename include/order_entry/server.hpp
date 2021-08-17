// A server for hosting a financial instrument exchange micro-service.
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

#ifndef ORDER_ENTRY_SERVER_HPP
#define ORDER_ENTRY_SERVER_HPP

#include "exceptions.hpp"
#include "limit_order_book/limit_order_book.hpp"
#include "connection.hpp"
#include "authorizer.hpp"
#include <asio.hpp>
#include <nlohmann/json.hpp>
#include <vector>

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// A server that manages multiple client connections for direct market access.
class Server {
 private:
    /// the IO context the server is running at
    asio::io_context& context;
    /// the acceptor for handling new connections
    asio::ip::tcp::acceptor acceptor;
    /// the account authorizer
    Authorizer<Connection> authorizer;
    /// the limit order book for the instrument
    LOB::LimitOrderBook& book;

    /// @brief Start accepting new connections.
    void accept() {
        // create a new connection with a shared pointer
        auto connection = Connection::create(context, authorizer, book);
        // asynchronously accept a new connection
        acceptor.async_accept(
            connection->get_socket(),
            [this, connection](const std::error_code& error) {
                // throw an error if there is an error code
                if (error) throw Exception(error.message());
                // start the connection and accept another connection
                connection->start();
                accept();
            }
        );
    }

 public:
    /// @brief Initialize a new order entry server.
    ///
    /// @param context_ the IO context to run the server with
    /// @param book_ the book for the server
    /// @param port the port to run the service at
    /// @param accounts a vector of JSON objects, each containing account data
    ///
    Server(
        asio::io_context& context_,
        LOB::LimitOrderBook& book_,
        uint16_t port,
        std::vector<nlohmann::json> accounts
    ) :
        context(context_),
        acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        authorizer(),
        book(book_) {
        for (auto user : accounts) {  // iterate over the accounts
            auto username = user["username"].get<std::string>();
            auto password = user["password"].get<std::string>();
            // create a new user
            authorizer.new_user(username, password);
        }
        // start accepting connections
        accept();
    }

    /// @brief Return the address the server is running at.
    ///
    /// @returns the IP address for connecting to the service
    ///
    inline asio::ip::address address() const {
        return acceptor.local_endpoint().address();
    }

    /// @brief Return the port the server is running at.
    ///
    /// @returns the port for connecting to the service
    ///
    inline uint16_t port() const { return acceptor.local_endpoint().port(); }

    /// @brief Return the authorizer associated with this server.
    ///
    /// @returns the authorizer used to login client connections
    ///
    inline const Authorizer<Connection>& get_authorizer() { return authorizer; }

    /// @brief Return the limit order book for this server.
    ///
    /// @returns the limit order book that this server is maintaining
    ///
    inline const LOB::LimitOrderBook& get_book() { return book; }
};

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_SERVER_HPP
