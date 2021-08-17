// A class for multi-casting limit order book depth messages.
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

#ifndef DATA_FEED_SENDER_HPP
#define DATA_FEED_SENDER_HPP

#include <functional>
#include <utility>
#include <asio.hpp>
#include "exceptions.hpp"
#include "messages.hpp"

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// A class for multi-casting depth of book messages from a LOB::LimitOrderBook.
class Sender {
 private:
    /// the endpoint the server is running on
    asio::ip::udp::endpoint endpoint;
    /// the socket the server is running on
    asio::ip::udp::socket socket;
    /// the current message sequence number
    SequenceNumber sequence = 1;
    /// the queue of output messages
    Messages::PacketQueue output_buffer;
    /// the size of the output buffer
    uint32_t output_buffer_size = 0;

    /// @brief Start sending messages from the queue until it is empty.
    void send_message() {
        socket.async_send_to(
            asio::buffer(output_buffer.front(), sizeof(Messages::Packet)),
            endpoint,
            [this](const std::error_code& error, std::size_t) {
                if (error)  // an error occurred while sending the message
                    throw Exception("DataFeed::Sender::send - " + error.message());
                // remove the sent message from the queue
                output_buffer.pop_front();
                // send another message if there is more work
                if (--output_buffer_size) send_message();
            }
        );
    }

 public:
    /// @brief Initialize a new data feed multi-cast server.
    ///
    /// @param context the IO context to run the server on
    /// @param group the multi-cast group address in the range
    ///              [224.0.0.0, 239.255.255.255]
    /// @param port the port to send the packets to
    ///
    Sender(
        asio::io_context& context,
        const asio::ip::address& group,
        uint16_t port
    ) : endpoint(group, port), socket(context, endpoint.protocol()) { }

    /// @brief Set the TTL hops for messages rendered by the sender.
    ///
    /// @param ttl the number of hops to limit the multi-cast packets to
    ///
    inline void set_ttl(uint32_t ttl) {
        socket.set_option(asio::ip::multicast::hops(ttl));
    }

    /// @brief Enable loop-back for the sender.
    inline void enable_loopback() {
        socket.set_option(asio::ip::multicast::enable_loopback(true));
    }

    /// @brief Set the outbound interface for the sender.
    ///
    /// @param interface the IPv4 address of the interface to bind to
    ///
    inline void set_outbound_interface(const asio::ip::address_v4& interface) {
        socket.set_option(asio::ip::multicast::outbound_interface(interface));
    }

    /// @brief Return the sequence number at this sender.
    ///
    /// @returns the sequence number of the next message to send
    ///
    inline SequenceNumber get_sequence() const { return sequence; }

    /// @brief Return the group that this sender is sending to.
    ///
    /// @returns the UDP multi-cast group that this sender sends to
    ///
    inline asio::ip::address group() const { return endpoint.address(); }

    /// @brief Return the port this sender is running at.
    ///
    /// @returns the UDP port this service is running at
    ///
    inline uint16_t port() const { return endpoint.port(); }

    /// @brief Return the current size of the output buffer (number of messages).
    ///
    /// @returns the number of message currently in the output buffer
    ///
    inline uint32_t get_output_buffer_size() const { return output_buffer_size; }

    /// @brief Send a message.
    ///
    /// @tparam T the class of the message to send
    /// @param ...args the initialization parameters for the object
    ///
    template <class T, typename ...Args>
    inline void send(Args && ...args) {
        // put an empty packet on the back of the queue
        output_buffer.emplace_back();
        // initialize the message data on the buffer using forwarded arguments
        // and placement new to construct the object in the buffer memory
        new (&output_buffer.back()) T{std::forward<Args>(args)..., sequence++, Clock::get_time()};
        // increment the number of messages available now the buffer is ready
        // start the write event loop if this is the first message
        if (++output_buffer_size == 1) send_message();
    }
};

}    // namespace DataFeed

#endif    // DATA_FEED_SENDER_HPP
