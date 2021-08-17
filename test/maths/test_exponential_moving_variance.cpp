// Test cases for the exponential moving variance object.
// Copyright 2019 Christian Kauten
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "maths/exponential_moving_variance.hpp"

// ---------------------------------------------------------------------------
// MARK: Constructor
// ---------------------------------------------------------------------------

SCENARIO("initialize an exponential moving variance with alpha value") {
    WHEN("the EMV is initialized with alpha=0.0") {
        auto value = Maths::ExponentialMovingVariance(0.f);
        THEN("get_alpha returns 0.0")    { REQUIRE(0.f == value.get_alpha());    }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMV is initialized with alpha=-1.0") {
        auto value = Maths::ExponentialMovingVariance(-1.f);
        THEN("get_alpha returns 0.0")    { REQUIRE(0.f == value.get_alpha());    }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMV is initialized with alpha=1.0") {
        auto value = Maths::ExponentialMovingVariance(1.f);
        THEN("get_alpha returns 1.0")    { REQUIRE(1.f == value.get_alpha());    }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMV is initialized with alpha=2.0") {
        auto value = Maths::ExponentialMovingVariance(2.f);
        THEN("get_alpha returns 1.0")    { REQUIRE(1.f == value.get_alpha());    }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMV is initialized with alpha=0.1") {
        auto value = Maths::ExponentialMovingVariance(0.1f);
        THEN("get_alpha returns 0.1")    { REQUIRE(0.1f == value.get_alpha());   }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMV is initialized with alpha=0.9") {
        auto value = Maths::ExponentialMovingVariance(0.9f);
        THEN("get_alpha returns 0.9")    { REQUIRE(0.9f == value.get_alpha());   }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
}

SCENARIO("initialize an exponential moving variance with initial observation") {
    WHEN("the EMV is initialized with initial=0.0") {
        auto value = Maths::ExponentialMovingVariance(0.5f, 0.f);
        THEN("get_alpha returns 0.5")    { REQUIRE(0.5f == value.get_alpha());   }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMV is initialized with initial=-1.0") {
        auto value = Maths::ExponentialMovingVariance(0.5f, -1.f);
        THEN("get_alpha returns 0.5")    { REQUIRE(0.5f == value.get_alpha());   }
        THEN("get_average returns -1.0")  { REQUIRE(-1.f == value.get_average()); }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMV is initialized with initial=1.0") {
        auto value = Maths::ExponentialMovingVariance(0.5f, 1.f);
        THEN("get_alpha returns 0.5")    { REQUIRE(0.5f == value.get_alpha());   }
        THEN("get_average returns 1.0")  { REQUIRE(1.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
}

// ---------------------------------------------------------------------------
// MARK: reset
// ---------------------------------------------------------------------------

SCENARIO("reset an exponential moving average with a new initial observation") {
    WHEN("the EMA is reset with initial=0.0") {
        auto value = Maths::ExponentialMovingVariance(0.5f, 1.f);
        value.reset(0);
        THEN("get_alpha returns 0.5")    { REQUIRE(0.5f == value.get_alpha());   }
        THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMA is reset with initial=-1.0") {
        auto value = Maths::ExponentialMovingVariance(0.5f, 0.f);
        value.reset(-1);
        THEN("get_alpha returns 0.5")    { REQUIRE(0.5f == value.get_alpha());   }
        THEN("get_average returns -1.0") { REQUIRE(-1.f == value.get_average()); }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
    WHEN("the EMA is reset with initial=1.0") {
        auto value = Maths::ExponentialMovingVariance(0.5f, 0.f);
        value.reset(1);
        THEN("get_alpha returns 0.5")    { REQUIRE(0.5f == value.get_alpha());   }
        THEN("get_average returns 1.0")  { REQUIRE(1.f == value.get_average());  }
        THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
        THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
    }
}

// ---------------------------------------------------------------------------
// MARK: set_alpha
// ---------------------------------------------------------------------------

SCENARIO("set alpha of an exponential moving variance") {
    GIVEN("an exponential moving variance") {
        auto value = Maths::ExponentialMovingVariance(0.5f);
        WHEN("the alpha is set to 0.0") {
            value.set_alpha(0.f);
            THEN("get_alpha returns 0.0")    { REQUIRE(0.f == value.get_alpha());    }
            THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
            THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
            THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the alpha is set to -1.0") {
            value.set_alpha(-1.f);
            THEN("get_alpha returns 0.0")    { REQUIRE(0.f == value.get_alpha());    }
            THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
            THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
            THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the alpha is set to 1.0") {
            value.set_alpha(1.f);
            THEN("get_alpha returns 1.0")    { REQUIRE(1.f == value.get_alpha());    }
            THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
            THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
            THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the alpha is set to 2.0") {
            value.set_alpha(2.f);
            THEN("get_alpha returns 1.0")    { REQUIRE(1.f == value.get_alpha());    }
            THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
            THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
            THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the alpha is set to 0.1") {
            value.set_alpha(0.1f);
            THEN("get_alpha returns 0.1")    { REQUIRE(0.1f == value.get_alpha());   }
            THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
            THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
            THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the alpha is set to 0.9") {
            value.set_alpha(0.9f);
            THEN("get_alpha returns 0.9")    { REQUIRE(0.9f == value.get_alpha());   }
            THEN("get_average returns 0.0")  { REQUIRE(0.f == value.get_average());  }
            THEN("get_variance returns 0.0") { REQUIRE(0.f == value.get_variance()); }
            THEN("get_stddev returns 0.0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
}

// ---------------------------------------------------------------------------
// MARK: process
// ---------------------------------------------------------------------------

SCENARIO("calculate exponential moving variance of 0") {
    GIVEN("alpha of 0") {
        auto value = Maths::ExponentialMovingVariance(0.f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
    GIVEN("alpha of 1") {
        auto value = Maths::ExponentialMovingVariance(1.f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
    GIVEN("alpha of 0.5") {
        auto value = Maths::ExponentialMovingVariance(0.5f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(0); value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(0); value.process(0); value.process(0);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
}

SCENARIO("calculate exponential moving variance of 1") {
    GIVEN("alpha of 0") {
        auto value = Maths::ExponentialMovingVariance(0.f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(1);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
    GIVEN("alpha of 1") {
        auto value = Maths::ExponentialMovingVariance(1.f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(1);
            THEN("the average is 1")  { REQUIRE(1.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("the average is 1")  { REQUIRE(1.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("the average is 1")  { REQUIRE(1.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
    GIVEN("alpha of 0.5") {
        auto value = Maths::ExponentialMovingVariance(0.5f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(1);
            THEN("the average is 0.5")       { REQUIRE(0.5f == value.get_average());   }
            THEN("the variance is 0.25")     { REQUIRE(0.25f == value.get_variance()); }
            THEN("the stddev is sqrt(0.25)") { REQUIRE(sqrt(0.25f) == Approx(value.get_stddev())); }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(1); value.process(1);
            THEN("the average is 0.75")        { REQUIRE(0.75f == value.get_average());    }
            THEN("the variance is 0.1875")     { REQUIRE(0.1875f == value.get_variance()); }
            THEN("the stddev is sqrt(0.1875)") { REQUIRE(sqrt(0.1875f) == Approx(value.get_stddev())); }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(1); value.process(1); value.process(1);
            THEN("the average is 0.875f")        { REQUIRE(0.875f == value.get_average());     }
            THEN("the variance is 0.109375")     { REQUIRE(0.109375f == value.get_variance()); }
            THEN("the stddev is sqrt(0.109375)") { REQUIRE(sqrt(0.109375f) == Approx(value.get_stddev())); }
        }
    }
}

SCENARIO("calculate exponential moving variance of -1") {
    GIVEN("alpha of 0") {
        auto value = Maths::ExponentialMovingVariance(0.f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(-1);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(-1); value.process(-1);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(-1); value.process(-1); value.process(-1);
            THEN("the average is 0")  { REQUIRE(0.f == value.get_average());  }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
    GIVEN("alpha of 1") {
        auto value = Maths::ExponentialMovingVariance(1.f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(-1);
            THEN("the average is -1") { REQUIRE(-1.f == value.get_average()); }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(-1); value.process(-1);
            THEN("the average is -1") { REQUIRE(-1.f == value.get_average()); }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(-1); value.process(-1); value.process(-1);
            THEN("the average is -1") { REQUIRE(-1.f == value.get_average()); }
            THEN("the variance is 0") { REQUIRE(0.f == value.get_variance()); }
            THEN("the stddev is 0")   { REQUIRE(0.f == value.get_stddev());   }
        }
    }
    GIVEN("alpha of 0.5") {
        auto value = Maths::ExponentialMovingVariance(0.5f);
        WHEN("the EMV is calculated with 1 value") {
            value.process(-1);
            THEN("the average is -0.5")      { REQUIRE(-0.5f == value.get_average());  }
            THEN("the variance is 0.25")     { REQUIRE(0.25f == value.get_variance()); }
            THEN("the stddev is sqrt(0.25)") { REQUIRE(sqrt(0.25f) == Approx(value.get_stddev())); }
        }
        WHEN("the EMV is calculated with 2 values") {
            value.process(-1); value.process(-1);
            THEN("the average is -0.75")       { REQUIRE(-0.75f == value.get_average());   }
            THEN("the variance is 0.1875")     { REQUIRE(0.1875f == value.get_variance()); }
            THEN("the stddev is sqrt(0.1875)") { REQUIRE(sqrt(0.1875f) == Approx(value.get_stddev())); }
        }
        WHEN("the EMV is calculated with 3 values") {
            value.process(-1); value.process(-1); value.process(-1);
            THEN("the average is -0.875f")       { REQUIRE(-0.875f == value.get_average());    }
            THEN("the variance is 0.109375")     { REQUIRE(0.109375f == value.get_variance()); }
            THEN("the stddev is sqrt(0.109375)") { REQUIRE(sqrt(0.109375f) == Approx(value.get_stddev())); }
        }
    }
}
