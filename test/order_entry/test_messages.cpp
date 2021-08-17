// Test cases for the order entry protocol messages.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "order_entry/messages.hpp"
#include <sstream>

/// Sizes for message structures measured in bytes.
enum class MessageSize {
    Header =                   8,
    LoginRequest =    Header + 16,
    LoginResponse =   Header + 1,
    LogoutRequest =   Header + 0,
    LogoutResponse =  Header + 1,
    OrderRequest =    Header + 13,
    OrderResponse =   Header + 9,
    CancelRequest =   Header + 8,
    CancelResponse =  Header + 9,
    ReplaceRequest =  Header + 21,
    ReplaceResponse = Header + 17,
    PurgeRequest =    Header,
    PurgeResponse =   Header + 1,
    TradeResponse =   Header + 25
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

SCENARIO("convert strings to user names and passwords") {
    // username
    GIVEN("a short username") {
        auto username = "asd";
        WHEN("the username string is converted to a username object") {
            THEN("an error is raised") {
                REQUIRE_THROWS(OrderEntry::make_username(username));
            }
        }
    }
    GIVEN("a valid username") {
        auto username = "asdf";
        WHEN("the username string is converted to a username object") {
            THEN("a valid username is returned") {
                REQUIRE_NOTHROW(OrderEntry::make_username(username));
                auto made_username = OrderEntry::make_username(username);
                std::array<char, 4> valid = {'a', 's', 'd', 'f'};
                REQUIRE(std::equal(valid.begin(), valid.end(), made_username.begin()));
            }
        }
    }
    GIVEN("a long username") {
        auto username = "asdfg";
        WHEN("the username string is converted to a username object") {
            THEN("an error is raised") {
                REQUIRE_THROWS(OrderEntry::make_username(username));
            }
        }
    }
    // password
    GIVEN("a short password") {
        auto password = "qwertyuiopa";
        WHEN("the password string is converted to a password object") {
            THEN("an error is raised") {
                REQUIRE_THROWS(OrderEntry::make_password(password));
            }
        }
    }
    GIVEN("a valid password") {
        auto password = "qwertyuiopas";
        WHEN("the password string is converted to a password object") {
            THEN("a valid password is returned") {
                REQUIRE_NOTHROW(OrderEntry::make_password(password));
                auto made_password = OrderEntry::make_password(password);
                std::array<char, 12> valid = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's'};
                REQUIRE(std::equal(valid.begin(), valid.end(), made_password.begin()));
            }
        }
    }
    GIVEN("a long password") {
        auto password = "qwertyuiopasd";
        WHEN("the password string is converted to a password object") {
            THEN("an error is raised") {
                REQUIRE_THROWS(OrderEntry::make_password(password));
            }
        }
    }
}

TEST_CASE("side_to_double converts sell to -1") {
    REQUIRE(OrderEntry::side_to_double(OrderEntry::Side::Sell) == -1);
}

TEST_CASE("side_to_double converts buy to 1") {
    REQUIRE(OrderEntry::side_to_double(OrderEntry::Side::Buy) == 1);
}

TEST_CASE("side_to_bool converts sell to false") {
    REQUIRE(OrderEntry::side_to_bool(OrderEntry::Side::Sell) == false);
}

TEST_CASE("side_to_bool converts buy to true") {
    REQUIRE(OrderEntry::side_to_bool(OrderEntry::Side::Buy) == true);
}

TEST_CASE("side_to_LOB_side converts sell to LOB::Side::Sell") {
    REQUIRE(OrderEntry::side_to_LOB_side(OrderEntry::Side::Sell) == OrderEntry::LOB::Side::Sell);
}

TEST_CASE("side_to_LOB_side converts buy to LOB::Side::Buy") {
    REQUIRE(OrderEntry::side_to_LOB_side(OrderEntry::Side::Buy) == OrderEntry::LOB::Side::Buy);
}

TEST_CASE("to_side convert False to sell") {
    REQUIRE(OrderEntry::bool_to_side(false) == OrderEntry::Side::Sell);
}

TEST_CASE("to_side converts True to buy") {
    REQUIRE(OrderEntry::bool_to_side(true) == OrderEntry::Side::Buy);
}

// ---------------------------------------------------------------------------
// MARK: OrderEntry::Messages::Header
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::Header is appropriate size") {
    REQUIRE(MessageSize::Header == sizeof(OrderEntry::Messages::Header));
}

SCENARIO("initialize OrderEntry::Messages::Header") {
    GIVEN("arbitrary legal parameters") {
        uint16_t length = 1499;
        auto uid = OrderEntry::Messages::MessageID::LoginRequest;
        OrderEntry::SequenceNumber sequence = 12300032;
        WHEN("a OrderEntry::Messages::Header is initialized (designated)") {
            auto header = OrderEntry::Messages::Header(length, uid, sequence);
            THEN("the header is initialized with parameters") {
                REQUIRE(header.length == length);
                REQUIRE(header.uid == uid);
                REQUIRE(header.sequence == sequence);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << header;
                std::string expected = "Header(length=1499,uid='L',sequence=12300032)";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: OrderEntry::Messages::LoginRequest
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::LoginRequest is appropriate size") {
    REQUIRE(MessageSize::LoginRequest == sizeof(OrderEntry::Messages::LoginRequest));
}

SCENARIO("initialize OrderEntry::Messages::LoginRequest") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        std::array<char, 4> username = {'a', 's', 'd', 'f'};
        std::array<char, 12> password = {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'A', 'S'};
        WHEN("a OrderEntry::Messages::LoginRequest is initialized (designated)") {
            auto message = OrderEntry::Messages::LoginRequest(sequence, username, password);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::LoginRequest);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.username == username);
                REQUIRE_THAT("asdf", Catch::Equals(message.username_string()));
                REQUIRE(message.password == password);
                REQUIRE_THAT("QWERTYUIOPAS", Catch::Equals(message.password_string()));

            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "LoginRequest(header=Header(length=24,uid='L',sequence=12300032),username=\"asdf\",password=\"QWERTYUIOPAS\")";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: OrderEntry::Messages::LoginResponse
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::LoginResponse is appropriate size") {
    REQUIRE(MessageSize::LoginResponse == sizeof(OrderEntry::Messages::LoginResponse));
}

SCENARIO("initialize OrderEntry::Messages::LoginResponse") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        auto status = OrderEntry::Messages::LoginResponseStatus::NotAuthorized;
        WHEN("a OrderEntry::Messages::LoginResponse is initialized (designated)") {
            auto message = OrderEntry::Messages::LoginResponse(sequence, status);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::LoginResponse);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.status == status);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "LoginResponse(header=Header(length=9,uid='l',sequence=12300032),status='N')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: OrderEntry::Messages::LogoutRequest
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::LogoutRequest is appropriate size") {
    REQUIRE(MessageSize::LogoutRequest == sizeof(OrderEntry::Messages::LogoutRequest));
}

SCENARIO("initialize OrderEntry::Messages::LogoutRequest") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        WHEN("a OrderEntry::Messages::LogoutRequest is initialized (designated)") {
            auto message = OrderEntry::Messages::LogoutRequest(sequence);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::LogoutRequest);
                REQUIRE(message.header.sequence == sequence);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "LogoutRequest(header=Header(length=8,uid='O',sequence=12300032))";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: LogoutResponse
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::LogoutResponse is appropriate size") {
    REQUIRE(MessageSize::LogoutResponse == sizeof(OrderEntry::Messages::LogoutResponse));
}

SCENARIO("initialize OrderEntry::Messages::LogoutResponse") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        auto reason = OrderEntry::Messages::LogoutReason::UserRequested;
        WHEN("a OrderEntry::Messages::LogoutResponse is initialized (designated)") {
            auto message = OrderEntry::Messages::LogoutResponse(sequence, reason);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::LogoutResponse);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.reason == reason);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "LogoutResponse(header=Header(length=9,uid='o',sequence=12300032),reason='U')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: OrderRequest
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::OrderRequest is appropriate size") {
    REQUIRE(MessageSize::OrderRequest == sizeof(OrderEntry::Messages::OrderRequest));
}

SCENARIO("initialize OrderEntry::Messages::OrderRequest") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        OrderEntry::Price price = 55;
        OrderEntry::Quantity quantity = 20;
        auto side = OrderEntry::Side::Buy;
        WHEN("a OrderEntry::Messages::OrderRequest is initialized (designated)") {
            auto message = OrderEntry::Messages::OrderRequest(sequence, price, quantity, side);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::OrderRequest);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.price == price);
                REQUIRE(message.quantity == quantity);
                REQUIRE(message.side == side);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "OrderRequest(header=Header(length=21,uid='N',sequence=12300032),price=55,quantity=20,side='B')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: OrderResponse
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::OrderResponse is appropriate size") {
    REQUIRE(MessageSize::OrderResponse == sizeof(OrderEntry::Messages::OrderResponse));
}

SCENARIO("initialize OrderEntry::Messages::OrderResponse") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        OrderEntry::OrderID order_id = 54;
        auto status = OrderEntry::Messages::OrderStatus::Rejected;
        WHEN("a OrderEntry::Messages::OrderResponse is initialized (designated)") {
            auto message = OrderEntry::Messages::OrderResponse(sequence, order_id, status);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::OrderResponse);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.order_id == order_id);
                REQUIRE(message.status == status);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "OrderResponse(header=Header(length=17,uid='n',sequence=12300032),order_id=54,status='R')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: CancelRequest
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::CancelRequest is appropriate size") {
    REQUIRE(MessageSize::CancelRequest == sizeof(OrderEntry::Messages::CancelRequest));
}

SCENARIO("initialize OrderEntry::Messages::CancelRequest") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        OrderEntry::OrderID order_id = 54;
        WHEN("a OrderEntry::Messages::CancelRequest is initialized (designated)") {
            auto message = OrderEntry::Messages::CancelRequest(sequence, order_id);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::CancelRequest);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.order_id == order_id);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "CancelRequest(header=Header(length=16,uid='C',sequence=12300032),order_id=54)";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: CancelResponse
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::CancelResponse is appropriate size") {
    REQUIRE(MessageSize::CancelResponse == sizeof(OrderEntry::Messages::CancelResponse));
}

SCENARIO("initialize OrderEntry::Messages::CancelResponse") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        OrderEntry::OrderID order_id = 54;
        auto status = OrderEntry::Messages::CancelStatus::Rejected;
        WHEN("a OrderEntry::Messages::CancelResponse is initialized (designated)") {
            auto message = OrderEntry::Messages::CancelResponse(sequence, order_id, status);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::CancelResponse);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.order_id == order_id);
                REQUIRE(message.status == status);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "CancelResponse(header=Header(length=17,uid='c',sequence=12300032),order_id=54,status='R')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: ReplaceRequest
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::ReplaceRequest is appropriate size") {
    REQUIRE(MessageSize::ReplaceRequest == sizeof(OrderEntry::Messages::ReplaceRequest));
}

SCENARIO("initialize OrderEntry::Messages::ReplaceRequest") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        OrderEntry::OrderID order_id = 5;
        OrderEntry::Price price = 55;
        OrderEntry::Quantity quantity = 20;
        auto side = OrderEntry::Side::Buy;
        WHEN("a OrderEntry::Messages::ReplaceRequest is initialized (designated)") {
            auto message = OrderEntry::Messages::ReplaceRequest(sequence, order_id, price, quantity, side);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::ReplaceRequest);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.order_id == order_id);
                REQUIRE(message.price == price);
                REQUIRE(message.quantity == quantity);
                REQUIRE(message.side == side);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "ReplaceRequest(header=Header(length=29,uid='R',sequence=12300032),order_id=5,price=55,quantity=20,side='B')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: ReplaceResponse
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::ReplaceResponse is appropriate size") {
    REQUIRE(MessageSize::ReplaceResponse == sizeof(OrderEntry::Messages::ReplaceResponse));
}

SCENARIO("initialize OrderEntry::Messages::ReplaceResponse") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        OrderEntry::OrderID canceled = 54;
        OrderEntry::OrderID new_order_id = 61;
        auto status = OrderEntry::Messages::ReplaceStatus::Rejected;
        WHEN("a OrderEntry::Messages::ReplaceResponse is initialized (designated)") {
            auto message = OrderEntry::Messages::ReplaceResponse(sequence, canceled, new_order_id, status);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::ReplaceResponse);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.canceled == canceled);
                REQUIRE(message.new_order_id == new_order_id);
                REQUIRE(message.status == status);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "ReplaceResponse(header=Header(length=25,uid='r',sequence=12300032),canceled=54,new_order_id=61,status='R')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: PurgeRequest
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::PurgeRequest is appropriate size") {
    REQUIRE(MessageSize::PurgeRequest == sizeof(OrderEntry::Messages::PurgeRequest));
}

SCENARIO("initialize OrderEntry::Messages::PurgeRequest") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        WHEN("a OrderEntry::Messages::PurgeRequest is initialized (designated)") {
            auto message = OrderEntry::Messages::PurgeRequest(sequence);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::PurgeRequest);
                REQUIRE(message.header.sequence == sequence);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "PurgeRequest(header=Header(length=8,uid='P',sequence=12300032))";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: PurgeResponse
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::PurgeResponse is appropriate size") {
    REQUIRE(MessageSize::PurgeResponse == sizeof(OrderEntry::Messages::PurgeResponse));
}

SCENARIO("initialize OrderEntry::Messages::PurgeResponse") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        auto status = OrderEntry::Messages::PurgeStatus::Rejected;
        WHEN("a OrderEntry::Messages::PurgeResponse is initialized (designated)") {
            auto message = OrderEntry::Messages::PurgeResponse(sequence, status);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::PurgeResponse);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.status == status);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "PurgeResponse(header=Header(length=9,uid='p',sequence=12300032),status='R')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: TradeResponse
// ---------------------------------------------------------------------------

TEST_CASE("OrderEntry::Messages::TradeResponse is appropriate size") {
    REQUIRE(MessageSize::TradeResponse == sizeof(OrderEntry::Messages::TradeResponse));
}

SCENARIO("initialize OrderEntry::Messages::TradeResponse") {
    GIVEN("arbitrary legal parameters") {
        OrderEntry::SequenceNumber sequence = 12300032;
        OrderEntry::OrderID order_id = 54;
        OrderEntry::Price price = 1234;
        OrderEntry::Quantity quantity = 100;
        OrderEntry::Quantity leaves_quantity = 40;
        auto side = OrderEntry::Side::Buy;
        WHEN("a OrderEntry::Messages::TradeResponse is initialized (designated)") {
            auto message = OrderEntry::Messages::TradeResponse(sequence, order_id, price, quantity, leaves_quantity, side);
            THEN("the message is initialized with parameters") {
                REQUIRE(message.header.length == sizeof(message));
                REQUIRE(message.header.uid == OrderEntry::Messages::MessageID::TradeResponse);
                REQUIRE(message.header.sequence == sequence);
                REQUIRE(message.order_id == order_id);
                REQUIRE(message.price == price);
                REQUIRE(message.quantity == quantity);
                REQUIRE(message.leaves_quantity == leaves_quantity);
                REQUIRE(message.side == side);
            }
            THEN("the operator<< returns a string representations") {
                std::ostringstream stream;
                stream << message;
                std::string expected = "TradeResponse(header=Header(length=33,uid='t',sequence=12300032),order_id=54,price=1234,quantity=100,leaves_quantity=40,side='B')";
                REQUIRE_THAT(expected, Catch::Equals(stream.str()));
            }
        }
    }
}
