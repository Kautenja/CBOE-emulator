// Test cases for the exponential moving average object.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "maths/exponential_moving_average.hpp"

// ---------------------------------------------------------------------------
// MARK: Constructor
// ---------------------------------------------------------------------------

SCENARIO("initialize an exponential moving average with alpha value") {
    WHEN("the EMA is initialized with alpha=0.0") {
        auto value = Maths::ExponentialMovingAverage(0.f);
        THEN("get_alpha returns 0.0")   { REQUIRE(0.f == value.get_alpha()); }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
    WHEN("the EMA is initialized with alpha=-1.0") {
        auto value = Maths::ExponentialMovingAverage(-1.f);
        THEN("get_alpha returns 0.0")   { REQUIRE(0.f == value.get_alpha()); }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
    WHEN("the EMA is initialized with alpha=1.0") {
        auto value = Maths::ExponentialMovingAverage(1.f);
        THEN("get_alpha returns 1.0")   { REQUIRE(1.f == value.get_alpha()); }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
    WHEN("the EMA is initialized with alpha=2.0") {
        auto value = Maths::ExponentialMovingAverage(2.f);
        THEN("get_alpha returns 1.0")   { REQUIRE(1.f == value.get_alpha()); }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
    WHEN("the EMA is initialized with alpha=0.1") {
        auto value = Maths::ExponentialMovingAverage(0.1f);
        THEN("get_alpha returns 0.1")   { REQUIRE(0.1f == value.get_alpha()); }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
    WHEN("the EMA is initialized with alpha=0.9") {
        auto value = Maths::ExponentialMovingAverage(0.9f);
        THEN("get_alpha returns 0.9")   { REQUIRE(0.9f == value.get_alpha()); }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
}

SCENARIO("initialize an exponential moving average with initial observation") {
    WHEN("the EMA is initialized with initial=0.0") {
        auto value = Maths::ExponentialMovingAverage(0.5f, 0.f);
        THEN("get_alpha returns 0.5")   { REQUIRE(0.5f == value.get_alpha()); }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
    WHEN("the EMA is initialized with initial=-1.0") {
        auto value = Maths::ExponentialMovingAverage(0.5f, -1.f);
        THEN("get_alpha returns 0.5")   { REQUIRE(0.5f == value.get_alpha()); }
        THEN("get_average returns -1.0") { REQUIRE(-1.f == value.get_average()); }
    }
    WHEN("the EMA is initialized with initial=1.0") {
        auto value = Maths::ExponentialMovingAverage(0.5f, 1.f);
        THEN("get_alpha returns 0.5")   { REQUIRE(0.5f == value.get_alpha()); }
        THEN("get_average returns 1.0") { REQUIRE(1.f == value.get_average()); }
    }
}

// ---------------------------------------------------------------------------
// MARK: reset
// ---------------------------------------------------------------------------

SCENARIO("reset an exponential moving average with a new initial observation") {
    WHEN("the EMA is reset with initial=0.0") {
        auto value = Maths::ExponentialMovingAverage(0.5f, 1.f);
        value.reset(0);
        THEN("get_alpha returns 0.5")   { REQUIRE(0.5f == value.get_alpha());  }
        THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
    }
    WHEN("the EMA is reset with initial=-1.0") {
        auto value = Maths::ExponentialMovingAverage(0.5f, 0.f);
        value.reset(-1);
        THEN("get_alpha returns 0.5")    { REQUIRE(0.5f == value.get_alpha());   }
        THEN("get_average returns -1.0") { REQUIRE(-1.f == value.get_average()); }
    }
    WHEN("the EMA is reset with initial=1.0") {
        auto value = Maths::ExponentialMovingAverage(0.5f, 0.f);
        value.reset(1);
        THEN("get_alpha returns 0.5")   { REQUIRE(0.5f == value.get_alpha());  }
        THEN("get_average returns 1.0") { REQUIRE(1.f == value.get_average()); }
    }
}

// ---------------------------------------------------------------------------
// MARK: set_alpha
// ---------------------------------------------------------------------------

SCENARIO("set alpha of an exponential moving average") {
    GIVEN("an exponential moving average") {
        auto value = Maths::ExponentialMovingAverage(0.5f);
        WHEN("the alpha is set to 0.0") {
            value.set_alpha(0.f);
            THEN("get_alpha returns 0.0")   { REQUIRE(0.f == value.get_alpha()); }
            THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the alpha is set to -1.0") {
            value.set_alpha(-1.f);
            THEN("get_alpha returns 0.0")   { REQUIRE(0.f == value.get_alpha()); }
            THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the alpha is set to 1.0") {
            value.set_alpha(1.f);
            THEN("get_alpha returns 1.0")   { REQUIRE(1.f == value.get_alpha()); }
            THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the alpha is set to 2.0") {
            value.set_alpha(2.f);
            THEN("get_alpha returns 1.0") { REQUIRE(1.f == value.get_alpha()); }
            THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the alpha is set to 0.1") {
            value.set_alpha(0.1f);
            THEN("get_alpha returns 0.1")   { REQUIRE(0.1f == value.get_alpha()); }
            THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the alpha is set to 0.9") {
            value.set_alpha(0.9f);
            THEN("get_alpha returns 0.9")   { REQUIRE(0.9f == value.get_alpha()); }
            THEN("get_average returns 0.0") { REQUIRE(0.f == value.get_average()); }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: process
// ---------------------------------------------------------------------------

SCENARIO("calculate exponential moving average of 0") {
    GIVEN("alpha of 0") {
        auto value = Maths::ExponentialMovingAverage(0.f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
    }
    GIVEN("alpha of 1") {
        auto value = Maths::ExponentialMovingAverage(1.f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
    }
    GIVEN("alpha of 0.5") {
        auto value = Maths::ExponentialMovingAverage(0.5f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
    }
}

SCENARIO("calculate exponential moving average of 1") {
    GIVEN("alpha of 0") {
        auto value = Maths::ExponentialMovingAverage(0.f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(1);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
    }
    GIVEN("alpha of 1") {
        auto value = Maths::ExponentialMovingAverage(1.f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(1);
            THEN("the average is 1") { REQUIRE(1.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("the average is 1") { REQUIRE(1.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("the average is 1") { REQUIRE(1.f == value.get_average()); }
        }
    }
    GIVEN("alpha of 0.5") {
        auto value = Maths::ExponentialMovingAverage(0.5f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(1);
            THEN("the average is 0.5") { REQUIRE(0.5f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("the average is 0.75") { REQUIRE(0.75f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("the average is 0.875f") { REQUIRE(0.875f == value.get_average()); }
        }
    }
}

SCENARIO("calculate exponential moving average of -1") {
    GIVEN("alpha of 0") {
        auto value = Maths::ExponentialMovingAverage(0.f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(-1);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(-1); value.process(-1);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(-1); value.process(-1); value.process(-1);
            THEN("the average is 0") { REQUIRE(0.f == value.get_average()); }
        }
    }
    GIVEN("alpha of 1") {
        auto value = Maths::ExponentialMovingAverage(1.f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(-1);
            THEN("the average is -1") { REQUIRE(-1.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(-1); value.process(-1);
            THEN("the average is -1") { REQUIRE(-1.f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(-1); value.process(-1); value.process(-1);
            THEN("the average is -1") { REQUIRE(-1.f == value.get_average()); }
        }
    }
    GIVEN("alpha of 0.5") {
        auto value = Maths::ExponentialMovingAverage(0.5f);
        WHEN("the EMA is calculated with 1 value") {
            value.process(-1);
            THEN("the average is -0.5") { REQUIRE(-0.5f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 2 values") {
            value.process(-1); value.process(-1);
            THEN("the average is -0.75") { REQUIRE(-0.75f == value.get_average()); }
        }
        WHEN("the EMA is calculated with 3 values") {
            value.process(-1); value.process(-1); value.process(-1);
            THEN("the average is -0.875f") { REQUIRE(-0.875f == value.get_average()); }
        }
    }
}
