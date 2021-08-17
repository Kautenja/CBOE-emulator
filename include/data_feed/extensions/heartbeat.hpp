// A context extension for printing data from a feed using a heartbeat timer.
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

#ifndef DATA_FEED_EXTENSIONS_HEARTBEAT_HPP
#define DATA_FEED_EXTENSIONS_HEARTBEAT_HPP

#include "data_feed/receiver.hpp"
#include <asio.hpp>
#include <cstdint>
#include <chrono>
#include <ostream>

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// Extensions to the Sender / Receiver asio context.
namespace Extensions {

/// A Receiver context extension for printing data using a heartbeat timer.
template<typename T>
class Heartbeat {
 private:
    /// the receiver to get data from
    Receiver<T>& receiver;
    /// the timer for waiting
    asio::steady_timer timer;
    /// the amount of time between heart beats in milliseconds
    uint16_t time;
    /// the output stream to dump the data to
    std::ostream& stream;

    /// @brief Start the heartbeat service.
    void start() {
        // clear the terminal
        stream << "\033[2J" << "\033[1;1H" << std::endl;
        // print the top of book information
        stream << receiver.get_book() << std::endl;
        // set a timer to start the strategy again
        timer.expires_after(std::chrono::milliseconds(time));
        timer.async_wait([this](const std::error_code& error) {
            if (error) throw error;
            start();
        });
    }

 public:
    /// @brief Initialize a new receiver heartbeat.
    ///
    /// @param context the ASIO context to create the heartbeat with
    /// @param receiver_ the receiver to read data from
    /// @param time_ the number of milliseconds between beats
    /// @param stream_ the output stream to write the data to
    ///
    Heartbeat(
        asio::io_context& context,
        Receiver<T>& receiver_,
        uint16_t time_ = 300,
        std::ostream& stream_ = std::cout
    ) : receiver(receiver_), timer(context), time(time_), stream(stream_) {
        start();
    }
};

}  // namespace Extensions

}  // namespace DataFeed

#endif  // DATA_FEED_EXTENSIONS_HEARTBEAT_HPP
