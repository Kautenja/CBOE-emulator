// A data feed handler that writes every message to an output stream.
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

#ifndef DATA_FEED_HANDLERS_ECHO_HPP
#define DATA_FEED_HANDLERS_ECHO_HPP

#include "data_feed/receiver.hpp"
#include <ostream>

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// Logic for handling incoming messages from the Receiver.
namespace Handlers {

/// A Receiver handler that writes every message to an output stream.
class Echo {
 private:
    /// the output stream to dump the data to
    std::ostream& stream;

 public:
    /// @brief Initialize a new data feed echo handler.
    ///
    /// @param stream_ the stream to output the data to
    ///
    Echo(std::ostream& stream_ = std::cout) : stream(stream_) { }

    /// @brief Handle a start of session message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<Echo>* receiver, const Messages::StartOfSession& message) {
        stream << message << std::endl;
        stream << receiver->get_book() << std::endl;
    }

    /// @brief Handle an end of session message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<Echo>* receiver, const Messages::EndOfSession& message) {
        stream << message << std::endl;
        stream << receiver->get_book() << std::endl;
    }

    /// @brief Handle a clear book message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<Echo>* receiver, const Messages::Clear& message) {
        stream << message << std::endl;
        stream << receiver->get_book() << std::endl;
    }

    /// @brief Handle an add order message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<Echo>* receiver, const Messages::AddOrder& message) {
        stream << message << std::endl;
        stream << receiver->get_book() << std::endl;
    }

    /// @brief Handle a delete order message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<Echo>* receiver, const Messages::DeleteOrder& message) {
        stream << message << std::endl;
        stream << receiver->get_book() << std::endl;
    }

    /// @brief Handle a trade message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<Echo>* receiver, const Messages::Trade& message) {
        stream << message << std::endl;
        stream << receiver->get_book() << std::endl;
    }
};

}  // namespace Handlers

}  // namespace DataFeed

#endif  // DATA_FEED_HANDLERS_ECHO_HPP
