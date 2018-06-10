#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

struct Test
{
    static std::false_type f(...);
    template<typename T> static typename std::true_type f(T&&x){ ++x; return std::true_type();}
};

SCENARIO("test kvo::__utils__::inherit")
{
    GIVEN("three structs: A, B and C")
    {
        struct A{int a{1};};
        struct B{int b{2};};
        struct C{int c{3};};
        THEN("test inherit<A,B,C>")
        {
            kvo::__utils__::inherit<A,B,C> x;
            REQUIRE(x.a == 1);
            REQUIRE(x.b == 2);
            REQUIRE(x.c == 3);
        }
    }
}

SCENARIO("test kvo::variable<T> operator", "")
{
    GIVEN("a integral variable named n")
    {
        kvo::variable<int> n{5};
        
        REQUIRE(5 == n);
        
        THEN("++n")
        {
            auto&m = ++n;
            REQUIRE(6 == n);
            REQUIRE(m.get() == n.get());
        }
        
        THEN("n++")
        {
//            AND_THEN("rvalue for m")
//            {
//                auto&&m = n++;
//                REQUIRE(6 == n);
//                REQUIRE(m.get() == n.get()-1);
//            }
//            THEN("assign to m")
//            {
//                auto m = n++;
//                REQUIRE(6 == n);
//                REQUIRE(m.get() == n.get()-1);
//            }
        }

        THEN("--n")
        {
            auto&m = --n;
            REQUIRE(4 == n);
            REQUIRE(m.get() == n.get());
        }
        
        THEN("n--")
        {
//            AND_THEN("rvalue for m")
//            {
//                auto&&m = n--;
//                REQUIRE(4 == n);
//                REQUIRE(m.get() == n.get()+1);
//            }
//            THEN("assign to m")
//            {
//                auto m = n--;
//                REQUIRE(4 == n);
//                REQUIRE(m.get() == n.get()+1);
//            }
        }
        
        THEN("n+=2")
        {
            auto&&m = n+=2;
            REQUIRE(7 == n);
            REQUIRE(m.get() == n.get());
        }
        
        THEN("n-=2")
        {
            auto&&m = n-=2;
            REQUIRE(3 == n);
            REQUIRE(m.get() == n.get());
        }
    }
    GIVEN("a string variable named s")
    {
        using namespace std::literals::string_literals;
        kvo::variable<std::string> s{"hello"};
        
        REQUIRE("hello"s == s());
        
        THEN("s+=\"world\"")
        {
            auto&&m = s+="world";
            REQUIRE("helloworld"s == s());
            REQUIRE(m.get() == s.get());
        }
    }
}

