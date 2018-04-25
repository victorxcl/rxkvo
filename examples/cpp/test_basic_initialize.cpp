#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

namespace mail{
}

SCENARIO("test kvo::variable<T> initialize", "")
{
    GIVEN("kvo::variable<int> initialized by a integer")
    {
        kvo::variable<int> a{1};
        kvo::variable<int> b{2};
        kvo::variable<int> c{3};
        
        REQUIRE(1 == a);
        REQUIRE(2 == b);
        REQUIRE(3 == c);
        
        REQUIRE(1 == a.get());
        REQUIRE(2 == b.get());
        REQUIRE(3 == c.get());
        
        REQUIRE(1 == a());
        REQUIRE(2 == b());
        REQUIRE(3 == c());
    }
    GIVEN("kvo::variable<std::string> initialized by a string")
    {
        kvo::variable<std::string> a{"1"};
        kvo::variable<std::string> b{"2"};
        kvo::variable<std::string> c{"3"};
        
        //REQUIRE("1" == a);
        //REQUIRE("2" == b);
        //REQUIRE("3" == c);
        
        REQUIRE("1" == a.get());
        REQUIRE("2" == b.get());
        REQUIRE("3" == c.get());
        
        REQUIRE("1" == a());
        REQUIRE("2" == b());
        REQUIRE("3" == c());
    }
}

