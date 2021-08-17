// Test cases for the Limit Order Book (LOB) Object.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "order_entry/limit_order_book/limit_order_book.hpp"

using namespace OrderEntry::LOB;

// ---------------------------------------------------------------------------
// MARK: Constructor
// ---------------------------------------------------------------------------

SCENARIO("initialize LimitOrderBook") {
    GIVEN("default parameters") {
        WHEN("a LimitOrderBook is initialized with no parameters") {
            REQUIRE_NOTHROW(new LimitOrderBook());
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: limit
// ---------------------------------------------------------------------------

SCENARIO("send single order to LimitOrderBook") {
    GIVEN("an order book and a single sell order") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantity = 57;
        Price price = 0xFEDCBA9876543210;
        WHEN("the order is sent") {
            auto uid = book.limit(&account, side, quantity, price);
            THEN("order ID is returned and the order is recorded") {
                // the first order should have ID 1
                REQUIRE(1 == uid);
                REQUIRE(quantity == book.volume(price));
                REQUIRE(0 == book.volume(price - 1));
                REQUIRE(0 == book.volume(price + 1));
                REQUIRE(0 == book.best_buy());
                REQUIRE(price == book.best_sell());
            }
            THEN("the account information is correct") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.size() == 1);
                REQUIRE(account.orders.find(book.get_pointer(uid)) != account.orders.end());
            }
        }
    }
    GIVEN("an order book and a single buy order") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        Quantity quantity = 57;
        Price price = 0xFEDCBA9876543210;
        WHEN("the order is sent") {
            auto uid = book.limit(&account, side, quantity, price);
            THEN("order ID is returned and the order is recorded") {
                // the first order should have ID 1
                REQUIRE(1 == uid);
                REQUIRE(quantity == book.volume(price));
                REQUIRE(0 == book.volume(price - 1));
                REQUIRE(0 == book.volume(price + 1));
                REQUIRE(price == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
            THEN("the account information is correct") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.size() == 1);
                REQUIRE(account.orders.find(book.get_pointer(uid)) != account.orders.end());
            }
        }
    }
}

SCENARIO("send homogeneous orders to LimitOrderBook at same price") {
    GIVEN("an order book and a series of sell orders") {
        auto book = LimitOrderBook();
        Account accountA;
        Account accountB;
        Account accountC;
        auto side = Side::Sell;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 0xFEDCBA9876543210;
        WHEN("the orders are sent") {
            auto uidA = book.limit(&accountA, side, quantityA, price);
            auto uidB = book.limit(&accountB, side, quantityB, price);
            auto uidC = book.limit(&accountC, side, quantityC, price);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(1 == uidA);
                REQUIRE(2 == uidB);
                REQUIRE(3 == uidC);
                REQUIRE(quantityA + quantityB + quantityC == book.volume(price));
                REQUIRE(0 == book.volume(price - 1));
                REQUIRE(0 == book.volume(price + 1));
                REQUIRE(0 == book.best_buy());
                REQUIRE(price == book.best_sell());
            }
            THEN("the accountA information is correct") {
                REQUIRE(accountA.shares == 0);
                REQUIRE(accountA.capital == 0);
                REQUIRE(accountA.orders.size() == 1);
                REQUIRE(accountA.orders.find(book.get_pointer(uidA)) != accountA.orders.end());
            }
            THEN("the accountB information is correct") {
                REQUIRE(accountB.shares == 0);
                REQUIRE(accountB.capital == 0);
                REQUIRE(accountB.orders.size() == 1);
                REQUIRE(accountB.orders.find(book.get_pointer(uidB)) != accountB.orders.end());
            }
            THEN("the accountC information is correct") {
                REQUIRE(accountC.shares == 0);
                REQUIRE(accountC.capital == 0);
                REQUIRE(accountC.orders.size() == 1);
                REQUIRE(accountC.orders.find(book.get_pointer(uidC)) != accountC.orders.end());
            }
        }
    }
    GIVEN("an order book and a series of buy orders") {
        auto book = LimitOrderBook();
        Account accountA;
        Account accountB;
        Account accountC;
        auto side = Side::Buy;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 0xFEDCBA9876543210;
        WHEN("the orders are sent") {
            auto uidA = book.limit(&accountA, side, quantityA, price);
            auto uidB = book.limit(&accountB, side, quantityB, price);
            auto uidC = book.limit(&accountC, side, quantityC, price);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(1 == uidA);
                REQUIRE(2 == uidB);
                REQUIRE(3 == uidC);
                REQUIRE(quantityA + quantityB + quantityC == book.volume(price));
                REQUIRE(0 == book.volume(price - 1));
                REQUIRE(0 == book.volume(price + 1));
                REQUIRE(price == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
            THEN("the accountA information is correct") {
                REQUIRE(accountA.shares == 0);
                REQUIRE(accountA.capital == 0);
                REQUIRE(accountA.orders.size() == 1);
                REQUIRE(accountA.orders.find(book.get_pointer(uidA)) != accountA.orders.end());
            }
            THEN("the accountB information is correct") {
                REQUIRE(accountB.shares == 0);
                REQUIRE(accountB.capital == 0);
                REQUIRE(accountB.orders.size() == 1);
                REQUIRE(accountB.orders.find(book.get_pointer(uidB)) != accountB.orders.end());
            }
            THEN("the accountC information is correct") {
                REQUIRE(accountC.shares == 0);
                REQUIRE(accountC.capital == 0);
                REQUIRE(accountC.orders.size() == 1);
                REQUIRE(accountC.orders.find(book.get_pointer(uidC)) != accountC.orders.end());
            }
        }
    }
}

SCENARIO("send homogeneous orders to LimitOrderBook at different prices") {
    GIVEN("an order book and a series of sell orders") {
        auto book = LimitOrderBook();
        Account accountA;
        Account accountB;
        Account accountC;
        auto side = Side::Sell;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price priceA = 3253;
        Price priceB = 3240;
        Price priceC = 3245;
        WHEN("the orders are sent") {
            auto uidA = book.limit(&accountA, side, quantityA, priceA);
            auto uidB = book.limit(&accountB, side, quantityB, priceB);
            auto uidC = book.limit(&accountC, side, quantityC, priceC);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(1 == uidA);
                REQUIRE(2 == uidB);
                REQUIRE(3 == uidC);
                REQUIRE(quantityA == book.volume(priceA));
                REQUIRE(quantityB == book.volume(priceB));
                REQUIRE(quantityC == book.volume(priceC));
                REQUIRE(0 == book.best_buy());
                REQUIRE(priceB == book.best_sell());
            }
            THEN("the accountA information is correct") {
                REQUIRE(accountA.shares == 0);
                REQUIRE(accountA.capital == 0);
                REQUIRE(accountA.orders.size() == 1);
                REQUIRE(accountA.orders.find(book.get_pointer(uidA)) != accountA.orders.end());
            }
            THEN("the accountB information is correct") {
                REQUIRE(accountB.shares == 0);
                REQUIRE(accountB.capital == 0);
                REQUIRE(accountB.orders.size() == 1);
                REQUIRE(accountB.orders.find(book.get_pointer(uidB)) != accountB.orders.end());
            }
            THEN("the accountC information is correct") {
                REQUIRE(accountC.shares == 0);
                REQUIRE(accountC.capital == 0);
                REQUIRE(accountC.orders.size() == 1);
                REQUIRE(accountC.orders.find(book.get_pointer(uidC)) != accountC.orders.end());
            }
        }
    }
    GIVEN("an order book and a series of buy orders") {
        auto book = LimitOrderBook();
        Account accountA;
        Account accountB;
        Account accountC;
        auto side = Side::Buy;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price priceA = 3240;
        Price priceB = 3253;
        Price priceC = 3245;
        WHEN("the orders are sent") {
            auto uidA = book.limit(&accountA, side, quantityA, priceA);
            auto uidB = book.limit(&accountB, side, quantityB, priceB);
            auto uidC = book.limit(&accountC, side, quantityC, priceC);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(1 == uidA);
                REQUIRE(2 == uidB);
                REQUIRE(3 == uidC);
                REQUIRE(quantityA == book.volume(priceA));
                REQUIRE(quantityB == book.volume(priceB));
                REQUIRE(quantityC == book.volume(priceC));
                REQUIRE(priceB == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
            THEN("the accountA information is correct") {
                REQUIRE(accountA.shares == 0);
                REQUIRE(accountA.capital == 0);
                REQUIRE(accountA.orders.size() == 1);
                REQUIRE(accountA.orders.find(book.get_pointer(uidA)) != accountA.orders.end());
            }
            THEN("the accountB information is correct") {
                REQUIRE(accountB.shares == 0);
                REQUIRE(accountB.capital == 0);
                REQUIRE(accountB.orders.size() == 1);
                REQUIRE(accountB.orders.find(book.get_pointer(uidB)) != accountB.orders.end());
            }
            THEN("the accountC information is correct") {
                REQUIRE(accountC.shares == 0);
                REQUIRE(accountC.capital == 0);
                REQUIRE(accountC.orders.size() == 1);
                REQUIRE(accountC.orders.find(book.get_pointer(uidC)) != accountC.orders.end());
            }
        }
    }
}

SCENARIO("a limit order is submitted that crosses") {
    GIVEN("a book with 2 buy limit orders") {
        auto book = LimitOrderBook();
        Account account1;
        Account account2;
        Account account3;
        auto side = Side::Buy;
        Quantity quantity = 20;
        Quantity quantityMarket = 40;
        Price priceA = 100;
        Price priceB = 101;
        auto uidA = book.limit(&account1, side, quantity, priceA);
        auto uidB = book.limit(&account2, side, quantity, priceB);
        WHEN("a sell limit order is sent that crosses") {
            auto uidC = book.limit(&account3, !side, quantityMarket, priceB);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(1 == uidA);
                REQUIRE(2 == uidB);
                REQUIRE(3 == uidC);
                // check the buy side
                REQUIRE(1 == book.count_buy());
                REQUIRE(quantity == book.volume_buy(priceA));
                REQUIRE(0 == book.volume_buy(priceB));
                REQUIRE(priceA == book.best_buy());
                // check the sell side
                REQUIRE(1 == book.count_sell());
                REQUIRE(quantityMarket - quantity == book.volume_sell(priceB));
                REQUIRE(priceB == book.best_sell());
            }
            THEN("the account1 information is correct") {
                REQUIRE(account1.shares == 0);
                REQUIRE(account1.capital == 0);
                REQUIRE(account1.orders.size() == 1);
                REQUIRE(account1.orders.find(book.get_pointer(uidA)) != account1.orders.end());
            }
            THEN("the account2 information is correct") {
                REQUIRE(account2.shares == quantity);
                REQUIRE(account2.capital == -priceB * quantity);
                REQUIRE(account2.orders.empty());
            }
            THEN("the account3 information is correct") {
                REQUIRE(account3.shares == -static_cast<Shares>(quantity));
                REQUIRE(account3.capital == priceB * quantity);
                REQUIRE(account3.orders.size() == 1);
                REQUIRE(account3.orders.find(book.get_pointer(uidC)) != account3.orders.end());
            }
        }
    }
    GIVEN("a book with 2 sell limit orders") {
        auto book = LimitOrderBook();
        Account account1;
        Account account2;
        Account account3;
        auto side = Side::Sell;
        Quantity quantity = 20;
        Quantity quantityMarket = 40;
        Price priceA = 101;
        Price priceB = 100;
        auto uidA = book.limit(&account1, side, quantity, priceA);
        auto uidB = book.limit(&account2, side, quantity, priceB);
        WHEN("a buy limit order is sent that crosses") {
            auto uidC = book.limit(&account3, !side, quantityMarket, priceB);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(1 == uidA);
                REQUIRE(2 == uidB);
                REQUIRE(3 == uidC);
                // check the sell side
                REQUIRE(1 == book.count_sell());
                REQUIRE(quantity == book.volume_sell(priceA));
                REQUIRE(0 == book.volume_sell(priceB));
                REQUIRE(priceA == book.best_sell());
                // check the buy side
                REQUIRE(1 == book.count_buy());
                REQUIRE(quantityMarket - quantity == book.volume_buy(priceB));
                REQUIRE(priceB == book.best_buy());
            }
            THEN("the account1 information is correct") {
                REQUIRE(account1.shares == 0);
                REQUIRE(account1.capital == 0);
                REQUIRE(account1.orders.size() == 1);
                REQUIRE(account1.orders.find(book.get_pointer(uidA)) != account1.orders.end());
            }
            THEN("the account2 information is correct") {
                REQUIRE(account2.shares == -static_cast<Shares>(quantity));
                REQUIRE(account2.capital == priceB * quantity);
                REQUIRE(account2.orders.empty());
            }
            THEN("the account3 information is correct") {
                REQUIRE(account3.shares == quantity);
                REQUIRE(account3.capital == -priceB * quantity);
                REQUIRE(account3.orders.size() == 1);
                REQUIRE(account3.orders.find(book.get_pointer(uidC)) != account3.orders.end());
            }
        }
    }
}

SCENARIO("a limit order is submitted that crosses and fills") {
    GIVEN("a book with a buy limit order") {
        auto book = LimitOrderBook();
        Account account1;
        Account account2;
        auto side = Side::Buy;
        Quantity quantity = 20;
        Price price = 100;
        book.limit(&account1, side, quantity, price);
        WHEN("the sell limit order is sent") {
            book.limit(&account2, !side, quantity, price);
            THEN("order ID is returned and the order is recorded") {
                // check the buy side
                REQUIRE(0 == book.count_buy());
                REQUIRE(0 == book.volume_buy(price));
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.best_buy());
                // check the sell side
                REQUIRE(0 == book.count_sell());
                REQUIRE(0 == book.volume_sell(price));
                REQUIRE(0 == book.best_sell());
            }
            THEN("the account1 information is correct") {
                REQUIRE(account1.shares == quantity);
                REQUIRE(account1.capital == -price * quantity);
                REQUIRE(account1.orders.empty());
            }
            THEN("the account2 information is correct") {
                REQUIRE(account2.shares == -static_cast<Shares>(quantity));
                REQUIRE(account2.capital == price * quantity);
                REQUIRE(account2.orders.empty());
            }
        }
    }
    GIVEN("a book with a sell limit order") {
        auto book = LimitOrderBook();
        Account account1;
        Account account2;
        auto side = Side::Sell;
        Quantity quantity = 20;
        Price price = 100;
        book.limit(&account1, side, quantity, price);
        WHEN("the sell limit order is sent") {
            book.limit(&account2, !side, quantity, price);
            THEN("order ID is returned and the order is recorded") {
                // check the sell side
                REQUIRE(0 == book.count_sell());
                REQUIRE(0 == book.volume_sell(price));
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.best_sell());
                // check the buy side
                REQUIRE(0 == book.count_buy());
                REQUIRE(0 == book.volume_buy(price));
                REQUIRE(0 == book.best_buy());
            }
            THEN("the account1 information is correct") {
                REQUIRE(account1.shares == -static_cast<Shares>(quantity));
                REQUIRE(account1.capital == price * quantity);
                REQUIRE(account1.orders.empty());
            }
            THEN("the account2 information is correct") {
                REQUIRE(account2.shares == quantity);
                REQUIRE(account2.capital == -price * quantity);
                REQUIRE(account2.orders.empty());
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: cancel
// ---------------------------------------------------------------------------

// limit removal

SCENARIO(R"(LOB
tree shape (single node):
*
)") {
    GIVEN("an order book and an ID for a single sell order") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantity = 50;
        Price price = 3253;
        auto uid = book.limit(&account, side, quantity, price);
        WHEN("the order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(price));
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, price);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(price));
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
    GIVEN("an order book and an ID for a single buy order") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        Quantity quantity = 50;
        Price price = 3253;
        auto uid = book.limit(&account, side, quantity, price);
        WHEN("the orders is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(price));
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, price);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(price));
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - left:
  1
 / \
0   2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with V shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        auto uid = book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the left order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the left order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[0]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with V shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        auto uid = book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the left order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the left order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[0]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - right:
  1
 / \
0   2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with V shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        auto uid = book.limit(&account, side, quantity, prices[2]);
        WHEN("the right order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the right order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[2]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with V shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        auto uid = book.limit(&account, side, quantity, prices[2]);
        WHEN("the right order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the right order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[2]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - root:
  1
 / \
0   2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with V shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        // submit the MIDDLE order first (price-wise)
        auto uid = book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with V shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        auto uid = book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - root:
0
 \
  1
   \
    2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with right leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        auto uid = book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[0]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with right leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        auto uid = book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[0]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - middle:
0
 \
  1
   \
    2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with right leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[0]);
        auto uid = book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the middle order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the middle order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with right leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[0]);
        auto uid = book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the middle order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the middle order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - leaf:
0
 \
  1
   \
    2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with right leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        auto uid = book.limit(&account, side, quantity, prices[2]);
        WHEN("the leaf order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the leaf order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[2]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with right leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        auto uid = book.limit(&account, side, quantity, prices[2]);
        WHEN("the leaf order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the leaf order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[2]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - root:
    2
   /
  1
 /
0
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with left leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        auto uid = book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[0]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[2]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with left leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        auto uid = book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[0]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[2]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(0 == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - middle:
    2
   /
  1
 /
0
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with left leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[2]);
        auto uid = book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[0]);
        WHEN("the middle order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the middle order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[0] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with left leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[2]);
        auto uid = book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[0]);
        WHEN("the middle order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the middle order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - leaf:
    2
   /
  1
 /
0
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with left leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[1]);
        auto uid = book.limit(&account, side, quantity, prices[0]);
        WHEN("the leaf order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the leaf order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[0]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[1] == book.best_sell());
            }
        }
    }
    GIVEN("an order book with left leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[1]);
        auto uid = book.limit(&account, side, quantity, prices[0]);
        WHEN("the leaf order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the leaf order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[0]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(0 == book.volume(prices[0]));
                REQUIRE(quantity == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - root:
  2
 / \
1   4
   /
  3
)") {
    Quantity quantity = 50;
    Price prices[4] = {1, 2, 4, 3};
    GIVEN("an order book with right subtree with left branch--shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        auto uid = book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        // the last price will be the left branch of the right child
        book.limit(&account, side, quantity, prices[3]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - root:
  2
 / \
1   5
   /
  3
   \
    4
)") {
    Quantity quantity = 50;
    Price prices[5] = {1, 2, 5, 3, 4};
    GIVEN("an order book with right subtree with left branch and terminal right child--shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        auto uid = book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        // the last price will be the left branch of the right child
        book.limit(&account, side, quantity, prices[3]);
        // the last price will be the left branch of the right child
        book.limit(&account, side, quantity, prices[4]);
        WHEN("the root order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(quantity == book.volume(prices[4]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the root order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(quantity == book.volume(prices[4]));
                REQUIRE(prices[2] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - node 4:
1
 \
  4
 /
2
 \
  3
)") {
    Quantity quantity = 50;
    Price prices[4] = {1, 4, 2, 3};
    GIVEN("an order book with right zigzag--shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[0]);
        auto uid = book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[3]);
        WHEN("the root-child order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(prices[3] == book.best_buy());
            }
        }
        // tree integrity check
        WHEN("the root-child order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(prices[3] == book.best_buy());
            }
        }
    }
}

SCENARIO(R"(LOB
tree shape - node 1:
  4
 /
1
 \
  3
 /
2
)") {
    Quantity quantity = 50;
    Price prices[4] = {4, 1, 3, 2};
    GIVEN("an order book with left zigzag--shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[0]);
        auto uid = book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[3]);
        WHEN("the root-child order is canceled") {
            book.cancel(uid);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(prices[3] == book.best_sell());
            }
        }
        // tree integrity check
        WHEN("the root-child order is duplicated, added, and canceled again") {
            book.cancel(uid);
            UID uid1 = book.limit(&account, side, quantity, prices[1]);
            book.cancel(uid1);
            THEN("the limit is cleared") {
                REQUIRE(quantity == book.volume(prices[0]));
                REQUIRE(0 == book.volume(prices[1]));
                REQUIRE(quantity == book.volume(prices[2]));
                REQUIRE(quantity == book.volume(prices[3]));
                REQUIRE(prices[3] == book.best_sell());
            }
        }
    }
}

// order removal

SCENARIO("cancel first order in a Limit queue of orders") {
    GIVEN("an order book and a Limit queue of sell orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        auto uid = book.limit(&account, side, quantityA, price);
        book.limit(&account, side, quantityB, price);
        book.limit(&account, side, quantityC, price);
        WHEN("the first order is canceled") {
            book.cancel(uid);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(quantityB + quantityC == book.volume(price));
                REQUIRE(0 == book.best_buy());
                REQUIRE(price == book.best_sell());
            }
        }
    }
    GIVEN("an order book and a Limit queue of buy orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        auto uid = book.limit(&account, side, quantityA, price);
        book.limit(&account, side, quantityB, price);
        book.limit(&account, side, quantityC, price);
        WHEN("the first order is canceled") {
            book.cancel(uid);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(quantityB + quantityC == book.volume(price));
                REQUIRE(price == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO("cancel middle order in a Limit queue of orders") {
    GIVEN("an order book and a Limit queue of sell orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        book.limit(&account, side, quantityA, price);
        auto uid = book.limit(&account, side, quantityB, price);
        book.limit(&account, side, quantityC, price);
        WHEN("the middle order is canceled") {
            book.cancel(uid);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(quantityA + quantityC == book.volume(price));
                REQUIRE(0 == book.best_buy());
                REQUIRE(price == book.best_sell());
            }
        }
    }
    GIVEN("an order book and a Limit queue of buy orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        book.limit(&account, side, quantityA, price);
        auto uid = book.limit(&account, side, quantityB, price);
        book.limit(&account, side, quantityC, price);
        WHEN("the middle order is canceled") {
            book.cancel(uid);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(quantityA + quantityC == book.volume(price));
                REQUIRE(price == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO("cancel last order in a Limit queue of orders") {
    GIVEN("an order book and a Limit queue of sell orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        book.limit(&account, side, quantityA, price);
        book.limit(&account, side, quantityB, price);
        auto uid = book.limit(&account, side, quantityC, price);
        WHEN("the last order is canceled") {
            book.cancel(uid);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(quantityA + quantityB == book.volume(price));
                REQUIRE(0 == book.best_buy());
                REQUIRE(price == book.best_sell());
            }
        }
    }
    GIVEN("an order book and a Limit queue of buy orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        book.limit(&account, side, quantityA, price);
        book.limit(&account, side, quantityB, price);
        auto uid = book.limit(&account, side, quantityC, price);
        WHEN("the last order is canceled") {
            book.cancel(uid);
            THEN("order ID is returned and the order is recorded") {
                REQUIRE(quantityA + quantityB == book.volume(price));
                REQUIRE(price == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: market
// ---------------------------------------------------------------------------

SCENARIO("a market order is submitted with no order in the book") {
    GIVEN("An empty limit order book") {
        Account account;
        LimitOrderBook book;
        Quantity quantity = 100;
        WHEN("a buy market order is submitted") {
            book.market(&account, Side::Sell, quantity);
            THEN("the account should not be updated") {
                REQUIRE(account.shares == 0);
                REQUIRE(account.capital == 0);
                REQUIRE(account.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted with a perfect match") {
    GIVEN("An an order book with a limit order and a matched market order") {
        Quantity quantity = 100;
        Price price = 50;
        Account maker;
        Account taker;
        auto book = LimitOrderBook();

        WHEN("a sell market order is matched to a buy limit order") {
            book.limit(&maker, Side::Buy, quantity, price);
            book.market(&taker, Side::Sell, quantity);
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.volume(price));
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

SCENARIO("a market order is submitted that is partially filled by a limit") {
    GIVEN("An order book with a limit order and a smaller market order") {
        Quantity quantity_limit = 100;
        Quantity quantity_market = 20;
        Price price = 50;
        Account maker;
        Account taker;
        auto book = LimitOrderBook();
        auto uid = book.limit(&maker, Side::Buy, quantity_limit, price);

        WHEN("a buy market order is submitted") {
            book.market(&taker, Side::Sell, quantity_market);
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(price == book.best_buy());
                REQUIRE(quantity_limit - quantity_market == book.volume(price));
            }
            THEN("the maker account should be updated") {
                REQUIRE(maker.shares == quantity_market);
                REQUIRE(maker.capital == -price * quantity_market);
                REQUIRE(maker.orders.size() == 1);
                REQUIRE(maker.orders.find(book.get_pointer(uid)) != maker.orders.end());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity_market));
                REQUIRE(taker.capital == price * quantity_market);
                REQUIRE(taker.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted that spans several limits") {
    GIVEN("An order book with two limits and a market order that requires both") {
        Quantity quantity_limit1 = 40;
        Quantity quantity_limit2 = 20;
        Quantity quantity_market = 50;
        Price price = 100;
        Account maker1;
        Account maker2;
        Account taker;
        auto book = LimitOrderBook();
        book.limit(&maker1, Side::Buy, quantity_limit1, price);
        auto uid_maker2 = book.limit(&maker2, Side::Buy, quantity_limit2, price);

        WHEN("a buy market order is submitted") {
            book.market(&taker, Side::Sell, quantity_market);
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(100 == book.best_buy());
                REQUIRE(quantity_limit1 + quantity_limit2 - quantity_market == book.volume(price));
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
                REQUIRE(maker2.orders.find(book.get_pointer(uid_maker2)) != maker2.orders.end());
            }
            THEN("the taker account should be updated") {
                REQUIRE(taker.shares == -static_cast<Shares>(quantity_market));
                REQUIRE(taker.capital == price * quantity_market);
                REQUIRE(taker.orders.empty());
            }
        }
    }
}

SCENARIO("a market order is submitted that spans several limits and depletes book") {
    GIVEN("An order book with two limits and a market order that requires both") {
        Quantity quantity_limit1 = 20;
        Quantity quantity_limit2 = 20;
        Quantity quantity_market = 50;
        Price price = 100;

        Account maker1;
        Account maker2;
        Account taker;
        auto book = LimitOrderBook();
        book.limit(&maker1, Side::Buy, quantity_limit1, price);
        book.limit(&maker2, Side::Buy, quantity_limit2, price);

        WHEN("a buy market order is submitted") {
            book.market(&taker, Side::Sell, quantity_market);
            THEN("the limit_fill and market_fill functions should fire") {
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.volume(price));
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

// ---------------------------------------------------------------------------
// MARK: clear
// ---------------------------------------------------------------------------

SCENARIO(R"(LOB
clear()
tree shape (single node):
*
)") {
    GIVEN("an order book and an ID for a single sell order") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantity = 50;
        Price price = 3253;
        book.limit(&account, side, quantity, price);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume(price));
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
    GIVEN("an order book and an ID for a single buy order") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        Quantity quantity = 50;
        Price price = 3253;
        book.limit(&account, side, quantity, price);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume(price));
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO(R"(LOB
clear()
tree shape:
  1
 / \
0   2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with V shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
    GIVEN("an order book with V shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}


SCENARIO(R"(LOB
clear()
tree shape:
0
 \
  1
   \
    2
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with right leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
    GIVEN("an order book with right leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO(R"(LOB
clear()
tree shape:
    2
   /
  1
 /
0
)") {
    Quantity quantity = 50;
    Price prices[3] = {1, 2, 3};
    GIVEN("an order book with left leg shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[0]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
    GIVEN("an order book with left leg shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[0]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO(R"(LOB
clear()
tree shape:
  2
 / \
1   4
   /
  3
)") {
    Quantity quantity = 50;
    Price prices[4] = {1, 2, 4, 3};
    GIVEN("an order book with right subtree with left branch--shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        // the last price will be the left branch of the right child
        book.limit(&account, side, quantity, prices[3]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO(R"(LOB
clear()
tree shape:
  2
 / \
1   5
   /
  3
   \
    4
)") {
    Quantity quantity = 50;
    Price prices[5] = {1, 2, 5, 3, 4};
    GIVEN("an order book with right subtree with left branch and terminal right child--shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        // submit the MIDDLE order first (price-wise)
        book.limit(&account, side, quantity, prices[1]);
        // the lowest price will be the left child
        book.limit(&account, side, quantity, prices[0]);
        // the highest price will be the right child
        book.limit(&account, side, quantity, prices[2]);
        // the last price will be the left branch of the right child
        book.limit(&account, side, quantity, prices[3]);
        // the last price will be the left branch of the right child
        book.limit(&account, side, quantity, prices[4]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO(R"(LOB
clear()
tree shape:
1
 \
  4
 /
2
 \
  3
)") {
    Quantity quantity = 50;
    Price prices[4] = {1, 4, 2, 3};
    GIVEN("an order book with right zigzag--shaped limit tree (buy)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[3]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

SCENARIO(R"(LOB
clear()
tree shape:
  4
 /
1
 \
  3
 /
2
)") {
    Quantity quantity = 50;
    Price prices[4] = {4, 1, 3, 2};
    GIVEN("an order book with left zigzag--shaped limit tree (sell)") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        book.limit(&account, side, quantity, prices[0]);
        book.limit(&account, side, quantity, prices[1]);
        book.limit(&account, side, quantity, prices[2]);
        book.limit(&account, side, quantity, prices[3]);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}


// order removal

SCENARIO("clear book with queue of orders at limit") {
    GIVEN("an order book and a Limit queue of sell orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        book.limit(&account, side, quantityA, price);
        book.limit(&account, side, quantityB, price);
        book.limit(&account, side, quantityC, price);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
    GIVEN("an order book and a Limit queue of buy orders") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Buy;
        Quantity quantityA = 50;
        Quantity quantityB = 40;
        Quantity quantityC = 30;
        Price price = 3253;
        book.limit(&account, side, quantityA, price);
        book.limit(&account, side, quantityB, price);
        book.limit(&account, side, quantityC, price);
        WHEN("the book is cleared") {
            book.clear();
            THEN("the book is cleared") {
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: reduce
// ---------------------------------------------------------------------------

SCENARIO("reduce the size of an active order") {
    GIVEN("an order book with an active order") {
        auto book = LimitOrderBook();
        Account account;
        auto side = Side::Sell;
        Quantity quantity = 50;
        Price price = 3000;
        auto uid = book.limit(&account, side, quantity, price);
        WHEN("the order quantity is reduced") {
            Quantity reduce = 20;
            book.reduce(uid, reduce);
            THEN("the book is updated") {
                REQUIRE(0 == book.count_buy());
                REQUIRE(1 == book.count_sell());
                REQUIRE(0 == book.volume_buy());
                REQUIRE(quantity - reduce == book.volume_sell());
                REQUIRE(quantity - reduce == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(price == book.best_sell());
                REQUIRE(quantity - reduce == book.get(uid).quantity);
            }
        }
        WHEN("the order quantity is reduced entirely") {
            Quantity reduce = 50;
            book.reduce(uid, reduce);
            THEN("the book is updated (the order is canceled)") {
                REQUIRE(0 == book.count_buy());
                REQUIRE(0 == book.count_sell());
                REQUIRE(0 == book.volume_buy());
                REQUIRE(0 == book.volume_sell());
                REQUIRE(0 == book.volume());
                REQUIRE(0 == book.best_buy());
                REQUIRE(0 == book.best_sell());
                REQUIRE_FALSE(book.has(uid));
            }
        }
        WHEN("the reduce quantity exceeds what is available") {
            Quantity reduce = 70;
            THEN("an error is thrown") {
                REQUIRE_THROWS(book.reduce(uid, reduce));
            }
        }
    }
}
