// A doubly linked list data structure.
// Copyright 2019 Christian Kauten
//

#include <sstream>
#include <string>
#include <vector>
#include "catch.hpp"
#include "csv.hpp"

//
// MARK: CSV::_next_line
//

SCENARIO("a row is read") {
    GIVEN("a string with a CSV row (1 item)") {
        auto row = "";
        WHEN("the row is read") {
            THEN("a vector with the cells are returned") {
                auto stream = std::stringstream(row);
                auto data = CSV::_next_line(stream);
                REQUIRE(1 == data.size());
                REQUIRE_THAT("", Catch::Equals(data[0]));
            }
        }
    }
    GIVEN("a string with a CSV row (4 items)") {
        auto row = "a,b,c,d";
        WHEN("the row is read") {
            THEN("a vector with the cells are returned") {
                auto stream = std::stringstream(row);
                auto data = CSV::_next_line(stream);
                REQUIRE(4 == data.size());
                REQUIRE_THAT("a", Catch::Equals(data[0]));
                REQUIRE_THAT("b", Catch::Equals(data[1]));
                REQUIRE_THAT("c", Catch::Equals(data[2]));
                REQUIRE_THAT("d", Catch::Equals(data[3]));
            }
        }
    }
    GIVEN("a string with a CSV row (4 items + trailing)") {
        auto row = "a,b,c,d,";
        WHEN("the row is read") {
            THEN("a vector with the cells are returned") {
                auto stream = std::stringstream(row);
                auto data = CSV::_next_line(stream);
                REQUIRE(5 == data.size());
                REQUIRE_THAT("a", Catch::Equals(data[0]));
                REQUIRE_THAT("b", Catch::Equals(data[1]));
                REQUIRE_THAT("c", Catch::Equals(data[2]));
                REQUIRE_THAT("d", Catch::Equals(data[3]));
                REQUIRE_THAT("", Catch::Equals(data[4]));
            }
        }
    }
}

//
// MARK: CSV::load
//

SCENARIO("a CSV is loaded") {
    GIVEN("a string with a CSV") {
        auto csv_string = "a,b,c,d\ne,f,g,h\ni,j,k,l\n";
        WHEN("the CSV is read") {
            auto stream = std::stringstream(csv_string);
            auto csv = CSV::load(stream);
            THEN("the returned data is correct") {
                REQUIRE(3 == csv.size());

                REQUIRE(4 == csv[0].size());
                REQUIRE_THAT("a", Catch::Equals(csv[0][0]));
                REQUIRE_THAT("b", Catch::Equals(csv[0][1]));
                REQUIRE_THAT("c", Catch::Equals(csv[0][2]));
                REQUIRE_THAT("d", Catch::Equals(csv[0][3]));

                REQUIRE(4 == csv[1].size());
                REQUIRE_THAT("e", Catch::Equals(csv[1][0]));
                REQUIRE_THAT("f", Catch::Equals(csv[1][1]));
                REQUIRE_THAT("g", Catch::Equals(csv[1][2]));
                REQUIRE_THAT("h", Catch::Equals(csv[1][3]));

                REQUIRE(4 == csv[2].size());
                REQUIRE_THAT("i", Catch::Equals(csv[2][0]));
                REQUIRE_THAT("j", Catch::Equals(csv[2][1]));
                REQUIRE_THAT("k", Catch::Equals(csv[2][2]));
                REQUIRE_THAT("l", Catch::Equals(csv[2][3]));
            }
        }
    }
}

//
// MARK: CSV::print
//

SCENARIO("a CSV is printed") {
    GIVEN("a CSV") {
        auto expected = "a,b,c,d\ne,f,g,h\ni,j,k,l\n";
        std::vector<std::vector<std::string>> csv = {
            {"a", "b", "c", "d"},
            {"e", "f", "g", "h"},
            {"i", "j", "k", "l"}
        };
        WHEN("the CSV is printed to a string stream") {
            std::stringstream stream;
            CSV::print(csv, stream);
            THEN("then output string is correct") {
                REQUIRE_THAT(stream.str(), Catch::Equals(expected));
            }
        }
    }
}
