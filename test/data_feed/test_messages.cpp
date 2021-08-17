// Test cases for the data feed protocol messages.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "data_feed/messages.hpp"
#include <sstream>

/// Sizes for message structures measured in bytes.
enum class MessageSize {
    Header =         16,
    Clear =          16,
    AddOrder =       37,
    DeleteOrder =    24,
    Trade =          37,
    StartOfSession = 16,
    EndOfSession =   16
};

/// Compare the message size with the given value to check for equality.
///
/// @param a the left operand (the message size)
/// @param b the right operand (the other typed value)
/// @returns true if the values are equal after casting 'a' to the type of 'b'
///
template<typename T>
inline bool operator==(const MessageSize& a, const T& b) {
    return static_cast<T>(a) == b;
}

// ---------------------------------------------------------------------------
// MARK: types
// ---------------------------------------------------------------------------

TEST_CASE("side_to_double converts sell to -1") {
    REQUIRE(DataFeed::side_to_double(DataFeed::Side::Sell) == -1);
}

TEST_CASE("side_to_double converts buy to 1") {
    REQUIRE(DataFeed::side_to_double(DataFeed::Side::Buy) == 1);
}

TEST_CASE("side_to_bool converts sell to false") {
    REQUIRE(DataFeed::side_to_bool(DataFeed::Side::Sell) == false);
}

TEST_CASE("side_to_bool converts buy to true") {
    REQUIRE(DataFeed::side_to_bool(DataFeed::Side::Buy) == true);
}

TEST_CASE("side_to_LOB_side converts sell to LOB::Side::Sell") {
    REQUIRE(DataFeed::side_to_LOB_side(DataFeed::Side::Sell) == DataFeed::LOB::Side::Sell);
}

TEST_CASE("side_to_LOB_side converts buy to LOB::Side::Buy") {
    REQUIRE(DataFeed::side_to_LOB_side(DataFeed::Side::Buy) == DataFeed::LOB::Side::Buy);
}

TEST_CASE("to_side convert False to sell") {
    REQUIRE(DataFeed::bool_to_side(false) == DataFeed::Side::Sell);
}

TEST_CASE("to_side converts True to buy") {
    REQUIRE(DataFeed::bool_to_side(true) == DataFeed::Side::Buy);
}

// ---------------------------------------------------------------------------
// MARK: DataFeed::Messages::Header
// ---------------------------------------------------------------------------

TEST_CASE("DataFeed::Messages::Header is appropriate size") {
    REQUIRE(MessageSize::Header == sizeof(DataFeed::Messages::Header));
}

SCENARIO("initialize DataFeed::Messages::Header") {
    GIVEN("arbitrary legal parameters") {
        uint8_t length = 144;
        auto uid = DataFeed::Messages::MessageID::AddOrder;
        WHEN("a DataFeed::Messages::Header is initialized (designated)") {
            auto header = DataFeed::Messages::Header(length, uid);
            THEN("the header is initialized with parameters") {
                REQUIRE(header.length == length);
                REQUIRE(header.uid == uid);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << header;
                std::string expected = "Header(length=144,uid='a',sequence=0,time=0)";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: DataFeed::Messages::Clear
// ---------------------------------------------------------------------------

TEST_CASE("DataFeed::Messages::Clear is appropriate size") {
    REQUIRE(MessageSize::Clear == sizeof(DataFeed::Messages::Clear));
}

SCENARIO("initialize DataFeed::Messages::Clear") {
    GIVEN("arbitrary legal parameters") {
        WHEN("a DataFeed::Messages::Clear is initialized (designated)") {
            auto message = DataFeed::Messages::Clear();
            THEN("the message header is correct") {
                REQUIRE(message.header.length == sizeof(DataFeed::Messages::Clear));
                REQUIRE(message.header.uid == DataFeed::Messages::MessageID::Clear);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "Clear(Header(length=16,uid='c',sequence=0,time=0))";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
        WHEN("the message is initialized with optional sequence and time") {
            DataFeed::SequenceNumber sequence = 2;
            DataFeed::TimeStamp time = 3;
            auto message = DataFeed::Messages::Clear(sequence, time);
            THEN("the message header is correct") {
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.header.time == time);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: DataFeed::Messages::AddOrder
// ---------------------------------------------------------------------------

TEST_CASE("DataFeed::Messages::AddOrder is appropriate size") {
    REQUIRE(MessageSize::AddOrder == sizeof(DataFeed::Messages::AddOrder));
}

SCENARIO("initialize DataFeed::Messages::AddOrder") {
    GIVEN("arbitrary legal parameters") {\
        DataFeed::OrderID uid = 125;
        DataFeed::Price price = 23;
        DataFeed::Quantity quantity = 5;
        auto side = DataFeed::Side::Buy;
        WHEN("a DataFeed::Messages::AddOrderLong is initialized (designated)") {
            auto message = DataFeed::Messages::AddOrder(uid, price, quantity, side);
            THEN("the message header is correct") {
                REQUIRE(message.header.length == sizeof(DataFeed::Messages::AddOrder));
                REQUIRE(message.header.uid == DataFeed::Messages::MessageID::AddOrder);
            }
            THEN("the message is initialized with parameters") {
                REQUIRE(message.uid == uid);
                REQUIRE(message.price == price);
                REQUIRE(message.quantity == quantity);
                REQUIRE(message.side == side);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "AddOrder(Header(length=37,uid='a',sequence=0,time=0),uid=125,price=23,quantity=5,side='B')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
        WHEN("the message is initialized with optional sequence and time") {
            DataFeed::SequenceNumber sequence = 2;
            DataFeed::TimeStamp time = 3;
            auto message = DataFeed::Messages::AddOrder(uid, price, quantity, side, sequence, time);
            THEN("the message header is correct") {
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.header.time == time);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: DataFeed::Messages::DeleteOrder
// ---------------------------------------------------------------------------

TEST_CASE("DataFeed::Messages::DeleteOrder is appropriate size") {
    REQUIRE(MessageSize::DeleteOrder == sizeof(DataFeed::Messages::DeleteOrder));
}

SCENARIO("initialize DataFeed::Messages::DeleteOrder") {
    GIVEN("arbitrary legal parameters") {\
        DataFeed::OrderID uid = 125;
        WHEN("a DataFeed::Messages::DeleteOrder is initialized (designated)") {
            auto message = DataFeed::Messages::DeleteOrder(uid);
            THEN("the message header is correct") {
                REQUIRE(message.header.length == sizeof(DataFeed::Messages::DeleteOrder));
                REQUIRE(message.header.uid == DataFeed::Messages::MessageID::DeleteOrder);
            }
            THEN("the message is initialized with parameters") {
                REQUIRE(message.uid == uid);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "DeleteOrder(Header(length=24,uid='d',sequence=0,time=0),uid=125)";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
        WHEN("the message is initialized with optional sequence and time") {
            DataFeed::SequenceNumber sequence = 2;
            DataFeed::TimeStamp time = 3;
            auto message = DataFeed::Messages::DeleteOrder(uid, sequence, time);
            THEN("the message header is correct") {
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.header.time == time);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: DataFeed::Messages::Trade
// ---------------------------------------------------------------------------

TEST_CASE("DataFeed::Messages::Trade is appropriate size") {
    REQUIRE(MessageSize::Trade == sizeof(DataFeed::Messages::Trade));
}

SCENARIO("initialize DataFeed::Messages::Trade") {
    GIVEN("arbitrary legal parameters") {\
        DataFeed::OrderID uid = 125;
        auto side = DataFeed::Side::Buy;
        DataFeed::Quantity quantity = 5;
        DataFeed::Price price = 23;
        WHEN("a DataFeed::Messages::Trade is initialized (designated)") {
            auto message = DataFeed::Messages::Trade(uid, price, quantity, side);
            THEN("the message header is correct") {
                REQUIRE(message.header.length == sizeof(DataFeed::Messages::Trade));
                REQUIRE(message.header.uid == DataFeed::Messages::MessageID::Trade);
            }
            THEN("the message is initialized with parameters") {
                REQUIRE(message.uid == uid);
                REQUIRE(message.price == price);
                REQUIRE(message.quantity == quantity);
                REQUIRE(message.side == side);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "Trade(Header(length=37,uid='t',sequence=0,time=0),uid=125,price=23,quantity=5,side='B')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
        WHEN("the message is initialized with optional sequence and time") {
            DataFeed::SequenceNumber sequence = 2;
            DataFeed::TimeStamp time = 3;
            auto message = DataFeed::Messages::Trade(uid, price, quantity, side, sequence, time);
            THEN("the message header is correct") {
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.header.time == time);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: DataFeed::Messages::StartOfSession
// ---------------------------------------------------------------------------

TEST_CASE("DataFeed::Messages::StartOfSession is appropriate size") {
    REQUIRE(MessageSize::StartOfSession == sizeof(DataFeed::Messages::StartOfSession));
}

SCENARIO("initialize DataFeed::Messages::StartOfSession") {
    GIVEN("arbitrary legal parameters") {
        WHEN("a DataFeed::Messages::StartOfSession is initialized (designated)") {
            auto message = DataFeed::Messages::StartOfSession();
            THEN("the message header is correct") {
                REQUIRE(message.header.length == sizeof(DataFeed::Messages::StartOfSession));
                REQUIRE(message.header.uid == DataFeed::Messages::MessageID::StartOfSession);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "StartOfSession(Header(length=16,uid='s',sequence=0,time=0))";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
        WHEN("the message is initialized with optional sequence and time") {
            DataFeed::SequenceNumber sequence = 2;
            DataFeed::TimeStamp time = 3;
            auto message = DataFeed::Messages::StartOfSession(sequence, time);
            THEN("the message header is correct") {
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.header.time == time);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: DataFeed::Messages::EndOfSession
// ---------------------------------------------------------------------------

TEST_CASE("DataFeed::Messages::EndOfSession is appropriate size") {
    REQUIRE(MessageSize::EndOfSession == sizeof(DataFeed::Messages::EndOfSession));
}

SCENARIO("initialize DataFeed::Messages::EndOfSession") {
    GIVEN("arbitrary legal parameters") {
        WHEN("a DataFeed::Messages::EndOfSession is initialized (designated)") {
            auto message = DataFeed::Messages::EndOfSession();
            THEN("the message header is correct") {
                REQUIRE(message.header.length == sizeof(DataFeed::Messages::EndOfSession));
                REQUIRE(message.header.uid == DataFeed::Messages::MessageID::EndOfSession);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "EndOfSession(Header(length=16,uid='e',sequence=0,time=0))";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
        WHEN("the message is initialized with optional sequence and time") {
            DataFeed::SequenceNumber sequence = 2;
            DataFeed::TimeStamp time = 3;
            auto message = DataFeed::Messages::EndOfSession(sequence, time);
            THEN("the message header is correct") {
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.header.time == time);
            }
        }
    }
}
