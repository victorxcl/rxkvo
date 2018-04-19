#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include <string>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

namespace mail{
}

SCENARIO("kvo::variable' [will] and [did] subjects", "")
{
    GIVEN("one integer variable A")
    {
        struct Test
        {
            Test()
            {
                A.subject_will.get_observable().subscribe([this](int x){ this->A_will = x; });
                A.subject_did.get_observable().subscribe([this](int x){ this->A_did = x; });
            }
            kvo::variable<int> A;
            int A_will = -1000;
            int A_did  = -1000;
        };
        
        auto test = std::make_shared<Test>();
        REQUIRE(    0 == test->A);
        REQUIRE(-1000 == test->A_will);
        REQUIRE(    0 == test->A_did);
        
        THEN("modify A to another value")
        {
            test->A = 123;
            REQUIRE(123 == test->A);
            REQUIRE(  0 == test->A_will);
            REQUIRE(123 == test->A_did);
            
            THEN("modify A to another value again")
            {
                test->A = 456;
                REQUIRE(456 == test->A);
                REQUIRE(123 == test->A_will);
                REQUIRE(456 == test->A_did);
                
                THEN("modify A to another value again and again")
                {
                    test->A = 789;
                    REQUIRE(789 == test->A);
                    REQUIRE(456 == test->A_will);
                    REQUIRE(789 == test->A_did);
                }
            }
        }
    }
    
    GIVEN("one std::string variable A")
    {
        struct Test
        {
            Test()
            {
                A.subject_will.get_observable().subscribe([this](std::string x){ this->A_will = x; });
                A.subject_did.get_observable().subscribe([this](std::string x){ this->A_did = x; });
            }
            kvo::variable<std::string> A;
            std::string A_will = "-1000";
            std::string A_did  = "-1000";
        };
        //using namespace std::literals::string_literals;
        auto test = std::make_shared<Test>();
        REQUIRE(     "" == test->A());
        REQUIRE("-1000" == test->A_will);
        REQUIRE(     "" == test->A_did);
        
        THEN("modify A to another value")
        {
            test->A = "123";
            REQUIRE("123" == test->A());
            REQUIRE(   "" == test->A_will);
            REQUIRE("123" == test->A_did);
            
            THEN("modify A to another value again")
            {
                test->A = "456";
                REQUIRE("456" == test->A());
                REQUIRE("123" == test->A_will);
                REQUIRE("456" == test->A_did);
                
                THEN("modify A to another value again and again")
                {
                    test->A = "789";
                    REQUIRE("789" == test->A());
                    REQUIRE("456" == test->A_will);
                    REQUIRE("789" == test->A_did);
                }
            }
        }
    }
}

SCENARIO("kvo::collection' [will] and [did] subjects", "")
{
    GIVEN("")
    {
        THEN("")
        {
        }
    }
}
