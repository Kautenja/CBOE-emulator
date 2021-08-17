// A side of a limit order tree.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "order_entry/limit_order_book/limit_tree.hpp"

using namespace OrderEntry::LOB;

// ---------------------------------------------------------------------------
// MARK: constructor
// ---------------------------------------------------------------------------

SCENARIO("should initialize LimitTree") {
    WHEN("a buy limit tree is constructed") {
        THEN("the initial parameters are correct") {
            LimitTree<Side::Buy> tree;
            REQUIRE(tree.root == nullptr);
            REQUIRE(tree.limits.size() == 0);
            REQUIRE(tree.best == nullptr);
        }
    }
    WHEN("a sell limit tree is constructed") {
        THEN("the initial parameters are correct") {
            LimitTree<Side::Sell> tree;
            REQUIRE(tree.root == nullptr);
            REQUIRE(tree.limits.size() == 0);
            REQUIRE(tree.best == nullptr);
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: limit
// ---------------------------------------------------------------------------

SCENARIO("add a single order to LimitTree") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    GIVEN("a LimitTree and a single buy order") {
        auto tree = LimitTree<Side::Buy>();
        WHEN("the order is added") {
            Account account;
            Order node = {1, Side::Buy, quantity, price, nullptr, &account};
            tree.limit(&node);
            THEN("the order is recorded in the tree data structures") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(price - 1));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(0 == tree.volume_at(price + 1));
                REQUIRE(0 == tree.count_at(price - 1));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(price + 1));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node == tree.best->order_head);
                REQUIRE(&node == tree.best->order_tail);
                REQUIRE(tree.best == tree.root);
            }
        }
    }
    GIVEN("a LimitTree and a single sell order") {
        auto tree = LimitTree<Side::Sell>();
        WHEN("the order is added") {
            Account account;
            Order node = {1, Side::Sell, quantity, price, nullptr, &account};
            tree.limit(&node);
            THEN("the order is recorded in the tree data structures") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(price - 1));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(0 == tree.volume_at(price + 1));
                REQUIRE(0 == tree.count_at(price - 1));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(price + 1));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node == tree.best->order_head);
                REQUIRE(&node == tree.best->order_tail);
                REQUIRE(tree.best == tree.root);
            }
        }
    }
}

SCENARIO("add two orders to LimitTree (best first)") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    auto priceHigher = price + 1;
    GIVEN("a LimitTree and 2 buy orders") {
        auto tree = LimitTree<Side::Buy>();
        WHEN("the orders are added") {
            Account account;
            Order node1 = {1, Side::Buy, quantity, priceHigher, nullptr, &account};
            tree.limit(&node1);
            Order node2 = {2, Side::Buy, quantity, price, nullptr, &account};
            tree.limit(&node2);
            THEN("the orders are recorded in the tree data structures") {
                REQUIRE(2 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(priceHigher == tree.best->key);
                REQUIRE(&node1 == tree.best->order_head);
                REQUIRE(&node1 == tree.best->order_tail);
            }
        }
    }
    GIVEN("a LimitTree and 2 sell orders") {
        auto tree = LimitTree<Side::Sell>();
        WHEN("the orders are added") {
            Account account;
            Order node1 = {1, Side::Sell, quantity, price, nullptr, &account};
            tree.limit(&node1);
            Order node2 = {2, Side::Buy, quantity, priceHigher, nullptr, &account};
            tree.limit(&node2);
            THEN("the orders are recorded in the tree data structures") {
                REQUIRE(2 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node1 == tree.best->order_head);
                REQUIRE(&node1 == tree.best->order_tail);
            }
        }
    }
}

SCENARIO("add two orders to LimitTree (best last)") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    auto priceHigher = price + 1;
    GIVEN("a LimitTree and 2 buy orders") {
        auto tree = LimitTree<Side::Buy>();
        WHEN("the orders are added") {
            Account account;
            Order node1 = {1, Side::Buy, quantity, price, nullptr, &account};
            tree.limit(&node1);
            Order node2 = {2, Side::Buy, quantity, priceHigher, nullptr, &account};
            tree.limit(&node2);
            THEN("the orders are recorded in the tree data structures") {
                REQUIRE(2 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best != tree.root);
                REQUIRE(priceHigher == tree.best->key);
                REQUIRE(&node2 == tree.best->order_head);
                REQUIRE(&node2 == tree.best->order_tail);
            }
        }
    }
    GIVEN("a LimitTree and 2 sell orders") {
        auto tree = LimitTree<Side::Sell>();
        WHEN("the orders are added") {
            Account account;
            Order node1 = {1, Side::Sell, quantity, priceHigher, nullptr, &account};
            tree.limit(&node1);
            Order node2 = {2, Side::Buy, quantity, price, nullptr, &account};
            tree.limit(&node2);
            THEN("the orders are recorded in the tree data structures") {
                REQUIRE(2 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best != tree.root);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node2 == tree.best->order_head);
                REQUIRE(&node2 == tree.best->order_tail);
            }
        }
    }
}

SCENARIO("add two orders to LimitTree (same price)") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    GIVEN("a LimitTree and 2 orders with the same price") {
        // only need to test buy because this logic is side independent
        auto tree = LimitTree<Side::Buy>();
        WHEN("the orders are added") {
            Account account;
            Order node1 = {1, Side::Buy, quantity, price, nullptr, &account};
            tree.limit(&node1);
            Order node2 = {2, Side::Buy, quantity, price, nullptr, &account};
            tree.limit(&node2);
            THEN("the orders are recorded in the tree data structures") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(2 * quantity == tree.volume_at(price));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(2 == tree.count_at(price));
                REQUIRE(tree.best == tree.root);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node1 == tree.best->order_head);
                REQUIRE(&node2 == tree.best->order_tail);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: remove
// ---------------------------------------------------------------------------

SCENARIO("remove a single order from LimitTree") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    GIVEN("a LimitTree with a single buy order") {
        auto tree = LimitTree<Side::Buy>();
        Account account;
        Order node = {1, Side::Buy, quantity, price, nullptr, &account};
        tree.limit(&node);
        WHEN("the order is removed") {
            tree.remove(&node);
            THEN("the data structures are updated") {
                REQUIRE(0 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(nullptr == tree.root);
                REQUIRE(nullptr == tree.best);
            }
        }
    }
    GIVEN("a LimitTree and with a single sell order") {
        auto tree = LimitTree<Side::Sell>();
        Account account;
        Order node = {1, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node);
        WHEN("the order is removed") {
            tree.remove(&node);
            THEN("the data structures are updated") {
                REQUIRE(0 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(nullptr == tree.root);
                REQUIRE(nullptr == tree.best);
            }
        }
    }
    GIVEN("a LimitTree and with 2 single sell orders of the same price") {
        // just need to test sell because logic is side independent
        auto tree = LimitTree<Side::Sell>();
        Account account;
        Order node1 = {1, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node2);
        WHEN("the first order is removed") {
            tree.remove(&node1);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(tree.root == tree.best);
                REQUIRE(&node2 == tree.root->order_head);
                REQUIRE(&node2 == tree.root->order_tail);
            }
        }
        WHEN("the second order is removed") {
            tree.remove(&node2);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(tree.root == tree.best);
                REQUIRE(&node1 == tree.root->order_head);
                REQUIRE(&node1 == tree.root->order_tail);
            }
        }
    }
}

SCENARIO("remove an order from LimitTree (best first)") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    auto priceHigher = price + 1;
    GIVEN("a LimitTree with 2 buy orders") {
        auto tree = LimitTree<Side::Buy>();
        Account account;
        Order node1 = {1, Side::Buy, quantity, priceHigher, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Buy, quantity, price, nullptr, &account};
        tree.limit(&node2);
        WHEN("the best order is removed") {
            tree.remove(&node1);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node2 == tree.best->order_head);
                REQUIRE(&node2 == tree.best->order_tail);
            }
        }
        WHEN("the arbitrary order is removed") {
            tree.remove(&node2);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(priceHigher == tree.best->key);
                REQUIRE(&node1 == tree.best->order_head);
                REQUIRE(&node1 == tree.best->order_tail);
            }
        }
    }
    GIVEN("a LimitTree with 2 sell orders") {
        auto tree = LimitTree<Side::Sell>();
        Account account;
        Order node1 = {1, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Buy, quantity, priceHigher, nullptr, &account};
        tree.limit(&node2);
        WHEN("the best order is removed") {
            tree.remove(&node1);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(priceHigher == tree.best->key);
                REQUIRE(&node2 == tree.best->order_head);
                REQUIRE(&node2 == tree.best->order_tail);
            }
        }
        WHEN("the arbitrary order is removed") {
            tree.remove(&node2);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node1 == tree.best->order_head);
                REQUIRE(&node1 == tree.best->order_tail);
            }
        }
    }
}

SCENARIO("remove an order from LimitTree (best last)") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    auto priceHigher = price + 1;
    GIVEN("a LimitTree with 2 buy orders") {
        auto tree = LimitTree<Side::Buy>();
        Account account;
        Order node1 = {1, Side::Buy, quantity, price, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Buy, quantity, priceHigher, nullptr, &account};
        tree.limit(&node2);
        WHEN("the best order is removed") {
            tree.remove(&node2);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node1 == tree.best->order_head);
                REQUIRE(&node1 == tree.best->order_tail);
            }
        }
        WHEN("the arbitrary order is removed") {
            tree.remove(&node1);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(priceHigher == tree.best->key);
                REQUIRE(&node2 == tree.best->order_head);
                REQUIRE(&node2 == tree.best->order_tail);
            }
        }
    }
    GIVEN("a LimitTree with 2 sell orders") {
        auto tree = LimitTree<Side::Sell>();
        Account account;
        Order node1 = {1, Side::Sell, quantity, priceHigher, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Buy, quantity, price, nullptr, &account};
        tree.limit(&node2);
        WHEN("the best order is removed") {
            tree.remove(&node2);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(quantity == tree.volume_at(priceHigher));
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(1 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(priceHigher == tree.best->key);
                REQUIRE(&node1 == tree.best->order_head);
                REQUIRE(&node1 == tree.best->order_tail);
            }
        }
        WHEN("the arbitrary order is removed") {
            tree.remove(&node1);
            THEN("the data structures are updated") {
                REQUIRE(1 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(priceHigher));
                REQUIRE(quantity == tree.volume_at(price));
                REQUIRE(1 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(priceHigher));
                REQUIRE(nullptr != tree.root);
                REQUIRE(nullptr != tree.best);
                REQUIRE(tree.best == tree.root);
                REQUIRE(price == tree.best->key);
                REQUIRE(&node2 == tree.best->order_head);
                REQUIRE(&node2 == tree.best->order_tail);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: market
// ---------------------------------------------------------------------------

SCENARIO("a market order is submitted with no order in the tree") {
    GIVEN("An empty limit tree") {
        Account account;
        LimitTree<Side::Buy> tree;
        Quantity quantity = 100;
        WHEN("a buy market order is submitted") {
            Order market = {1, Side::Sell, quantity, 0, nullptr, &account};
            tree.market(&market, [](UID) { });
            THEN("the account should not be updated") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted to tree with a perfect match") {
    GIVEN("An order book with a limit order and a matched market order") {
        Quantity quantity = 100;
        Price price = 50;
        Account maker;
        Account taker;
        LimitTree<Side::Buy> tree;

        WHEN("a sell market order is matched to a buy limit order") {
            Order limit = {1, Side::Buy, quantity, price, nullptr, &maker};
            tree.limit(&limit);
            Order market = {2, Side::Sell, quantity, 0, nullptr, &taker};
            tree.market(&market, [](UID) { });
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(nullptr == tree.best);
                REQUIRE(0 == tree.volume_at(price));
            }
            THEN("the maker account should be updated") {
                REQUIRE(maker.shares == quantity);
                REQUIRE(maker.capital == -price * quantity);
                REQUIRE(maker.orders.empty());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity));
                REQUIRE(taker.capital == price * quantity);
                REQUIRE(taker.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted that is partially filled") {
    GIVEN("An order book with a limit order and a smaller market order") {
        Quantity quantity_limit = 100;
        Quantity quantity_market = 20;
        Price price = 50;
        Account maker;
        Account taker;
        LimitTree<Side::Buy> tree;
        Order limit = {1, Side::Buy, quantity_limit, price, nullptr, &maker};
        tree.limit(&limit);

        WHEN("a buy market order is submitted") {
            Order market = {2, Side::Sell, quantity_market, 0, nullptr, &taker};
            tree.market(&market, [](UID) { });
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(nullptr != tree.best);
                REQUIRE(price == tree.best->key);
                REQUIRE(quantity_limit - quantity_market == tree.volume_at(price));
            }
            THEN("the maker account should be updated") {
                REQUIRE(maker.shares == quantity_market);
                REQUIRE(maker.capital == -price * quantity_market);
                REQUIRE(maker.orders.size() == 1);
                REQUIRE(maker.orders.find(&limit) != maker.orders.end());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity_market));
                REQUIRE(taker.capital == price * quantity_market);
                REQUIRE(taker.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted that spans several limit orders") {
    GIVEN("An order book with two limits and a market order that requires both") {
        Quantity quantity_limit1 = 40;
        Quantity quantity_limit2 = 20;
        Quantity quantity_market = 50;
        Price price = 100;
        Account maker1;
        Account maker2;
        Account taker;
        LimitTree<Side::Buy> tree;
        Order limit1 = {1, Side::Buy, quantity_limit1, price, nullptr, &maker1};
        tree.limit(&limit1);
        Order limit2 = {2, Side::Buy, quantity_limit2, price, nullptr, &maker2};
        tree.limit(&limit2);

        WHEN("a buy market order is submitted") {
            Order market = {3, Side::Sell, quantity_market, 0, nullptr, &taker};
            tree.market(&market, [](UID) { });
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(nullptr != tree.best);
                REQUIRE(100 == tree.best->key);
                REQUIRE(quantity_limit1 + quantity_limit2 - quantity_market == tree.volume_at(price));
            }
            THEN("the maker1 account should be updated") {
                REQUIRE(maker1.shares == quantity_limit1);
                REQUIRE(maker1.capital == -price * quantity_limit1);
                REQUIRE(maker1.orders.empty());
            }
            THEN("the maker2 account should be updated") {
                REQUIRE(maker2.shares == quantity_market - quantity_limit1);
                REQUIRE(maker2.capital == -price * (quantity_market - quantity_limit1));
                REQUIRE(maker2.orders.size() == 1);
                REQUIRE(maker2.orders.find(&limit2) != maker2.orders.end());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity_market));
                REQUIRE(taker.capital == price * quantity_market);
                REQUIRE(taker.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted that spans several limit orders and clears book") {
    GIVEN("An order book with two limits and a market order that requires both") {
        Quantity quantity_limit1 = 20;
        Quantity quantity_limit2 = 20;
        Quantity quantity_market = 50;
        Price price = 100;

        Account maker1;
        Account maker2;
        Account taker;
        LimitTree<Side::Buy> tree;
        Order limit1 = {1, Side::Buy, quantity_limit1, price, nullptr, &maker1};
        tree.limit(&limit1);
        Order limit2 = {2, Side::Buy, quantity_limit2, price, nullptr, &maker2};
        tree.limit(&limit2);

        WHEN("a buy market order is submitted") {
            Order market = {3, Side::Sell, quantity_market, 0, nullptr, &taker};
            tree.market(&market, [](UID) { });
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(nullptr == tree.best);
                REQUIRE(0 == tree.volume_at(price));
            }
            THEN("the maker1 account should be updated") {
                REQUIRE(maker1.shares == quantity_limit1);
                REQUIRE(maker1.capital == -price * quantity_limit1);
                REQUIRE(maker1.orders.empty());
            }
            THEN("the maker2 account should be updated") {
                REQUIRE(maker2.shares == quantity_limit2);
                REQUIRE(maker2.capital == -price * quantity_limit2);
                REQUIRE(maker2.orders.empty());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity_limit1 + quantity_limit2));
                REQUIRE(taker.capital == price * (quantity_limit1 + quantity_limit2));
                REQUIRE(taker.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted that spans several limit prices and clears book") {
    GIVEN("An order book with two limits and a market order that requires both") {
        Quantity quantity = 20;
        Price price1 = 101;
        Price price2 = 102;

        Account maker1;
        Account maker2;
        Account taker;
        LimitTree<Side::Buy> tree;
        Order limit1 = {1, Side::Buy, quantity, price1, nullptr, &maker1};
        tree.limit(&limit1);
        Order limit2 = {2, Side::Buy, quantity, price2, nullptr, &maker2};
        tree.limit(&limit2);

        WHEN("a buy market order is submitted") {
            Order market = {3, Side::Sell, 2 * quantity, 0, nullptr, &taker};
            tree.market(&market, [](UID) { });
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(nullptr == tree.best);
                REQUIRE(0 == tree.volume_at(price1));
                REQUIRE(0 == tree.volume_at(price2));
            }
            THEN("the maker1 account should be updated") {
                REQUIRE(maker1.shares == quantity);
                REQUIRE(maker1.capital == -price1 * quantity);
                REQUIRE(maker1.orders.empty());
            }
            THEN("the maker2 account should be updated") {
                REQUIRE(maker2.shares == quantity);
                REQUIRE(maker2.capital == -price2 * quantity);
                REQUIRE(maker2.orders.empty());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(2 * quantity));
                REQUIRE(taker.capital == quantity * (price1 + price2));
                REQUIRE(taker.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted with a limit price") {
    GIVEN("A book with two limits and a market order with limit price") {
        Quantity quantity_limit1 = 20;
        Quantity quantity_limit2 = 20;
        Quantity quantity_market = 40;
        Price price1 = 100;
        Price price2 = 101;
        Account maker1;
        Account maker2;
        Account taker;
        LimitTree<Side::Buy> tree;
        Order limit1 = {1, Side::Buy, quantity_limit1, price1, nullptr, &maker1};
        tree.limit(&limit1);
        Order limit2 = {2, Side::Buy, quantity_limit2, price2, nullptr, &maker2};
        tree.limit(&limit2);

        WHEN("a buy market order is submitted") {
            Order market = {3, Side::Sell, quantity_market, price2, nullptr, &taker};
            tree.market(&market, [](UID) { });
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(nullptr != tree.best);
                REQUIRE(100 == tree.best->key);
                REQUIRE(20 == tree.volume_at(price1));
                REQUIRE(0 == tree.volume_at(price2));
            }
            THEN("the maker1 account should be updated") {
                REQUIRE(maker1.shares == 0);
                REQUIRE(maker1.capital == 0);
                REQUIRE(maker1.orders.size() == 1);
                REQUIRE(maker1.orders.find(&limit1) != maker1.orders.end());
            }
            THEN("the maker2 account should be updated") {
                REQUIRE(maker2.shares == quantity_limit2);
                REQUIRE(maker2.capital == -price2 * quantity_limit2);
                REQUIRE(maker2.orders.empty());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity_limit2));
                REQUIRE(taker.capital == price2 * quantity_limit2);
                REQUIRE(taker.orders.empty());
            }
            THEN("the order should have shares leftover") {
                REQUIRE(quantity_market - quantity_limit2 == market.quantity);
                REQUIRE(price2 == market.price);
            }
        }
    }
}

SCENARIO("a market order is submitted with a limit price that spans") {
    GIVEN("A book with two limits and a market order with limit price") {
        Quantity quantity_limit1 = 20;
        Quantity quantity_limit2 = 20;
        Quantity quantity_limit3 = 20;
        Quantity quantity_market = 60;
        Price price1 = 100;
        Price price2 = 101;
        Price price3 = 102;
        Account maker1;
        Account maker2;
        Account maker3;
        Account taker;
        LimitTree<Side::Buy> tree;
        Order limit1 = {1, Side::Buy, quantity_limit1, price1, nullptr, &maker1};
        tree.limit(&limit1);
        Order limit2 = {2, Side::Buy, quantity_limit2, price2, nullptr, &maker2};
        tree.limit(&limit2);
        Order limit3 = {3, Side::Buy, quantity_limit3, price3, nullptr, &maker3};
        tree.limit(&limit3);

        WHEN("a buy market order is submitted") {
            Order market = {4, Side::Sell, quantity_market, price2, nullptr, &taker};
            tree.market(&market, [](UID) { });
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(nullptr != tree.best);
                REQUIRE(100 == tree.best->key);
                REQUIRE(20 == tree.volume_at(price1));
                REQUIRE(0 == tree.volume_at(price2));
                REQUIRE(0 == tree.volume_at(price3));
            }
            THEN("the maker1 account should be updated") {
                REQUIRE(maker1.shares == 0);
                REQUIRE(maker1.capital == 0);
                REQUIRE(maker1.orders.size() == 1);
                REQUIRE(maker1.orders.find(&limit1) != maker1.orders.end());
            }
            THEN("the maker2 account should be updated") {
                REQUIRE(maker2.shares == quantity_limit2);
                REQUIRE(maker2.capital == -price2 * quantity_limit2);
                REQUIRE(maker2.orders.empty());

            }
            THEN("the maker3 account should be updated") {
                REQUIRE(maker3.shares == quantity_limit3);
                REQUIRE(maker3.capital == -price3 * quantity_limit3);
                REQUIRE(maker3.orders.empty());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity_limit3 + quantity_limit2));
                REQUIRE(taker.capital == (price2 * quantity_limit2) + (price3 *quantity_limit3));
                REQUIRE(taker.orders.empty());

            }
            THEN("the order should have shares leftover") {
                REQUIRE(quantity_market - (quantity_limit2 + quantity_limit3) == market.quantity);
                REQUIRE(price2 == market.price);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: clear
// ---------------------------------------------------------------------------

SCENARIO("clear a single limit from LimitTree") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    GIVEN("a LimitTree with a single buy order") {
        auto tree = LimitTree<Side::Buy>();
        Account account;
        Order node = {1, Side::Buy, quantity, price, nullptr, &account};
        tree.limit(&node);
        WHEN("the tree is cleared") {
            tree.clear();
            THEN("the data structures are updated") {
                REQUIRE(0 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(nullptr == tree.root);
                REQUIRE(nullptr == tree.best);
            }
        }
    }
    GIVEN("a LimitTree and with a single sell order") {
        auto tree = LimitTree<Side::Sell>();
        Account account;
        Order node = {1, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node);
        WHEN("the tree is cleared") {
            tree.clear();
            THEN("the data structures are updated") {
                REQUIRE(0 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(nullptr == tree.root);
                REQUIRE(nullptr == tree.best);
            }
        }
    }
    GIVEN("a LimitTree and with 2 single sell orders of the same price") {
        // just need to test sell because logic is side independent
        auto tree = LimitTree<Side::Sell>();
        Account account;
        Order node1 = {1, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node2);
        WHEN("the tree is cleared") {
            tree.clear();
            THEN("the data structures are updated") {
                REQUIRE(0 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(nullptr == tree.root);
                REQUIRE(nullptr == tree.best);
            }
        }
    }
}

SCENARIO("clear multiple limits from the tree") {
    Quantity quantity = 0x4545;
    Price price = 0xAABBCCDD00112233;
    auto priceHigher = price + 1;
    GIVEN("a LimitTree with 2 buy orders") {
        auto tree = LimitTree<Side::Buy>();
        Account account;
        Order node1 = {1, Side::Buy, quantity, priceHigher, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Buy, quantity, price, nullptr, &account};
        tree.limit(&node2);
        WHEN("the tree is cleared") {
            tree.clear();
            THEN("the data structures are updated") {
                REQUIRE(0 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(priceHigher));
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(priceHigher));
                REQUIRE(nullptr == tree.root);
                REQUIRE(nullptr == tree.best);
            }
        }
    }
    GIVEN("a LimitTree with 2 sell orders") {
        auto tree = LimitTree<Side::Sell>();
        Account account;
        Order node1 = {1, Side::Sell, quantity, price, nullptr, &account};
        tree.limit(&node1);
        Order node2 = {2, Side::Buy, quantity, priceHigher, nullptr, &account};
        tree.limit(&node2);
        WHEN("the tree is cleared") {
            tree.clear();
            THEN("the data structures are updated") {
                REQUIRE(0 == tree.limits.size());
                REQUIRE(0 == tree.volume_at(priceHigher));
                REQUIRE(0 == tree.volume_at(price));
                REQUIRE(0 == tree.count_at(price));
                REQUIRE(0 == tree.count_at(priceHigher));
                REQUIRE(nullptr == tree.root);
                REQUIRE(nullptr == tree.best);
            }
        }
    }
}
