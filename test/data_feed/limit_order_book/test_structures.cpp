// Test cases for the types and order, limit, and account structures.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "data_feed/limit_order_book/structures.hpp"

using namespace DataFeed::LOB;

TEST_CASE("should invert Side::Sell") {
    REQUIRE(Side::Buy == !Side::Sell);
}

TEST_CASE("should invert Side::Buy") {
    REQUIRE(Side::Sell == !Side::Buy);
}

// ---------------------------------------------------------------------------
// MARK: Order
// ---------------------------------------------------------------------------

TEST_CASE("initialize default Order") {
    Order order;
    REQUIRE(order.next == nullptr);
    REQUIRE(order.prev == nullptr);
    REQUIRE(order.uid == 0);
    REQUIRE(order.side == Side::Sell);
    REQUIRE(order.quantity == 0);
    REQUIRE(order.price == 0);
    REQUIRE(order.limit == nullptr);
}

SCENARIO("initialize Order") {
    GIVEN("arbitrary legal parameters") {
        UID uid = 5;
        auto side = Side::Buy;
        Quantity quantity = 100;
        Price price = 5746;
        WHEN("an Order is initialized") {
            Order order = {uid, side, quantity, price};
            THEN("the order is created with parameters") {
                REQUIRE(order.next == nullptr);
                REQUIRE(order.prev == nullptr);
                REQUIRE(order.uid == uid);
                REQUIRE(order.side == side);
                REQUIRE(order.quantity == quantity);
                REQUIRE(order.price == price);
                REQUIRE(order.limit == nullptr);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: Limit
// ---------------------------------------------------------------------------

TEST_CASE("initialize default Limit") {
    Limit limit;
    REQUIRE(limit.key == 0);
    REQUIRE(limit.parent == nullptr);
    REQUIRE(limit.left == nullptr);
    REQUIRE(limit.right == nullptr);
    REQUIRE(limit.count == 0);
    REQUIRE(limit.volume == 0);
    REQUIRE(limit.order_head == nullptr);
    REQUIRE(limit.order_tail == nullptr);
}

SCENARIO("initialize Limit") {
    GIVEN("arbitrary legal parameters and a new order") {
        Quantity quantity = 100;
        Price price = 5;
        Order order = {5, Side::Buy, quantity, price};
        WHEN("a Limit is initialized") {
            Limit limit{&order};
            THEN("the limit is created with parameters") {
                REQUIRE(limit.key == price);
                REQUIRE(limit.parent == nullptr);
                REQUIRE(limit.left == nullptr);
                REQUIRE(limit.right == nullptr);
                REQUIRE(limit.count == 1);
                REQUIRE(limit.volume == quantity);
                REQUIRE(limit.order_head == &order);
                REQUIRE(limit.order_tail == &order);
            }
        }
    }
}
