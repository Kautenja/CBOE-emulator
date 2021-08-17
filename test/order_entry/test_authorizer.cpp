// Test cases for the authorizer class.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "order_entry/authorizer.hpp"

struct Foo {
    void trade(
        OrderEntry::OrderID order_id,
        OrderEntry::Quantity size,
        OrderEntry::Quantity leaves_size,
        OrderEntry::Price price,
        OrderEntry::Side side
    ) { }
};

SCENARIO("initialize OrderEntry::Authorizer") {
    WHEN("a OrderEntry::Authorizer is initialized") {
        OrderEntry::Authorizer<Foo> authorizer;
        THEN("the size (number of users) is 0") {
            REQUIRE(0 == authorizer.size());
        }
    }
}

SCENARIO("add a new user") {
    GIVEN("an account authorizer and a new username and password") {
        OrderEntry::Authorizer<Foo> authorizer;
        std::string username = "fooser";
        std::string password = "barword";
        std::string username_invalid = "asdfasdf";
        std::string password_invalid = "poiupoiu";
        WHEN("a new account is created") {
            authorizer.new_user(username, password);
            THEN("the size (number of users) is 1") {
                REQUIRE(1 == authorizer.size());
            }
            THEN("has account is true for the username") {
                REQUIRE(authorizer.has_account(username));
            }
            THEN("has account is false for another username") {
                REQUIRE_FALSE(authorizer.has_account(username_invalid));
            }
            THEN("is valid is true for username and password combination") {
                REQUIRE(authorizer.is_valid(username, password));
            }
            THEN("is valid is false for username and invalid password combination") {
                REQUIRE_FALSE(authorizer.is_valid(username, password_invalid));
            }
            THEN("is valid is false for invalid username and password combination") {
                REQUIRE_FALSE(authorizer.is_valid(username_invalid, password_invalid));
            }
        }
    }
    GIVEN("an account authorizer with an account") {
        OrderEntry::Authorizer<Foo> authorizer;
        std::string username = "fooser";
        std::string password = "barword";
        authorizer.new_user(username, password);
        WHEN("a new account is created twice") {
            THEN("an exception is thrown") {
                REQUIRE_THROWS(authorizer.new_user(username, password));
            }
        }
    }
}
