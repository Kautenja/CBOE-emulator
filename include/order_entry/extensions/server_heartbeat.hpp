// A context extension for printing data from an order entry server.
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

#ifndef ORDER_ENTRY_EXTENSIONS_SERVER_HEARTBEAT_HPP
#define ORDER_ENTRY_EXTENSIONS_SERVER_HEARTBEAT_HPP

#include "order_entry/server.hpp"
#include "data_feed/sender.hpp"
#include <asio.hpp>
#include <cstdint>
#include <chrono>
#include <ostream>

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// Extensions to the Client / Server asio context.
namespace Extensions {

/// A Server context extension for printing data using a heartbeat timer.
class ServerHeartbeat {
 private:
    /// the server the CLI is interacting with
    Server& server;
    /// the data feed the server is hosting
    DataFeed::Sender& feed;
    /// a timer for outputting to the command line
    asio::steady_timer timer;
    /// the amount of time between heart beats in milliseconds
    uint16_t time;
    /// the output stream to dump the data to
    std::ostream& stream;

    /// @brief Start the heartbeat service.
    void start() {
        timer.expires_after(std::chrono::milliseconds(time));
        timer.async_wait([this](const std::error_code& error) {
            if (error) throw error;
            // clear the terminal
            stream << "\033[2J" << "\033[1;1H" << std::endl;
            // print the server information
            stream << "Order Entry: ";
            stream << server.address() << ":" << server.port() << std::endl;
            // print the feed information
            stream << "Data Feed: ";
            stream << feed.group() << ":" << feed.port() << std::endl;
            stream << "\tqueued: " << feed.get_output_buffer_size() << std::endl;
            // print the authorizer information
            stream << std::endl;
            stream << server.get_authorizer() << std::endl;
            // print the top of book information
            stream << server.get_book() << std::endl;
            // start the timer again
            start();
        });
    }

 public:
    /// @brief Initialize a new receiver heartbeat.
    ///
    /// @param context the ASIO context to create the heartbeat with
    /// @param server_ the server to read data from
    /// @param feed_ the data feed to read data from
    /// @param time_ the number of milliseconds between beats
    /// @param stream_ the output stream to write the data to
    ///
    ServerHeartbeat(
        asio::io_context& context,
        Server& server_,
        DataFeed::Sender& feed_,
        uint16_t time_ = 300,
        std::ostream& stream_ = std::cout
    ) :
        server(server_),
        feed(feed_),
        timer(context),
        time(time_),
        stream(stream_) { start(); }
};

}  // namespace Extensions

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_EXTENSIONS_SERVER_HEARTBEAT_HPP
