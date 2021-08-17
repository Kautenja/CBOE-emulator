// A single connection to the order entry server.
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

#ifndef ORDER_ENTRY_CONNECTION_HPP
#define ORDER_ENTRY_CONNECTION_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <asio.hpp>
#include "exceptions.hpp"
#include "limit_order_book/limit_order_book.hpp"
#include "messages.hpp"
#include "authorizer.hpp"
#include "system_account.hpp"

using asio::ip::tcp;

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// A TCP connection to a client device for providing direct market access.
class Connection : public std::enable_shared_from_this<Connection> {
 private:
    /// the socket this connection is running at
    tcp::socket socket;
    /// the account manager for handling credentials
    Authorizer<Connection>& authorizer;
    /// the limit order book for the instrument
    LOB::LimitOrderBook& book;
    /// the account for this connection (nullptr indicates not signed in)
    SystemAccount<Connection>* account = nullptr;
    /// the queue of input messages
    Messages::PacketQueue input_buffer;
    /// the queue of output messages
    Messages::PacketQueue output_buffer;
    /// the size of the output buffer
    uint32_t output_buffer_size = 0;
    /// the sequence number for the client messages
    SequenceNumber sequence = 0;
    /// the timer for waiting
    asio::steady_timer timer;

    /// @brief Initialize a new connection.
    ///
    /// @param context the IO context to create the connection with
    /// @param authorizer the authorizer for credential / account management
    /// @param book_ the limit order book for the instrument
    ///
    Connection(
        asio::io_context& context,
        Authorizer<Connection>& authorizer_,
        LOB::LimitOrderBook& book_
    ) : socket(context), authorizer(authorizer_), book(book_), timer(context) { }

    /// @brief Logout the user from the connection.
    void logout() {
        // user is already logged out
        if (account == nullptr) return;
        // mark the account as disconnected
        account->is_connected = false;
        // remove the handler from the account
        account->handler = nullptr;
        // remove the account from the connection
        account = nullptr;
    }

    /// @brief Begin the asynchronous write loop for data in the buffer.
    inline void write_buffer() {
        asio::async_write(
            socket,
            asio::buffer(output_buffer.front(), sizeof(Messages::Packet)),
            [this](const std::error_code& error, std::size_t) {
                if (error) {
                    std::cout << "OrderEntry::Connection::write_buffer - " << error << std::endl;
                    return;
                }
                // remove the sent message from the queue
                output_buffer.pop_front();
                // send another message if there is more work
                if (--output_buffer_size) write_buffer();

            }
        );
    }

    /// @brief Write a message.
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

    /// @brief Asynchronously receive a message.
    void read_buffer() {
        auto self(shared_from_this());
        input_buffer.emplace_back();
        asio::async_read(
            socket,
            asio::buffer(input_buffer.back(), sizeof(Messages::Packet)),
            [this, self](const std::error_code& error, std::size_t) {
                if (error) {
                    if (error == asio::error::eof) {  // connection closed
                        // logout the account and close the connection
                        logout();
                        return;
                    }
                    // an unexpected error occurred, throw the error
                    throw Exception("Server::read_buffer - " + error.message());
                }
                // copy the message from the front of the queue and pop it off
                Messages::Packet message = input_buffer.front();
                input_buffer.pop_front();
                // read another message from the client
                read_buffer();
                // unwrap and handle the data using the template ID
                switch (reinterpret_cast<Messages::Header&>(message).uid) {
                    case Messages::MessageID::LoginRequest:   { handle(reinterpret_cast<Messages::LoginRequest&>(message));   break; }
                    case Messages::MessageID::LogoutRequest:  { handle(reinterpret_cast<Messages::LogoutRequest&>(message));  break; }
                    case Messages::MessageID::OrderRequest:   { handle(reinterpret_cast<Messages::OrderRequest&>(message));   break; }
                    case Messages::MessageID::CancelRequest:  { handle(reinterpret_cast<Messages::CancelRequest&>(message));  break; }
                    case Messages::MessageID::ReplaceRequest: { handle(reinterpret_cast<Messages::ReplaceRequest&>(message)); break; }
                    case Messages::MessageID::PurgeRequest:   { handle(reinterpret_cast<Messages::PurgeRequest&>(message));   break; }
                    default:                                  { handle(reinterpret_cast<Messages::Header&>(message));         break; }
                }
            }
        );
    }

    /// @brief Handle a login request.
    ///
    /// @param request the login request to handle
    ///
    void handle(const Messages::LoginRequest& request) {
        // convert the user-name / password character arrays to strings
        auto username = request.username_string();
        auto password = request.password_string();
        if (is_logged_in()) {  // the client has already logged into this connection
            // just format a response, don't de-authorize the connection
            send<Messages::LoginResponse>(Messages::LoginResponseStatus::AlreadyAuthorized);
        } else if (not authorizer.is_valid(username, password)) {  // invalid credentials
            // just format a response, no other work needs done
            send<Messages::LoginResponse>(Messages::LoginResponseStatus::NotAuthorized);
        } else if (authorizer.get_account(username)->is_connected) {  // session in use on other connection
            // just format a response, no other work needs done
            send<Messages::LoginResponse>(Messages::LoginResponseStatus::SessionInUse);
        } else {  // valid credentials
            // set the account for this connection based on the user-name
            account = authorizer.get_account(username);
            // set the trade handler for the account to this connection
            // this ensures that `this->trade` is called by the account handler
            // when trades connected to the account occur
            account->handler = this;
            // set the connected flag for account to prevent additional sessions
            // being created on different connections
            account->is_connected = true;
            // format the login response message for the client
            send<Messages::LoginResponse>(Messages::LoginResponseStatus::Accepted);
        }
    }

    /// @brief Handle a logout request.
    ///
    /// @param request the logout request to handle
    ///
    void handle(const Messages::LogoutRequest& request) {
        // the client is not authorized to begin with
        if (not is_logged_in()) {
            send<Messages::LogoutResponse>(Messages::LogoutReason::ProtocolViolation);
            return;
        }
        // logout the user and format a response
        logout();
        send<Messages::LogoutResponse>(Messages::LogoutReason::UserRequested);
    }

    /// @brief Handle an order request.
    ///
    /// @param request the order request to handle
    ///
    void handle(const Messages::OrderRequest& request) {
        if (not is_logged_in()) {  // the client is not authorized to send
            send<Messages::OrderResponse>(Messages::ORDER_ID_REJECTED, Messages::OrderStatus::Rejected);
            return;
        }
        // handle the order
        if (request.price == Messages::ORDER_PRICE_MARKET) {  // market order
            book.market(account, side_to_LOB_side(request.side), request.quantity);
            send<Messages::OrderResponse>(Messages::ORDER_ID_MARKET, Messages::OrderStatus::Accepted);
        } else {  // limit order
            auto uid = book.limit(account, side_to_LOB_side(request.side), request.quantity, request.price);
            send<Messages::OrderResponse>(uid, Messages::OrderStatus::Accepted);
        }
    }

    /// @brief Handle a cancel request.
    ///
    /// @param request the cancel request to handle
    ///
    void handle(const Messages::CancelRequest& request) {
        // the client is not authorized to send
        if (not is_logged_in()) {
            send<Messages::CancelResponse>(request.order_id, Messages::CancelStatus::Rejected);
            return;
        }
        // the order doesn't exist
        if (not book.has(request.order_id)) {
            send<Messages::CancelResponse>(request.order_id, Messages::CancelStatus::Rejected);
            return;
        }
        // order doesn't belong to this account
        if (static_cast<SystemAccount<Connection>*>(book.get(request.order_id).account)->username.compare(account->username) != 0) {
            send<Messages::CancelResponse>(request.order_id, Messages::CancelStatus::Rejected);
            return;
        }
        // cancel the order and send the response
        book.cancel(request.order_id);
        send<Messages::CancelResponse>(request.order_id, Messages::CancelStatus::Accepted);
    }

    /// @brief Handle a replace request.
    ///
    /// @param request the replace request to handle
    ///
    void handle(const Messages::ReplaceRequest& request) {
        // the client is not authorized to send
        if (not is_logged_in()) {
            send<Messages::ReplaceResponse>(request.order_id, Messages::ORDER_ID_REJECTED, Messages::ReplaceStatus::Rejected);
            return;
        }
        // check if the replaced order still exists (it may have filled already)
        OrderID canceled = Messages::ORDER_ID_REJECTED;
        if (book.has(request.order_id)) {
            // order doesn't belong to this account
            if (static_cast<SystemAccount<Connection>*>(book.get(request.order_id).account)->username.compare(account->username) != 0) {
                send<Messages::ReplaceResponse>(request.order_id, Messages::ORDER_ID_REJECTED, Messages::ReplaceStatus::Rejected);
                return;
            }
            // cancel the order and send the response
            book.cancel(request.order_id);
            canceled = request.order_id;
        }
        // place the new order
        OrderID uid = Messages::ORDER_ID_REJECTED;
        switch (request.side) {
            case Side::Sell: {
                uid = book.limit_sell(account, request.quantity, request.price);
                break;
            }
            case Side::Buy: {
                uid = book.limit_buy(account, request.quantity, request.price);
                break;
            }
        }
        // send the acceptance response
        send<Messages::ReplaceResponse>(canceled, uid, Messages::ReplaceStatus::Accepted);
    }

    /// @brief Handle a purge request.
    ///
    /// @param request the purge request to handle
    ///
    void handle(const Messages::PurgeRequest& request) {
        // the client is not authorized to send
        if (not is_logged_in()) {
            send<Messages::PurgeResponse>(Messages::PurgeStatus::Rejected);
            return;
        }
        // cancel all the orders. it's important to use a while loop here
        // instead of a for loop because calls to cancel will change the state
        // of the orders set and cause a segmentation fault if a for loop is
        // used in some cases
        while (account->orders.size()) {
            book.cancel((*account->orders.begin())->uid);
        }
        // send the purge response to the client
        send<Messages::PurgeResponse>(Messages::PurgeStatus::Accepted);
    }

    /// @brief Handle a request with invalid ID.
    ///
    /// @param header the header of the invalid message
    ///
    void handle(const Messages::Header& header) {
        std::cout << "OrderEntry::Connection::handle(Header) - " << header << std::endl;
        // logout the user and send a response
        logout();
        send<Messages::LogoutResponse>(Messages::LogoutReason::ProtocolViolation);
    }

 public:
    /// a shared pointer type for this connection object
    typedef std::shared_ptr<Connection> Pointer;

    /// @brief Create a shared pointer to a new connection.
    ///
    /// @param context the IO context to create the connection with
    /// @param authorizer the authorizer for credential / account management
    /// @param book the limit order book for the instrument
    /// @returns a shared pointer to the new connection
    ///
    static Pointer create(
        asio::io_context& context,
        Authorizer<Connection>& authorizer,
        LOB::LimitOrderBook& book
    ) { return Pointer(new Connection(context, authorizer, book)); }

    /// @brief Return the socket this connection is using.
    ///
    /// @returns the socket this connection is using
    ///
    inline tcp::socket& get_socket() { return socket; }

    /// @brief Return true if the connection is authorized, false otherwise.
    ///
    /// @returns true if the connection has been authorized (logged in)
    ///
    inline bool is_logged_in() const { return account != nullptr; }

    /// @brief Return the size of the output buffer (number of queued messages).
    ///
    /// @returns the number of messages in the outbound queue
    ///
    inline uint32_t get_output_buffer_size() const { return output_buffer_size; }

    /// @brief Return the sequence number at this sender.
    ///
    /// @returns the sequence number of the next message to send
    ///
    inline SequenceNumber get_sequence() const { return sequence; }

    /// @brief Start the connection.
    inline void start() { read_buffer(); }

    /// @brief Send a trade message to the client.
    ///
    /// @param order_id the ID of the trade that occurred
    /// @param price the price the trade occurred at
    /// @param size the size of the trade the occurred
    /// @param leaves_size the size left on the order after the trade
    /// @param side the side of the book the trade is for
    ///
    inline void trade(
        OrderID order_id,
        Price price,
        Quantity size,
        Quantity leaves_size,
        Side side
    ) { send<Messages::TradeResponse>(order_id, price, size, leaves_size, side); }
};

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_CONNECTION_HPP
