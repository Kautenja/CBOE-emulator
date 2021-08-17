// Test cases for the types and order, limit, and account structures.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "order_entry/limit_order_book/structures.hpp"

using namespace OrderEntry::LOB;

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
    REQUIRE(order.account == nullptr);
}

SCENARIO("initialize Order") {
    GIVEN("arbitrary legal parameters") {
        UID uid = 5;
        Side side = Side::Buy;
        Quantity quantity = 100;
        Price price = 5746;
        Account account;
        WHEN("an Order is initialized") {
            Order order{uid, side, quantity, price, nullptr, &account};
            THEN("the order is created with parameters") {
                REQUIRE(order.next == nullptr);
                REQUIRE(order.prev == nullptr);
                REQUIRE(order.uid == uid);
                REQUIRE(order.side == side);
                REQUIRE(order.quantity == quantity);
                REQUIRE(order.price == price);
                REQUIRE(order.limit == nullptr);
                REQUIRE(order.account == &account);
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
        Account account;
        Order order{5, Side::Buy, quantity, price, nullptr, &account};
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

// ---------------------------------------------------------------------------
// MARK: Account
// ---------------------------------------------------------------------------

TEST_CASE("initialize Account") {
    GIVEN("no parameters") {
        WHEN("an Account is initialized") {
            Account account;
            THEN("the account is created with default values") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.empty());
            }
        }
    }
    GIVEN("arbitrary legal parameters") {
        Shares shares = 100;
        Capital capital = 5000;
        WHEN("an Account is initialized") {
            Account account{shares, capital};
            THEN("the account is created with parameter values") {
                REQUIRE(account.shares == shares);
                REQUIRE(account.capital == capital);
                REQUIRE(account.orders.empty());
            }
        }
    }
}

SCENARIO("add a limit order to an account") {
    GIVEN("an account") {
        Account account;
        WHEN("a buy limit order is placed") {
            Quantity quantity = 100;
            Order order{0, Side::Buy, quantity, 5000, nullptr, &account};
            account.limit(&order);
            THEN("the account is updated") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.size() == 1);
                REQUIRE(account.orders.find(&order) != account.orders.end());
            }
        }
        WHEN("a sell limit order is placed") {
            Quantity quantity = 100;
            Order order{0, Side::Sell, quantity, 5000, nullptr, &account};
            account.limit(&order);
            THEN("the account is updated") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.size() == 1);
                REQUIRE(account.orders.find(&order) != account.orders.end());
            }
        }
    }
}

SCENARIO("cancel a limit order on an account") {
    GIVEN("an account with a buy order") {
        Account account;
        Order order{0, Side::Buy, 100, 5000, nullptr, &account};
        account.limit(&order);
        WHEN("the limit order is canceled") {
            account.cancel(&order);
            THEN("the account is updated") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.empty());
            }
        }
    }
    GIVEN("an account with a sell order") {
        Account account;
        Order order{0, Side::Sell, 100, 5000, nullptr, &account};
        account.limit(&order);
        WHEN("the limit order is canceled") {
            account.cancel(&order);
            THEN("the account is updated") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.empty());
            }
        }
    }
}

SCENARIO("a limit order is partially filled on an account") {
    GIVEN("an account with a sell order") {
        Account account;
        Quantity quantity_limit = 100;
        Quantity quantity_market = 40;
        Price price = 5000;
        Order limit{0, Side::Sell, quantity_limit, price, nullptr, &account};
        account.limit(&limit);
        WHEN("a buy market order matches partially") {
            Order market{0, Side::Buy, quantity_market, price, nullptr, &account};
            account.limit_partial(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == -static_cast<Shares>(quantity_market));
                REQUIRE(account.capital == price * quantity_market);
                REQUIRE(account.orders.size() == 1);
                REQUIRE(account.orders.find(&limit) != account.orders.end());
            }
        }
    }
    GIVEN("an account with a buy order") {
        Account account;
        Quantity quantity_limit = 100;
        Quantity quantity_market = 40;
        Price price = 5000;
        Order limit{0, Side::Buy, quantity_limit, price, nullptr, &account};
        account.limit(&limit);
        WHEN("a sell market order matches partially") {
            Order market{0, Side::Sell, quantity_market, price, nullptr, &account};
            account.limit_partial(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == static_cast<Shares>(quantity_market));
                REQUIRE(account.capital == -price * quantity_market);
                REQUIRE(account.orders.size() == 1);
                REQUIRE(account.orders.find(&limit) != account.orders.end());
            }
        }
    }
}

SCENARIO("a limit order is filled an account") {
    GIVEN("an account with a sell order") {
        Account account;
        Quantity quantity = 100;
        Price price = 5000;
        Order limit{0, Side::Sell, quantity, price, nullptr, &account};
        account.limit(&limit);
        WHEN("a buy market order matches entirely") {
            Order market{0, Side::Buy, quantity, price, nullptr, &account};
            account.limit_fill(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == -static_cast<Shares>(quantity));
                REQUIRE(account.capital == price * quantity);
                REQUIRE(account.orders.empty());
            }
        }
    }
    GIVEN("an account with a buy order") {
        Account account;
        Quantity quantity = 100;
        Price price = 5000;
        Order limit{0, Side::Buy, quantity, price, nullptr, &account};
        account.limit(&limit);
        WHEN("a sell market order matches entirely") {
            Order market{0, Side::Sell, quantity, price, nullptr, &account};
            account.limit_fill(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == static_cast<Shares>(quantity));
                REQUIRE(account.capital == -price * quantity);
                REQUIRE(account.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is partially filled on an account") {
    GIVEN("an account") {
        Account account;
        WHEN("a sell market order is partially filled") {
            Quantity quantity_limit = 100;
            Quantity quantity_market = 40;
            Price price = 50;
            Order limit{0, Side::Buy, quantity_limit, price, nullptr, &account};
            Order market{0, Side::Sell, quantity_market, price, nullptr, &account};
            account.market_fill(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == -static_cast<Shares>(quantity_market));
                REQUIRE(account.capital == price * quantity_market);
                REQUIRE(account.orders.empty());
            }
        }
        WHEN("a buy market order is partially filled") {
            Quantity quantity_limit = 100;
            Quantity quantity_market = 40;
            Price price = 50;
            Order limit{0, Side::Sell, quantity_limit, price, nullptr, &account};
            Order market{0, Side::Buy, quantity_market, price, nullptr, &account};
            account.market_fill(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == static_cast<Shares>(quantity_market));
                REQUIRE(account.capital == -price * quantity_market);
                REQUIRE(account.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is filled on an account") {
    GIVEN("an account") {
        Account account;
        WHEN("a sell market order is entirely filled") {
            Quantity quantity = 100;
            Price price = 50;
            Order limit{0, Side::Buy, quantity, price, nullptr, &account};
            Order market{0, Side::Sell, quantity, price, nullptr, &account};
            account.market_fill(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == -static_cast<Shares>(quantity));
                REQUIRE(account.capital == price * quantity);
                REQUIRE(account.orders.empty());
            }
        }
        WHEN("a buy market order is entirely filled") {
            Quantity quantity = 100;
            Price price = 50;
            Order limit{0, Side::Sell, quantity, price, nullptr, &account};
            Order market{0, Side::Buy, quantity, price, nullptr, &account};
            account.market_fill(&limit, &market);
            THEN("the account is updated") {
                REQUIRE(account.shares == static_cast<Shares>(quantity));
                REQUIRE(account.capital == -price * quantity);
                REQUIRE(account.orders.empty());
            }
        }
    }
}
