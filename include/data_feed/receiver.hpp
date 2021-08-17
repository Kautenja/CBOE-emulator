// A class for handling limit order book depth messages from multi-cast.
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

#ifndef DATA_FEED_RECEIVER_HPP
#define DATA_FEED_RECEIVER_HPP

#include <functional>
#include <iostream>
#include <asio.hpp>
#include "exceptions.hpp"
#include "limit_order_book/limit_order_book.hpp"
#include "messages.hpp"

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// @brief A multi-cast receiver for recreating a LOB::LimitOrderBook from
///        network messages.
/// @tparam Handler a callback type for handling messages from the receiver
///
template<typename Handler>
class Receiver {
 private:
    /// the socket to receive messages on
    asio::ip::udp::socket socket;
    /// the endpoint of the sender
    asio::ip::udp::endpoint sender_endpoint;
    /// the queue of output messages
    Messages::PacketQueue input_buffer;
    /// the re-built limit order book for the instrument
    LOB::LimitOrderBook book;
    /// whether the trading session is active
    bool _is_session_active = false;
    /// the handler to pass messages to
    Handler& handler;
    /// the last received sequence number
    SequenceNumber last_sequence = 0;

    /// @brief Asynchronously receive a message.
    void read_message() {
        input_buffer.emplace_back();
        socket.async_receive_from(
            asio::buffer(input_buffer.back(), sizeof(Messages::Packet)),
            sender_endpoint,
            [this](const std::error_code& error, std::size_t) {
                if (error)  // an error occurred while reading a message
                    throw Exception("DataFeed::read_message - " + error.message());
                // copy the message from the front of the queue and pop it off
                auto message = input_buffer.front();
                input_buffer.pop_front();
                // check if the sequence number is out of order
                if (reinterpret_cast<Messages::Header&>(message).sequence != ++last_sequence) {
                    std::cerr << "DataFeed::Receiver::read_message - out of sequence with server" << std::endl;
                    std::cerr << "DataFeed::Receiver::read_message - got sequence: " << reinterpret_cast<Messages::Header&>(message).sequence << std::endl;
                    std::cerr << "DataFeed::Receiver::read_message - expected sequence: " << last_sequence - 1 << std::endl;
                    last_sequence = reinterpret_cast<Messages::Header&>(message).sequence;
                }
                // read another message from the server
                read_message();
                // unwrap and handle the data using the template ID
                switch (reinterpret_cast<Messages::Header&>(message).uid) {
                    case Messages::MessageID::StartOfSession: { handle(reinterpret_cast<Messages::StartOfSession&>(message));  break; }
                    case Messages::MessageID::EndOfSession:   { handle(reinterpret_cast<Messages::EndOfSession&>(message));    break; }
                    case Messages::MessageID::Clear:          { handle(reinterpret_cast<Messages::Clear&>(message));           break; }
                    case Messages::MessageID::AddOrder:       { handle(reinterpret_cast<Messages::AddOrder&>(message));        break; }
                    case Messages::MessageID::DeleteOrder:    { handle(reinterpret_cast<Messages::DeleteOrder&>(message));     break; }
                    case Messages::MessageID::Trade:          { handle(reinterpret_cast<Messages::Trade&>(message));           break; }
                    default:                                  { handle(reinterpret_cast<Messages::Header&>(message));          break; }
                }
            }
        );
    }

    /// @brief Handle a start of session message.
    ///
    /// @param message the message to handle
    ///
    inline void handle(const Messages::StartOfSession& message) {
        // set the trading session to active
        _is_session_active = true;
        // pass the message to the handler
        handler.did_receive(this, message);
    }

    /// @brief Handle an end of session message.
    ///
    /// @param message the message to handle
    ///
    inline void handle(const Messages::EndOfSession& message) {
        // set the trading session to inactive
        _is_session_active = false;
        // pass the message to the handler
        handler.did_receive(this, message);
    }

    /// @brief Handle a clear message.
    ///
    /// @param message the message to handle
    ///
    inline void handle(const Messages::Clear& message) {
        // clear all the orders in the book
        book.clear();
        // pass the message to the handler
        handler.did_receive(this, message);
    }

    /// @brief Handle an add order message.
    ///
    /// @param message the message to handle
    ///
    inline void handle(const Messages::AddOrder& message) {
        // add the order to the book
        book.limit(side_to_LOB_side(message.side), message.uid, message.quantity, message.price);
        // pass the message to the handler
        handler.did_receive(this, message);
    }

    /// @brief Handle a delete order message.
    ///
    /// @param message the message to handle
    ///
    inline void handle(const Messages::DeleteOrder& message) {
        if (not book.has(message.uid)) {  // the limit order was not found
            std::cerr << "DataFeed::Receiver::handle(DeleteOrder) - received delete for non-existent order: " << message.uid << std::endl;
            // TODO: handle error
            return;
        }
        // delete the order in the book
        book.cancel(message.uid);
        // pass the message to the handler
        handler.did_receive(this, message);
    }

    /// @brief Handle a trade message.
    ///
    /// @param message the message to handle
    ///
    inline void handle(const Messages::Trade& message) {
        if (not book.has(message.uid)) {  // the limit order was not found
            std::cerr << "DataFeed::Receiver::handle(Trade) - received trade for non-existent order: " << message.uid << std::endl;
            // TODO: handle error
            return;
        }
        // reduce the size of the given order
        // (removes order if remaining size is 0)
        book.reduce(message.uid, message.quantity);
        // pass the message to the handler
        handler.did_receive(this, message);
    }

    /// @brief Handle a response with invalid ID.
    ///
    /// @param header the header of the invalid response
    ///
    inline void handle(const Messages::Header& header) {
        throw Exception("Receiver::handle(Header) - received message with invalid header " + header.to_string());
    }

 public:
    /// @brief Initialize a new data feed receiver.
    ///
    /// @param io_context the IO context to listen on
    /// @param listen the host address to listen on
    /// @param group the address of the multi-cast address to join
    /// @param port the port to listen for packets on
    /// @param handler_ a callback for handling messages from the receiver
    /// @param reuse_enabled whether reuse is enabled for the listening host
    ///
    Receiver(
        asio::io_context& io_context,
        const asio::ip::address& listen,
        const asio::ip::address& group,
        uint16_t port,
        Handler& handler_,
        bool reuse_enabled = true
    ) : socket(io_context), handler(handler_) {
        // Create the socket so that multiple may be bound to the same address.
        asio::ip::udp::endpoint listen_endpoint(listen, port);
        socket.open(listen_endpoint.protocol());
        socket.set_option(asio::ip::udp::socket::reuse_address(reuse_enabled));
        socket.bind(asio::ip::udp::endpoint(group, port));
        socket.set_option(asio::ip::multicast::join_group(group));
        // detect if the compilation platform is MacOS and don't try to set
        // the buffer, if so.
#if __APPLE__
#else
        // set the size of the receive buffer to the maximum allowed size
        auto size = std::numeric_limits<uint32_t>::max();
        auto recv_buffer_size = asio::socket_base::receive_buffer_size(size);
        socket.set_option(recv_buffer_size);
#endif
        // read the first message
        read_message();
    }

    /// @brief Return the limit order book for this receiver.
    ///
    /// @returns a constant reference to this receiver's limit order book
    ///
    inline const LOB::LimitOrderBook& get_book() { return book; }

    /// @brief Return whether the trading session is active.
    ///
    /// @returns true if the trading session has been started by the server
    ///
    inline bool is_session_active() const { return _is_session_active; }

    /// @brief Return the handler connected to this feed receiver.
    ///
    /// @returns a reference to the handler associated with this receiver
    ///
    inline const Handler& get_handler() const { return handler; }
};

}    // namespace DataFeed

#endif    // DATA_FEED_RECEIVER_HPP
