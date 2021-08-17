// Test cases for the rate of change (RoC) object.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "maths/rate_of_change.hpp"

// ---------------------------------------------------------------------------
// MARK: Constructor
// ---------------------------------------------------------------------------

SCENARIO("initialize an rate of change with length value") {
    WHEN("the RoC is initialized with length=min - 1") {
        auto size = Maths::RateOfChange<double, double>::LENGTH_MIN - 1;
        THEN("an error is thrown") { REQUIRE_THROWS(Maths::RateOfChange(size)); }
    }
    WHEN("the RoC is initialized with length=min") {
        auto size = Maths::RateOfChange<double, double>::LENGTH_MIN;
        THEN("no error is thrown") { REQUIRE_NOTHROW(Maths::RateOfChange(size)); }
        auto value = Maths::RateOfChange(size);
        THEN("get_length returns size")      { REQUIRE(size == value.get_length()); }
        THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
    }
    WHEN("the RoC is initialized with length=min + 1") {
        auto size = Maths::RateOfChange<double, double>::LENGTH_MIN + 1;
        THEN("no error is thrown") { REQUIRE_NOTHROW(Maths::RateOfChange(size)); }
        auto value = Maths::RateOfChange(size);
        THEN("get_length returns size")      { REQUIRE(size == value.get_length()); }
        THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
    }
    WHEN("the RoC is initialized with length=max - 1") {
        auto size = Maths::RateOfChange<double, double>::LENGTH_MAX - 1;
        THEN("no error is thrown") { REQUIRE_NOTHROW(Maths::RateOfChange(size)); }
        auto value = Maths::RateOfChange(size);
        THEN("get_length returns size")      { REQUIRE(size == value.get_length()); }
        THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
    }
    WHEN("the RoC is initialized with length=max") {
        auto size = Maths::RateOfChange<double, double>::LENGTH_MAX;
        THEN("no error is thrown") { REQUIRE_NOTHROW(Maths::RateOfChange(size)); }
        auto value = Maths::RateOfChange(size);
        THEN("get_length returns size")      { REQUIRE(size == value.get_length()); }
        THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
    }
    WHEN("the RoC is initialized with length=max + 1") {
        auto size = Maths::RateOfChange<double, double>::LENGTH_MAX + 1;
        THEN("an error is thrown") { REQUIRE_THROWS(Maths::RateOfChange(size)); }
    }
}

// ---------------------------------------------------------------------------
// MARK: process/get_head/get_tail/get_rate_of_change
// ---------------------------------------------------------------------------

SCENARIO("calculate rate of change with length 2") {
    auto size = 2;
    GIVEN("a series of 0s") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 1 value") {
            value.process(0);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
    }
    GIVEN("a series of 1s") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 1 value") {
            value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
    }
    GIVEN("a monotonically increasing series") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 1 value") {
            value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 2 values") {
            value.process(1); value.process(2);
            THEN("get_rate_of_change returns 1") { REQUIRE(1 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(1); value.process(2); value.process(3);
            THEN("get_rate_of_change returns 0.5") { REQUIRE(0.5 == value.get_rate_of_change()); }
        }
    }
}

SCENARIO("calculate rate of change with length 3") {
    auto size = 3;
    GIVEN("a series of 0s") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 1 value") {
            value.process(0);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 4 values") {
            value.process(0); value.process(0); value.process(0); value.process(0);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
    }
    GIVEN("a series of 1s") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 1 value") {
            value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1); value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
    }
    GIVEN("a monotonically increasing series") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 1 value") {
            value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 2 values") {
            value.process(1); value.process(2);
            THEN("get_rate_of_change returns 1") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(1); value.process(2); value.process(3);
            THEN("get_rate_of_change returns (3-1)/1") { REQUIRE(2 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 4 values") {
            value.process(1); value.process(2); value.process(3); value.process(4);
            THEN("get_rate_of_change returns (4-2)/2") { REQUIRE(1 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 5 values") {
            value.process(1); value.process(2); value.process(3); value.process(4); value.process(5);
            THEN("get_rate_of_change returns (5-3)/3") { REQUIRE(2.f/3.f == Approx(value.get_rate_of_change())); }
        }
    }
}

SCENARIO("calculate rate of change with length 4") {
    auto size = 4;
    GIVEN("a monotonically increasing series") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 1 value") {
            value.process(1);
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 2 values") {
            value.process(1); value.process(2);
            THEN("get_rate_of_change returns 1") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 3 values") {
            value.process(1); value.process(2); value.process(3);
            THEN("get_rate_of_change returns 1") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 4 values") {
            value.process(1); value.process(2); value.process(3); value.process(4);
            THEN("get_rate_of_change returns (4-1)/1") { REQUIRE(3 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 5 values") {
            value.process(1); value.process(2); value.process(3); value.process(4); value.process(5);
            THEN("get_rate_of_change returns (5-2)/2") { REQUIRE(3.f/2.f == Approx(value.get_rate_of_change())); }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: reset
// ---------------------------------------------------------------------------

SCENARIO("RoC is calculated then reset") {
    auto size = 4;
    GIVEN("a monotonically increasing series") {
        auto value = Maths::RateOfChange(size);
        WHEN("the RoC is calculated with 3 values and reset") {
            value.process(1); value.process(2); value.process(3);
            value.reset();
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
            value.process(1); value.process(2); value.process(3);
            THEN("get_rate_of_change returns 1") { REQUIRE(0 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 4 values and reset") {
            value.process(1); value.process(2); value.process(3); value.process(4);
            value.reset();
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
            value.process(1); value.process(2); value.process(3); value.process(4);
            THEN("get_rate_of_change returns (4-1)/1") { REQUIRE(3 == value.get_rate_of_change()); }
        }
        WHEN("the RoC is calculated with 5 values and reset") {
            value.process(1); value.process(2); value.process(3); value.process(4); value.process(5);
            value.reset();
            THEN("get_rate_of_change returns 0") { REQUIRE(0 == value.get_rate_of_change()); }
            value.process(1); value.process(2); value.process(3); value.process(4); value.process(5);
            THEN("get_rate_of_change returns (5-2)/2") { REQUIRE(3.f/2.f == Approx(value.get_rate_of_change())); }
        }
    }
}
