// A data feed handler that writes LOB::LimitOrderBook data in csv file format.
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

#ifndef DATA_FEED_HANDLERS_CSV_HPP
#define DATA_FEED_HANDLERS_CSV_HPP

#include "data_feed/receiver.hpp"
#include <ostream>

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// Logic for handling incoming messages from the Receiver.
namespace Handlers {

/// A Receiver handler that writes LOB::LimitOrderBook event data in csv format.
class CSV {
 private:
    /// the output stream to dump the data to
    std::ostream& stream;

    /// Print the CSV header for the file.
    inline void print_header() {
        stream
            << "event_time,"
            << "message_time,"
            << "receipt_time,"
            << "buy_volume,"
            << "buy_size,"
            << "buy_price,"
            << "sell_volume,"
            << "sell_size,"
            << "sell_price,"
            << "sell_adds,"
            << "buy_adds,"
            << "cancels,"
            << "sell_trades,"
            << "buy_trades"
            << std::endl;
    }

    /// Print the message header to the console.
    ///
    /// @param header the message header to print to the console
    ///
    inline void print(const Messages::Header& header) {
        stream
            << header.sequence << ","
            << header.time << ","
            << Clock::get_time() << ",";
    }

    /// Print the limit order book to the console.
    ///
    /// @param book the book to print to the console
    ///
    inline void print(const LOB::LimitOrderBook& book) {
        stream
            << book.volume_buy() << ","
            << book.count_buy() << ","
            << book.last_best_buy() << ","
            << book.volume_sell() << ","
            << book.count_sell() << ","
            << book.last_best_sell();
    }

 public:
    /// @brief Initialize a new data feed CSV handler.
    ///
    /// @param stream_ the stream to output the data to
    ///
    CSV(std::ostream& stream_ = std::cout) : stream(stream_) { print_header(); }

    /// @brief Handle a start of session message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<CSV>* receiver, const Messages::StartOfSession& message) {
        print(message.header);
        print(receiver->get_book());
        stream << ",0,0,0,0,0" << std::endl;
    }

    /// @brief Handle an end of session message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<CSV>* receiver, const Messages::EndOfSession& message) {
        print(message.header);
        print(receiver->get_book());
        stream << ",1,1,1,1,1" << std::endl;
    }

    /// @brief Handle a clear book message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<CSV>* receiver, const Messages::Clear& message) {
        std::cerr << "CSV::did_receive() - clear message" << std::endl;
    }

    /// @brief Handle an add order message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<CSV>* receiver, const Messages::AddOrder& message) {
        if (not receiver->is_session_active())
            return;
        print(message.header);
        print(receiver->get_book());
        // add order
        auto is_buy = DataFeed::side_to_bool(message.side);
        stream << "," << !is_buy << "," << is_buy << ",0,0,0" << std::endl;
    }

    /// @brief Handle a delete order message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<CSV>* receiver, const Messages::DeleteOrder& message) {
        if (not receiver->is_session_active())
            return;
        print(message.header);
        print(receiver->get_book());
        // delete order
        stream << ",0,0,1,0,0" << std::endl;

    }

    /// @brief Handle a trade message.
    ///
    /// @param receiver the receiver that is handling the message
    /// @param message the message to handle
    ///
    inline void did_receive(Receiver<CSV>* receiver, const Messages::Trade& message) {
        if (not receiver->is_session_active())
            return;
        print(message.header);
        print(receiver->get_book());
        // trade
        auto is_buy = DataFeed::side_to_bool(message.side);
        stream << ",0,0,0," << !is_buy << "," << is_buy << std::endl;
    }
};

}  // namespace Handlers

}  // namespace DataFeed

#endif    // DATA_FEED_HANDLERS_CSV_HPP
