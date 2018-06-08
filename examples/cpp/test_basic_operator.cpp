#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

SCENARIO("test kvo::variable<T> operator", "")
{
    {
        struct A{};
        struct B{};
        struct C{};
        kvo::__utils__::inherit<A,B,C,void> x;
        int a = 0;
        a++;
    }

    
    GIVEN("a integral variable named n")
    {
        kvo::variable<int> n{1};
        
        REQUIRE(1 == n);
        
//        ++n; REQUIRE(2 == n);
//
//        n++; REQUIRE(3 == n);
//
//        --n; REQUIRE(2 == n);
//
//        n--; REQUIRE(1 == n);
//
//        n+=1; REQUIRE(2 == n);
//
//        n-=1; REQUIRE(1 == n);
    }
    GIVEN("a string variable named s")
    {
        using namespace std::literals::string_literals;
        std::string str;
        str += "1";
        kvo::variable<std::string> s{"1"};
        
        REQUIRE("1"s == s());
    }
}

