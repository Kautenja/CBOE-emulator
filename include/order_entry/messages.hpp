// Structures for order entry messages.
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

#ifndef ORDER_ENTRY_MESSAGES_HPP
#define ORDER_ENTRY_MESSAGES_HPP

#include <array>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include "limit_order_book/structures.hpp"
#include "clock.hpp"

/// Logic for sending/receiving application messages in a financial market.
namespace OrderEntry {

/// A forward a declaration for the Clock timestamp.
typedef Clock::TimeStamp TimeStamp;

/// A type for sequence numbers.
typedef uint32_t SequenceNumber;

/// A type for user names.
typedef std::array<char, 4> Username;

/// @brief Make a username from the given input string.
///
/// @param username the string to convert to a username object
/// @returns a username (char array) with the username data
///
Username make_username(std::string username) {
    if (username.length() != sizeof(Username))
        throw "username must have " + std::to_string(sizeof(Username)) + " characters!";
    Username arr;
    std::copy(&username[0], &username[0] + sizeof(Username), arr.data());
    return arr;
}

/// A type for passwords.
typedef std::array<char, 12> Password;

/// @brief Make a password from the given input string.
///
/// @param password the string to convert to a password object
/// @returns a password (char array) with the password data
///
Password make_password(std::string password) {
    if (password.length() != sizeof(Password))
        throw "password must have " + std::to_string(sizeof(Password)) + " characters!";
    Password arr;
    std::copy(&password[0], &password[0] + sizeof(Password), arr.data());
    return arr;
}

/// The side of an order.
enum class Side : char { Sell = 'S', Buy = 'B' };

/// @brief Write the order side to a stream.
///
/// @param stream the stream to write the order side to
/// @param side the order side to write to the stream
/// @returns the stream after writing the order side to it
///
inline std::ostream& operator<<(std::ostream& stream, const Side& side) {
    stream << static_cast<char>(side);
    return stream;
}

/// @brief Convert an order side character to a double.
///
/// @param side the order side to convert to a double
/// @returns 1 if the side is Side::Buy, -1 if the side is Side::Sell
///
inline constexpr double side_to_double(Side side) {
    // B is ASCII 66
    // S is ASCII 83
    // B comes before S in ASCII, if we remove B and have 0, i.e., false, then
    // the side is buy.
    return (-2 * static_cast<bool>(static_cast<char>(side) - 'B')) + 1;
}

/// @brief Convert an order side character to a boolean value.
///
/// @param side the order side to convert
/// @returns true if side is Side::Buy, false if side is Side::Sell
///
inline constexpr bool side_to_bool(Side side) {
    // B is ASCII 66
    // S is ASCII 83
    // B comes before S in ASCII, if we remove B and have 0, i.e., false, then
    // the side is buy.
    return not (static_cast<char>(side) - 'B');
}

/// @brief Convert a side character to a LOB side value.
///
/// @param side the order side to convert
/// @returns the corresponding LOB side object
///
inline constexpr LOB::Side side_to_LOB_side(Side side) {
    return static_cast<LOB::Side>(side_to_bool(side));
}

/// @brief Convert a boolean to an order side.
///
/// @param side the side to convert to an order side
/// @return Side::Sell if side is false, Side::Buy if side is true
///
inline constexpr Side bool_to_side(bool side) {
    // B is ASCII 66
    // S is ASCII 83
    // start with ASCII S, which is larger than ASCII B, if side is true remove
    // the difference between ASCII S and B to get ASCII B
    return static_cast<Side>('S' - side * ('S' - 'B'));
}

/// A type for order quantities.
typedef uint32_t Quantity;

/// A type for order prices.
typedef uint64_t Price;

/// A type for order IDs.
typedef uint64_t OrderID;

/// Packet templates for the OrderEntry communication protocol.
namespace Messages {

/// A type for network buffers that is the size of an Ethernet frame
/// the largest packet is 33 bytes, aligned to 40 (5 64-bit registers).
typedef std::array<char, 40> Packet;

/// A type for queuing packet buffers.
typedef std::deque<Packet> PacketQueue;

/// Message IDs for messages in the protocol.
enum class MessageID : char {
    LoginRequest =    'L',
    LoginResponse =   'l',
    LogoutRequest =   'O',
    LogoutResponse =  'o',
    OrderRequest =    'N',
    OrderResponse =   'n',
    CancelRequest =   'C',
    CancelResponse =  'c',
    ReplaceRequest =  'R', // cancel and replace, but faster (one message)
    ReplaceResponse = 'r',
    // ModifyRequest =   'M', // modify without cancel (decrement order amount)
    // ModifyResponse =  'm',
    PurgeRequest =    'P', // cancel all orders
    PurgeResponse =   'p',
    TradeResponse  =  't'
};

/// @brief Write a template ID to a stream.
///
/// @param stream the stream to write the template id to
/// @param uid the template id to write to the stream
/// @returns the stream after writing the template id to it
///
inline std::ostream& operator<<(std::ostream& stream, const MessageID& uid) {
    stream << static_cast<char>(uid);
    return stream;
}

// ---------------------------------------------------------------------------
// MARK: Header
// ---------------------------------------------------------------------------

/// @brief A header containing type information and metadata for a message.
/// @details
/// \image html order-entry-header/order-entry-header.svg "Header"
/// \image latex order-entry-header/order-entry-header.pdf "Header"
///
struct Header {
    /// the length of the message in bytes
    const uint16_t length;
    /// the template ID for the message
    const MessageID uid;
    /// arbitrary padding to align the sequence number in memory
    const uint8_t padding = 0;
    /// sequence number of the message (event time)
    const SequenceNumber sequence;

    /// @brief Initialize a new session header.
    ///
    /// @param length_ the length of the message for the header in bytes
    /// @param uid_ the template ID of the message for the header
    /// @param sequence_ the sequence number for the message
    ///
    Header(uint16_t length_, MessageID uid_, SequenceNumber sequence_ = 0) :
        length(length_),
        uid(uid_),
        sequence(sequence_) { }

    /// @brief Write the data from the header to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param header the header to write to the stream
    /// @returns the stream with data from the header written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const Header& header
    ) {
        stream << "Header("
            << "length=" << header.length << ","
            << "uid='" << header.uid << "',"
            << "sequence=" << header.sequence << ")";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: LoginRequest
// ---------------------------------------------------------------------------

/// @brief A request that indicates a client is attempting to create a new session.
/// @details
/// \image html order-entry-login-request/order-entry-login-request.svg "Login Request"
/// \image latex order-entry-login-request/order-entry-login-request.pdf "Login Request"
///
struct LoginRequest {
    /// the header for the message
    const Header header;
    /// the user name of the connecting client
    const Username username;
    /// the password for the connecting client
    const Password password;

    /// @brief Initialize a new login request.
    ///
    /// @param sequence the sequence of the message in event time
    /// @param username_ the username for the connecting client
    /// @param password_ the password for the connecting client
    ///
    LoginRequest(SequenceNumber sequence, Username username_, Password password_) :
        header{sizeof(LoginRequest), MessageID::LoginRequest, sequence},
        username(username_),
        password(password_) { }

    /// @brief Return a string representation of the user-name for the account.
    inline std::string username_string() const {
        return std::string(std::begin(username), std::end(username));
    }

    /// @brief Return a string representation of the password for the account
    inline std::string password_string() const {
        return std::string(std::begin(password), std::end(password));
    }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const LoginRequest& message
    ) {
        stream << "LoginRequest(header=" << message.header << ",username=\"";
        stream.write(message.username.data(), 4);
        stream << "\",password=\"";
        stream.write(message.password.data(), 12);
        stream << "\")";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: LoginResponse
// ---------------------------------------------------------------------------

/// Possible statuses for a login response.
enum class LoginResponseStatus : char {
    Accepted =          'A',  // the login request was accepted
    NotAuthorized =     'N',  // the credentials are incorrect
    AlreadyAuthorized = 'C',  // the user is already authorized
    SessionInUse =      'B',  // the user is logged in already
};

/// @brief Write the login status to a stream.
///
/// @param stream the stream to write the login status to
/// @param status the login status to write to the stream
/// @returns the stream after writing the login status to it
///
inline std::ostream& operator<<(std::ostream& stream, const LoginResponseStatus& status) {
    stream << static_cast<char>(status);
    return stream;
}

/// @brief A response that tells the status of a session creation to the client.
/// @details
/// \image html order-entry-login-response/order-entry-login-response.svg "Login Response"
/// \image latex order-entry-login-response/order-entry-login-response.pdf "Login Response"
///
struct LoginResponse {
    /// the header for the message
    const Header header;
    /// the status of the login request
    const LoginResponseStatus status;

    /// @brief Initialize a new login response.
    ///
    /// @param sequence the sequence of the message in event time
    /// @param status_ the status of the login request
    ///
    LoginResponse(SequenceNumber sequence, LoginResponseStatus status_) :
        header{sizeof(LoginResponse), MessageID::LoginResponse, sequence},
        status(status_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const LoginResponse& message
    ) {
        stream << "LoginResponse("
            << "header=" << message.header << ","
            << "status='" << message.status << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: LogoutRequest
// ---------------------------------------------------------------------------

/// @brief A request that indicates a client is attempting to close an active session.
/// @details
/// \image html order-entry-logout-request/order-entry-logout-request.svg "Logout Request"
/// \image latex order-entry-logout-request/order-entry-logout-request.pdf "Logout Request"
///
struct LogoutRequest {
    /// the header for the message
    const Header header;

    /// @brief Initialize a new logout request message.
    ///
    /// @param sequence the sequence number for the message
    ///
    explicit LogoutRequest(SequenceNumber sequence) :
        header{sizeof(LogoutRequest), MessageID::LogoutRequest, sequence} { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const LogoutRequest& message
    ) {
        stream << "LogoutRequest(header=" << message.header << ")";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: LogoutResponse
// ---------------------------------------------------------------------------

/// Reasons that a user may have been logged out.
enum class LogoutReason : char {
    UserRequested =     'U',  // the user requested to logout
    EndOfDay =          'E',  // the trading day is ending, orders are canceled
    Administrative =    'A',  // administrative reason
    ProtocolViolation = '!'   // the client violated the application protocol
};

/// @brief Write the logout reason to a stream.
///
/// @param stream the stream to write the logout reason to
/// @param reason the logout reason to write to the stream
/// @returns the stream after writing the logout reason to it
///
inline std::ostream& operator<<(std::ostream& stream, const LogoutReason& reason) {
    stream << static_cast<char>(reason);
    return stream;
}

/// @brief A response that tells the status of a session destruction to the client.
/// @details
/// \image html order-entry-logout-response/order-entry-logout-response.svg "Logout Response"
/// \image latex order-entry-logout-response/order-entry-logout-response.pdf "Logout Response"
///
struct LogoutResponse {
    /// the header for the message
    const Header header;
    /// the reason the reason the logout
    const LogoutReason reason;

    /// @brief Initialize a new logout response message.
    ///
    /// @param sequence the sequence number for the message
    /// @param reason_ the reason the logout
    ///
    LogoutResponse(SequenceNumber sequence, LogoutReason reason_) :
        header{sizeof(LogoutResponse), MessageID::LogoutResponse, sequence},
        reason(reason_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const LogoutResponse& message
    ) {
        stream << "LogoutResponse(header=" << message.header << ","
            << "reason='" << message.reason << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: OrderRequest
// ---------------------------------------------------------------------------

/// sentinel values for prices
static constexpr Price ORDER_PRICE_MARKET = 0;

/// @brief A request to place a new limit / market order in the book.
/// @details
/// \image html order-entry-order-request/order-entry-order-request.svg "Order Request"
/// \image latex order-entry-order-request/order-entry-order-request.pdf "Order Request"
///
struct OrderRequest {
    /// the header for the message
    const Header header;
    /// the price for the order
    const Price price;
    /// the quantity of the order
    const Quantity quantity;
    /// the side of the order
    const Side side;

    /// @brief Initialize a new order request message.
    ///
    /// @param sequence the sequence number for the message
    /// @param price_ the price for the order
    /// @param quantity_ the quantity of the order
    /// @param side_ the side for the order
    ///
    OrderRequest(
        SequenceNumber sequence,
        Price price_,
        Quantity quantity_,
        Side side_
    ) :
        header{sizeof(OrderRequest), MessageID::OrderRequest, sequence},
        price(price_),
        quantity(quantity_),
        side(side_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const OrderRequest& message
    ) {
        stream << "OrderRequest(header=" << message.header << ","
            << "price=" << message.price << ","
            << "quantity=" << message.quantity << ","
            << "side='" << message.side << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: OrderResponse
// ---------------------------------------------------------------------------

/// sentinel values for market orders
static constexpr OrderID ORDER_ID_MARKET = 0;
/// sentinel values for rejected orders
static constexpr OrderID ORDER_ID_REJECTED = 0;
/// Possible statuses for an order response.
enum class OrderStatus : char { Accepted = 'A', Rejected = 'R' };

/// @brief Write the order status to a stream.
///
/// @param stream the stream to write the order status to
/// @param status the order status to write to the stream
/// @returns the stream after writing the order status to it
///
inline std::ostream& operator<<(std::ostream& stream, const OrderStatus& status) {
    stream << static_cast<char>(status);
    return stream;
}

/// @brief A response describing the status of an order request.
/// @details
/// \image html order-entry-order-response/order-entry-order-response.svg "Order Response"
/// \image latex order-entry-order-response/order-entry-order-response.pdf "Order Response"
///
struct OrderResponse {
    /// the header for the message
    const Header header;
    /// the ID of the newly created order
    const OrderID order_id;
    /// the status of the newly created order
    const OrderStatus status;

    /// @brief Initialize a new order response message.
    ///
    /// @param sequence the sequence number for the message
    /// @param order_id_ the ID of the newly created order
    /// @param status_ the status of the order request
    ///
    OrderResponse(
        SequenceNumber sequence,
        OrderID order_id_,
        OrderStatus status_
    ) :
        header{sizeof(OrderResponse), MessageID::OrderResponse, sequence},
        order_id(order_id_),
        status(status_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const OrderResponse& message
    ) {
        stream << "OrderResponse(header=" << message.header << ","
            << "order_id=" << message.order_id << ","
            << "status='" << message.status << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: CancelRequest
// ---------------------------------------------------------------------------

/// @brief A request to cancel an active limit order in the book.
/// @details
/// \image html order-entry-cancel-request/order-entry-cancel-request.svg "Cancel Request"
/// \image latex order-entry-cancel-request/order-entry-cancel-request.pdf "Cancel Request"
///
struct CancelRequest {
    /// the header for the message
    const Header header;
    /// the order ID of the order to cancel
    const OrderID order_id;

    /// @brief Initialize a new cancel request message.
    ///
    /// @param sequence the sequence number for the message
    /// @param order_id_ the order ID of the order to cancel
    ///
    CancelRequest(SequenceNumber sequence, OrderID order_id_) :
        header{sizeof(CancelRequest), MessageID::CancelRequest, sequence},
        order_id(order_id_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const CancelRequest& message
    ) {
        stream << "CancelRequest(header=" << message.header << ","
            << "order_id=" << message.order_id << ")";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: CancelResponse
// ---------------------------------------------------------------------------

/// Possible statuses for a cancellation response.
enum class CancelStatus : char { Accepted = 'A', Rejected = 'R' };

/// @brief Write the cancel status to a stream.
///
/// @param stream the stream to write the cancel status to
/// @param status the cancel status to write to the stream
/// @returns the stream after writing the cancel status to it
///
inline std::ostream& operator<<(std::ostream& stream, const CancelStatus& status) {
    stream << static_cast<char>(status);
    return stream;
}

/// @brief A response describing the cancellation of an active order in the book.
/// @details
/// \image html order-entry-cancel-response/order-entry-cancel-response.svg "Cancel Response"
/// \image latex order-entry-cancel-response/order-entry-cancel-response.pdf "Cancel Response"
///
struct CancelResponse {
    /// the header for the message
    const Header header;
    /// the order ID of the order to cancel
    const OrderID order_id;
    /// whether the cancel was accepted
    const CancelStatus status;

    /// @brief Initialize a new cancel response message.
    ///
    /// @param sequence the sequence number for the message
    /// @param order_id_ the order ID of the order to cancel
    /// @param status_ the status of the cancellation response
    ///
    CancelResponse(
        SequenceNumber sequence,
        OrderID order_id_,
        CancelStatus status_
    ) :
        header{sizeof(CancelResponse), MessageID::CancelResponse, sequence},
        order_id(order_id_),
        status(status_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const CancelResponse& message
    ) {
        stream << "CancelResponse(header=" << message.header << ","
            << "order_id=" << message.order_id << ","
            << "status='" << message.status << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: ReplaceRequest
// ---------------------------------------------------------------------------

/// @brief A request to replace an active order in the book with a new order.
/// @details
/// \image html order-entry-replace-request/order-entry-replace-request.svg "Replace Request"
/// \image latex order-entry-replace-request/order-entry-replace-request.pdf "Replace Request"
///
struct ReplaceRequest {
    /// the header for the message
    const Header header;
    /// the order ID of the order to replace
    const OrderID order_id;
    /// the price for the order
    const Price price;
    /// the quantity of the order
    const Quantity quantity;
    /// the side of the order
    const Side side;

    /// @brief Initialize a new order replace message.
    ///
    /// @param sequence the sequence number for the message
    /// @param order_id_ the order ID of the order to replace
    /// @param price_ the price for the order
    /// @param quantity_ the quantity of the order
    /// @param side_ the side for the order
    ///
    ReplaceRequest(
        SequenceNumber sequence,
        OrderID order_id_,
        Price price_,
        Quantity quantity_,
        Side side_
    ) :
        header{sizeof(ReplaceRequest), MessageID::ReplaceRequest, sequence},
        order_id(order_id_),
        price(price_),
        quantity(quantity_),
        side(side_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const ReplaceRequest& message
    ) {
        stream << "ReplaceRequest(header=" << message.header << ","
            << "order_id=" << message.order_id << ","
            << "price=" << message.price << ","
            << "quantity=" << message.quantity << ","
            << "side='" << message.side << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: ReplaceResponse
// ---------------------------------------------------------------------------

/// Possible statuses for a replace response.
enum class ReplaceStatus : char { Accepted = 'A', Rejected = 'R' };

/// @brief Write the order replace status to a stream.
///
/// @param stream the stream to write the order replace status to
/// @param status the order replace status to write to the stream
/// @returns the stream after writing the order replace status to it
///
inline std::ostream& operator<<(std::ostream& stream, const ReplaceStatus& status) {
    stream << static_cast<char>(status);
    return stream;
}

/// @brief A response describing the replacement of an active order with a new order.
/// @details
/// \image html order-entry-replace-response/order-entry-replace-response.svg "Replace Response"
/// \image latex order-entry-replace-response/order-entry-replace-response.pdf "Replace Response"
///
struct ReplaceResponse {
    /// the header for the message
    const Header header;
    /// the ID of the newly created order
    const OrderID canceled;
    /// the ID of the newly created order
    const OrderID new_order_id;
    /// the status of the newly created order
    const ReplaceStatus status;

    /// @brief Initialize a new order response message.
    ///
    /// @param sequence the sequence number for the message
    /// @param canceled_ the ID of the canceled order
    /// @param new_order_id_ the ID of the newly created order
    /// @param status_ the status of the order request
    ///
    ReplaceResponse(
        SequenceNumber sequence,
        OrderID canceled_,
        OrderID new_order_id_,
        ReplaceStatus status_
    ) :
        header{sizeof(ReplaceResponse), MessageID::ReplaceResponse, sequence},
        canceled(canceled_),
        new_order_id(new_order_id_),
        status(status_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const ReplaceResponse& message
    ) {
        stream << "ReplaceResponse(header=" << message.header << ","
            << "canceled=" << message.canceled << ","
            << "new_order_id=" << message.new_order_id << ","
            << "status='" << message.status << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: PurgeRequest
// ---------------------------------------------------------------------------

/// @brief A request to cancel all active orders in the book.
/// @details
/// \image html order-entry-purge-request/order-entry-purge-request.svg "Purge Request"
/// \image latex order-entry-purge-request/order-entry-purge-request.pdf "Purge Request"
///
struct PurgeRequest {
    /// the header for the message
    const Header header;

    /// @brief Initialize a new purge request message.
    ///
    /// @param sequence the sequence number for the message
    ///
    explicit PurgeRequest(SequenceNumber sequence) :
        header{sizeof(PurgeRequest), MessageID::PurgeRequest, sequence} { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const PurgeRequest& message
    ) {
        stream << "PurgeRequest(header=" << message.header << ")";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: PurgeResponse
// ---------------------------------------------------------------------------

/// Possible statuses for a purge response.
enum class PurgeStatus : char { Accepted = 'A', Rejected = 'R' };

/// @brief Write the purge status to a stream.
///
/// @param stream the stream to write the purge status to
/// @param status the purge status to write to the stream
/// @returns the stream after writing the purge status to it
///
inline std::ostream& operator<<(std::ostream& stream, const PurgeStatus& status) {
    stream << static_cast<char>(status);
    return stream;
}

/// @brief A response describing the status of canceling all active orders in the book.
/// @details
/// \image html order-entry-purge-response/order-entry-purge-response.svg "Purge Response"
/// \image latex order-entry-purge-response/order-entry-purge-response.pdf "Purge Response"
///
struct PurgeResponse {
    /// the header for the message
    const Header header;
    /// whether the purge was accepted
    const PurgeStatus status;

    /// @brief Initialize a new purge response message.
    ///
    /// @param sequence the sequence number for the message
    /// @param status_ the status of the cancellation response
    ///
    PurgeResponse(SequenceNumber sequence, PurgeStatus status_) :
        header{sizeof(PurgeResponse), MessageID::PurgeResponse, sequence},
        status(status_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const PurgeResponse& message
    ) {
        stream << "PurgeResponse(header=" << message.header << ","
            << "status='" << message.status << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

// ---------------------------------------------------------------------------
// MARK: TradeResponse
// ---------------------------------------------------------------------------

/// @brief A response describing a trade that occurred for a limit / market order.
/// @details
/// \image html order-entry-trade-response/order-entry-trade-response.svg "Trade Response"
/// \image latex order-entry-trade-response/order-entry-trade-response.pdf "Trade Response"
///
struct TradeResponse {
    /// the header for the message
    const Header header;
    /// the ID for the order connected to the trade
    const OrderID order_id;
    /// the price the trade executed at
    const Price price;
    /// the number of shares that were exchanges
    const Quantity quantity;
    /// the number of shares that remain in the book
    const Quantity leaves_quantity;
    /// the original side of the order
    const Side side;

    /// @brief Initialize a new trade response message.
    ///
    /// @param sequence the sequence number for the message
    /// @param order_id_ the ID for the order connected to the trade
    /// @param price_ the price the trade executed at
    /// @param quantity_ the number of shares that were exchanges
    /// @param leaves_quantity_ the number of shares that remain in the book
    /// @param side_ the original side of the order
    ///
    TradeResponse(
        SequenceNumber sequence,
        OrderID order_id_,
        Price price_,
        Quantity quantity_,
        Quantity leaves_quantity_,
        Side side_
    ) :
        header{sizeof(TradeResponse), MessageID::TradeResponse, sequence},
        order_id(order_id_),
        price(price_),
        quantity(quantity_),
        leaves_quantity(leaves_quantity_),
        side(side_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const TradeResponse& message
    ) {
        stream << "TradeResponse(header=" << message.header << ","
            << "order_id=" << message.order_id << ","
            << "price=" << message.price << ","
            << "quantity=" << message.quantity << ","
            << "leaves_quantity=" << message.leaves_quantity << ","
            << "side='" << message.side << "')";
        return stream;
    }

    /// @brief Convert the object to an STL string.
    ///
    /// @returns a string representation of the object
    ///
    inline std::string to_string() const {
        std::ostringstream stream;
        stream << this;
        return stream.str();
    }
} __attribute__ ((packed));

}  // namespace Messages

}  // namespace OrderEntry

#endif  // ORDER_ENTRY_MESSAGES_HPP
