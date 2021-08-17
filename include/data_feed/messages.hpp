// Structures for limit order book depth messages.
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

#ifndef DATA_FEED_MESSAGES_HPP
#define DATA_FEED_MESSAGES_HPP

#include <array>
#include <chrono>
#include <deque>
#include <iostream>
#include <sstream>
#include "limit_order_book/structures.hpp"
#include "clock.hpp"

/// Logic for sending and receiving messages on a financial data feed.
namespace DataFeed {

/// A forward a declaration for the Clock timestamp
typedef Clock::TimeStamp TimeStamp;

/// A type for sequence numbers.
typedef uint32_t SequenceNumber;

/// A type for order prices.
typedef uint64_t OrderID;

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
    // 'B' is 66 and 'S' is larger. subtracting 66 and performing a boolean
    // condition allows fast conversion from char to double
    return (-2 * static_cast<bool>(static_cast<char>(side) - 'B')) + 1;
}

/// @brief Convert an order side character to a boolean value.
///
/// @param side the order side to convert
/// @returns true if side is Side::Buy, false if side is Side::Sell
///
inline constexpr bool side_to_bool(Side side) {
    // 'B' is 66 and 'S' is larger. subtracting 66 and performing a boolean
    // condition allows fast conversion from char to double
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

/// Packet templates for the DataFeed communication protocol.
namespace Messages {

/// A type for network buffers that is smaller than the size of an Ethernet
/// frame. The largest packet is 37 bytes, aligned to 40 (5 64-bit registers).
typedef std::array<char, 40> Packet;

/// A type for queuing packet buffers.
typedef std::deque<Packet> PacketQueue;

/// Message IDs for messages in the protocol.
enum class MessageID : char {
    Clear =          'c',  // clear the book
    AddOrder =       'a',  // add an order
    DeleteOrder =    'd',  // delete an order
    Trade =          't',  // a trade occurred
    StartOfSession = 's',  // start of trading
    EndOfSession =   'e',  // end of trading
};

/// Write a message ID to a stream.
///
/// @param stream the stream to write the message ID to
/// @param uid the message ID to write to the stream
/// @returns the stream after writing the message ID to it
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
/// \image html data-feed-header/data-feed-header.svg "Message Header"
/// \image latex data-feed-header/data-feed-header.pdf "Message Header"
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
    /// the time for the message
    const TimeStamp time;

    /// @brief Initialize a new session header.
    ///
    /// @param length_ the length of the message for the header in bytes
    /// @param uid_ the template ID of the message for the header
    /// @param sequence_ the sequence number for the message
    /// @param time_ the time for the message
    ///
    Header(
        uint16_t length_,
        MessageID uid_,
        SequenceNumber sequence_ = 0,
        TimeStamp time_ = 0
    ) :
        length(length_),
        uid(uid_),
        sequence(sequence_),
        time(time_) { }

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
            << "length=" << static_cast<int>(header.length) << ","
            << "uid='" << header.uid << "',"
            << "sequence=" << header.sequence  << ","
            << "time=" << header.time << ")";
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
// MARK: Clear
// ---------------------------------------------------------------------------

/// @brief A message that indicates to clear all orders in the order book.
/// @details
/// \image html data-feed-clear/data-feed-clear.svg "Clear Message"
/// \image latex data-feed-clear/data-feed-clear.pdf "Clear Message"
///
struct Clear {
    /// the message header that defines the template ID and length of message
    const Header header;

    /// @brief Initialize a new clear book message.
    ///
    /// @param sequence the sequence number for the message
    /// @param time the time for the message
    ///
    Clear(SequenceNumber sequence = 0, TimeStamp time = 0) :
        header{sizeof(Clear), MessageID::Clear, sequence, time} { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const Clear& message
    ) {
        stream << "Clear(" << message.header << ")";
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
// MARK: AddOrder
// ---------------------------------------------------------------------------

/// @brief A message that indicates a limit order was added to the book.
/// @details
/// \image html data-feed-add-order/data-feed-add-order.svg "Add Order Message"
/// \image latex data-feed-add-order/data-feed-add-order.pdf "Add Order Message"
///
struct AddOrder {
    /// the message header that defines the template ID and length of message
    const Header header;
    /// the day specific ID for the order
    const OrderID uid;
    /// the price of the order
    const Price price;
    /// the quantity of the order
    const Quantity quantity;
    /// the side of the order
    const Side side;

    /// @brief Initialize a new add order message.
    ///
    /// @param uid_ the day specific ID for the order
    /// @param price_ the price of the order
    /// @param quantity_ the quantity of the order
    /// @param side_ the side of the order
    /// @param sequence the sequence number for the message
    /// @param time the time for the message
    ///
    AddOrder(
        OrderID uid_,
        Price price_,
        Quantity quantity_,
        Side side_,
        SequenceNumber sequence = 0,
        TimeStamp time = 0
    ) :
        header{sizeof(AddOrder), MessageID::AddOrder, sequence, time},
        uid(uid_),
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
        const AddOrder& message
    ) {
        stream << "AddOrder(" << message.header << ","
            << "uid=" << message.uid << ","
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
// MARK: DeleteOrder
// ---------------------------------------------------------------------------

/// @brief A message that indicates a limit order was added to the book.
/// @details
/// \image html data-feed-delete-order/data-feed-delete-order.svg "Delete Order Message"
/// \image latex data-feed-delete-order/data-feed-delete-order.pdf "Delete Order Message"
///
struct DeleteOrder {
    /// the message header that defines the template ID and length of message
    const Header header;
    /// the day specific ID for the order to delete
    const OrderID uid;

    /// @brief Initialize a new delete order message.
    ///
    /// @param uid_ the day specific ID for the order to delete
    /// @param sequence the sequence number for the message
    /// @param time the time for the message
    ///
    explicit DeleteOrder(
        OrderID uid_,
        SequenceNumber sequence = 0,
        TimeStamp time = 0
    ) :
        header{sizeof(DeleteOrder), MessageID::DeleteOrder, sequence, time},
        uid(uid_) { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const DeleteOrder& message
    ) {
        stream << "DeleteOrder(" << message.header << ","
            << "uid=" << message.uid << ")";
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
// MARK: Trade
// ---------------------------------------------------------------------------

/// @brief A message that indicates a market order matches with a limit order.
/// @details
/// \image html data-feed-trade/data-feed-trade.svg "Trade Message"
/// \image latex data-feed-trade/data-feed-trade.pdf "Trade Message"
///
struct Trade {
    /// the message header that defines the template ID and length of message
    const Header header;
    /// the MessageID for the limit order that matched
    const OrderID uid;
    /// the execution price for the trade
    const Price price;
    /// the quantity of the trade
    const Quantity quantity;
    /// the side of the market order
    const Side side;

    /// @brief Initialize a new trade message.
    ///
    /// @param uid_ the MessageID for the limit order that matched
    /// @param price_ the execution price for the trade
    /// @param quantity_ the quantity of the trade
    /// @param side_ the side of the market order
    /// @param sequence the sequence number for the message
    /// @param time the time for the message
    ///
    Trade(
        OrderID uid_,
        Price price_,
        Quantity quantity_,
        Side side_,
        SequenceNumber sequence = 0,
        TimeStamp time = 0
    ) :
        header{sizeof(Trade), MessageID::Trade, sequence, time},
        uid(uid_),
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
        const Trade& message
    ) {
        stream << "Trade(" << message.header << ","
            << "uid=" << message.uid << ","
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
// MARK: StartOfSession
// ---------------------------------------------------------------------------

/// @brief A message that indicates the start of a trading session.
/// @details
/// \image html data-feed-start-of-session/data-feed-start-of-session.svg "Start of Session Message"
/// \image latex data-feed-start-of-session/data-feed-start-of-session.pdf "Start of Session Message"
///
struct StartOfSession {
    /// the message header that defines the template ID and length of message
    const Header header;

    /// @brief Initialize a new start of session message.
    ///
    /// @param sequence the sequence number for the message
    /// @param time the time for the message
    ///
    StartOfSession(SequenceNumber sequence = 0, TimeStamp time = 0) :
        header{sizeof(StartOfSession), MessageID::StartOfSession, sequence, time} { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const StartOfSession& message
    ) {
        stream << "StartOfSession(" << message.header << ")";
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
// MARK: EndOfSession
// ---------------------------------------------------------------------------

/// @brief A message that indicates the end of a trading session.
/// @details
/// \image html data-feed-end-of-session/data-feed-end-of-session.svg "End of Session Message"
/// \image latex data-feed-end-of-session/data-feed-end-of-session.pdf "End of Session Message"
///
struct EndOfSession {
    /// the message header that defines the template ID and length of message
    const Header header;

    /// @brief Initialize a new end of session message.
    ///
    /// @param sequence the sequence number for the message
    /// @param time the time for the message
    ///
    EndOfSession(SequenceNumber sequence = 0, TimeStamp time = 0) :
        header{sizeof(EndOfSession), MessageID::EndOfSession, sequence, time} { }

    /// @brief Write the data from the message to a stream.
    ///
    /// @param stream the stream to write the data to
    /// @param message the message to write to the stream
    /// @returns the stream with data from the message written to it
    ///
    inline friend std::ostream& operator<<(
        std::ostream& stream,
        const EndOfSession& message
    ) {
        stream << "EndOfSession(" << message.header << ")";
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

}  // namespace DataFeed

#endif  // DATA_FEED_MESSAGES_HPP
