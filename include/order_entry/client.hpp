// A client for sending data to a financial instrument exchange micro-service.
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

#ifndef ORDER_ENTRY_CLIENT_HPP
#define ORDER_ENTRY_CLIENT_HPP

#include <deque>
#include <functional>
#include <iostream>
#include <string>
#include <set>
#include <asio.hpp>
#include "exceptions.hpp"
#include "messages.hpp"
#include "clock.hpp"

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// a type for keeping order IDs in an unordered set
typedef std::set<OrderID> OrderIDSet;
/// a type for storing the amount of shares a client has
typedef int32_t ClientShares;
/// a type for storing the amount of capital a client has
typedef int64_t ClientCapital;

/// A client for interacting with the direct market access server.
class Client {
 private:
    /// the socket the client is running on
    asio::ip::tcp::socket socket;
    /// the queue of input messages
    Messages::PacketQueue input_buffer;
    /// the queue of output messages
    Messages::PacketQueue output_buffer;
    /// the size of the output buffer
    uint32_t output_buffer_size = 0;
    /// the sequence number for the client messages
    SequenceNumber sequence = 0;
    /// whether the client connection is authorized or not
    bool is_authorized = false;
    /// a set of active orders for the client connection
    OrderIDSet active_orders = {};
    /// the number of shares owned by the account
    ClientShares shares = 0;
    /// the total capital the account has (funds)
    ClientCapital capital = 0;

    /// @brief Begin the asynchronous write loop for data in the buffer.
    inline void write_buffer() {
        asio::async_write(
            socket,
            asio::buffer(output_buffer.front(), sizeof(Messages::Packet)),
            [this](const std::error_code& error, std::size_t) {
                if (error)  // an error occurred while sending a message
                    throw Exception("Client::write_buffer - " + error.message());
                // remove the sent message from the queue
                output_buffer.pop_front();
                // send another message if there is more work
                if (--output_buffer_size) write_buffer();
            }
        );
    }

    /// @brief Read data from the socket into the input buffer asynchronously.
    inline void read_buffer() {
        input_buffer.emplace_back();
        asio::async_read(
            socket,
            asio::buffer(input_buffer.back(), sizeof(Messages::Packet)),
            [this](const std::error_code& error, std::size_t) {
                if (error) {  // an error occurred during the read operation
                    if (error == asio::error::eof) {  // connection closed
                        is_authorized = false;
                        throw Exception("Client::read_buffer - Connection to remote host closed!");
                    }
                    throw Exception("Client::read_buffer - " + error.message());
                }
                // copy the message from the front of the queue and pop it off
                auto message = input_buffer.front();
                input_buffer.pop_front();
                // read another message from the server
                read_buffer();
                // unwrap and handle the data using the template ID
                switch (reinterpret_cast<Messages::Header&>(message).uid) {
                    case Messages::MessageID::LoginResponse:   { handle(reinterpret_cast<Messages::LoginResponse&>(message));   break; }
                    case Messages::MessageID::LogoutResponse:  { handle(reinterpret_cast<Messages::LogoutResponse&>(message));  break; }
                    case Messages::MessageID::OrderResponse:   { handle(reinterpret_cast<Messages::OrderResponse&>(message));   break; }
                    case Messages::MessageID::CancelResponse:  { handle(reinterpret_cast<Messages::CancelResponse&>(message));  break; }
                    case Messages::MessageID::ReplaceResponse: { handle(reinterpret_cast<Messages::ReplaceResponse&>(message)); break; }
                    case Messages::MessageID::PurgeResponse:   { handle(reinterpret_cast<Messages::PurgeResponse&>(message));   break; }
                    case Messages::MessageID::TradeResponse:   { handle(reinterpret_cast<Messages::TradeResponse&>(message));   break; }
                    default:                                   { handle(reinterpret_cast<Messages::Header&>(message));          break; }
                }
            }
        );
    }

    /// @brief Handle a login response.
    ///
    /// @param response the login response to handle
    ///
    void handle(const Messages::LoginResponse& response) {
        switch (response.status) {  // handle the message based on its status
            case Messages::LoginResponseStatus::Accepted:          { is_authorized = true;  break; }
            case Messages::LoginResponseStatus::NotAuthorized:     { is_authorized = false; break; }
            case Messages::LoginResponseStatus::AlreadyAuthorized: { is_authorized = true;  break; }
            case Messages::LoginResponseStatus::SessionInUse:      { is_authorized = false; break; }
        }
    }

    /// @brief Handle a logout response.
    ///
    /// @param response the logout response to handle
    ///
    void handle(const Messages::LogoutResponse& response) {
        is_authorized = false;
        // clear the set of active orders
        active_orders = {};
        switch (response.reason) {  // handle the message based on its status
            case Messages::LogoutReason::UserRequested:     { break; }
            case Messages::LogoutReason::EndOfDay:          { break; }
            case Messages::LogoutReason::Administrative:    { break; }
            case Messages::LogoutReason::ProtocolViolation: { break; }
        }
    }

    /// @brief Handle an order response.
    ///
    /// @param response the order response to handle
    ///
    void handle(const Messages::OrderResponse& response) {
        if (response.status == Messages::OrderStatus::Rejected) {  // order rejected
            std::cerr << "Client::handle(OrderResponse) - order rejected" << std::endl;
            // TODO: handle order rejected
            return;
        }
        if (response.order_id == 0)  // market order
            return;
        // add the limit order ID to the active orders set
        active_orders.insert(response.order_id);
    }

    /// @brief Handle a cancel response.
    ///
    /// @param response the cancel response to handle
    ///
    void handle(const Messages::CancelResponse& response) {
        if (response.status == Messages::CancelStatus::Rejected) {  // cancel rejected
            std::cerr << "Client::handle(CancelResponse) - cancel rejected" << std::endl;
            // TODO: handle cancel rejected
            return;
        }
        // remove the limit order ID from the active orders set
        active_orders.erase(response.order_id);
    }

    /// @brief Handle a replace response.
    ///
    /// @param response the replace response to handle
    ///
    void handle(const Messages::ReplaceResponse& response) {
        if (response.status == Messages::ReplaceStatus::Rejected) {  // replace rejected
            std::cerr << "Client::handle(ReplaceResponse) - replace rejected" << std::endl;
            // TODO: handle replace rejected
            return;
        }
        // remove the canceled order from the set of active orders
        if (response.canceled != 0)
            active_orders.erase(response.canceled);
        // add the limit order ID to the active orders set
        if (response.new_order_id != 0)
            active_orders.insert(response.new_order_id);
    }

    /// @brief Handle a purge response.
    ///
    /// @param response the purge response to handle
    ///
    void handle(const Messages::PurgeResponse& response) {
        if (response.status == Messages::PurgeStatus::Rejected) {  // purge rejected
            std::cerr << "Client::handle(PurgeResponse) - purge rejected" << std::endl;
            // TODO: handle purge rejected
            return;
        }
        active_orders.clear();
    }

    /// @brief Handle a trade response.
    ///
    /// @param response the trade response to handle
    ///
    void handle(const Messages::TradeResponse& response) {
        switch (response.side) {  // handle the trade of shares and capital
            case Side::Sell: {
                shares -= response.quantity;
                capital += response.quantity * response.price;
                break;
            }
            case Side::Buy:  {
                shares += response.quantity;
                capital -= response.quantity * response.price;
                break;
            }
        }
        // update the set of active orders
        if (response.order_id != 0) {  // limit order
            if (response.leaves_quantity == 0)  // completely filled
                active_orders.erase(response.order_id);
        }
    }

    /// @brief Handle a response with invalid ID.
    ///
    /// @param header the header of the invalid response
    ///
    void handle(const Messages::Header& header) {
        is_authorized = false;
        throw Exception("Client::handle(Header) - received message with invalid header " + header.to_string());
    }

 public:
    /// @brief Create a new order entry client.
    ///
    /// @param context the IO context to run the client on
    /// @param host the host address of the order entry server
    /// @param port the port number the order entry server runs at
    ///
    Client(asio::io_context& context, std::string host, std::string port) :
        socket(context) {
        // connect to the server
        asio::connect(socket, asio::ip::tcp::resolver(context).resolve(host, port));
        // listen for messages
        read_buffer();
    }

    /// @brief Return true if the client is authorized, false otherwise.
    ///
    /// @returns true if the client is authorized to send messages
    ///
    inline bool is_logged_in() const { return is_authorized; }

    /// @brief Return the set of active orders for this client connection.
    ///
    /// @returns a constant set of order IDs representing active orders
    ///
    inline const OrderIDSet& get_active_orders() const { return active_orders; }

    /// @brief Return true if the client has an active order.
    ///
    /// @returns true if there is an active limit order for the account
    ///
    inline bool has_active_order() const { return not active_orders.empty(); }

    /// @brief Set the client shares to a new value.
    ///
    /// @param shares_ the new amount of shares for the client
    ///
    inline void set_shares(ClientShares shares_) { shares = shares_; }

    /// @brief Return the number of shares the client currently has.
    ///
    /// @returns the number of shares in the client's local account
    ///
    inline ClientShares get_shares() const { return shares; }

    /// @brief Set the client capital to a new value.
    ///
    /// @param capital_ the new amount of capital for the client
    ///
    inline void set_capital(ClientCapital capital_) { capital = capital_; }

    /// @brief Return the amount of capital the client currently has.
    ///
    /// @returns the amount of capital in the client's local account
    ///
    inline ClientCapital get_capital() const { return capital; }

    /// @brief Write a message asynchronously (non-blocking).
    ///
    /// @tparam T the class of the message to send
    /// @param ...args the initialization parameters for the object
    ///
    template <class T, typename ...Args>
    inline void send(Args && ...args) {
        // put an empty packet on the back of the queue
        output_buffer.emplace_back();
        // initialize the message data on the buffer using forwarded arguments
        new (&output_buffer.back()) T{sequence++, std::forward<Args>(args)...};
        // increment the number of messages available now the buffer is ready
        // start the write event loop if this is the first message
        if (++output_buffer_size == 1) write_buffer();
    }

    /// @brief Write a message synchronously (blocking).
    ///
    /// @tparam T the class of the message to send
    /// @param ...args the initialization parameters for the object
    ///
    template <class T, typename ...Args>
    inline void send_sync(Args && ...args) {
        // put an empty packet on the back of the queue
        output_buffer.emplace_back();
        // initialize the message data on the buffer using forwarded arguments
        new (&output_buffer.back()) T{sequence++, std::forward<Args>(args)...};
        // increment the number of messages available now the buffer is ready
        // start the write event loop if this is the first message
        if (++output_buffer_size == 1) write_buffer();
    }
};

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_CLIENT_HPP
