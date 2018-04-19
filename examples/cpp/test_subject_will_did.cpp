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

SCENARIO("kvo::collection<std::vector<T>> test [will] and [did] subjects", "")
{
    GIVEN("one std::vector<int> container")
    {
        typedef std::vector<int> T_container;
        typedef std::vector<T_container::difference_type> T_indices;
        struct Test
        {
            Test()
            {
                C.subject_setting_will.get_observable().subscribe([this](auto&&x){ this->C_setting_will = x; });
                C.subject_setting_did.get_observable().subscribe([this](auto&&x){ this->C_setting_did = x; });
                
                C.subject_insertion_index.get_observable().subscribe([this](auto&&x){ this->C_insertion_index = x; });
                C.subject_insertion_will.get_observable().subscribe([this](auto&&x){ this->C_insertion_will = x; });
                C.subject_insertion_did.get_observable().subscribe([this](auto&&x){ this->C_insertion_did = x; });
                
                C.subject_replacement_index.get_observable().subscribe([this](auto&&x){ this->C_replacement_index = x; });
                C.subject_replacement_will.get_observable().subscribe([this](auto&&x){ this->C_replacement_will = x; });
                C.subject_replacement_did.get_observable().subscribe([this](auto&&x){ this->C_replacement_did = x; });
                
                C.subject_removal_index.get_observable().subscribe([this](auto&&x){ this->C_removal_index = x; });
                C.subject_removal_will.get_observable().subscribe([this](auto&&x){ this->C_removal_will = x; });
                C.subject_removal_did.get_observable().subscribe([this](auto&&x){ this->C_removal_did = x; });
            }
            kvo::collection<T_container> C;
            T_container C_setting_will {};
            T_container C_setting_did  {};
            
            T_indices   C_insertion_index {};
            T_container C_insertion_will  {};
            T_container C_insertion_did   {};
            
            T_indices   C_replacement_index {};
            T_container C_replacement_will  {};
            T_container C_replacement_did   {};
            
            T_indices   C_removal_index {};
            T_container C_removal_will  {};
            T_container C_removal_did   {};
        };
        
        auto test = std::make_shared<Test>();
        REQUIRE(T_container{} == test->C());
        REQUIRE(T_container{} == test->C_setting_will);
        REQUIRE(T_container{} == test->C_setting_did);

        THEN("setting {1,2,3} to the container")
        {
            test->C.set({1,2,3});
            REQUIRE(T_container{1,2,3} == test->C());
            REQUIRE(T_container{     } == test->C_setting_will);
            REQUIRE(T_container{1,2,3} == test->C_setting_did);
            
            THEN("setting {4,5,6} to the container")
            {
                test->C.set({4,5,6});
                REQUIRE(T_container{4,5,6} == test->C());
                REQUIRE(T_container{1,2,3} == test->C_setting_will);
                REQUIRE(T_container{4,5,6} == test->C_setting_did);
                
                THEN("setting {7,8,9} to the container")
                {
                    test->C.set({7,8,9});
                    REQUIRE(T_container{7,8,9} == test->C());
                    REQUIRE(T_container{4,5,6} == test->C_setting_will);
                    REQUIRE(T_container{7,8,9} == test->C_setting_did);
                }
            }
            THEN("append {7,8,9} to the container")
            {
                test->C.insert({7,8,9});
                REQUIRE(T_container{1,2,3,7,8,9} == test->C());
                REQUIRE(T_indices  {3,4,5} == test->C_insertion_index);
                REQUIRE(T_container{7,8,9} == test->C_insertion_will);
                REQUIRE(T_container{7,8,9} == test->C_insertion_did);
                
                THEN("insert {4,5,6} at index {4-1,5-1,6-1} to the container")
                {
                    test->C.insert({4,5,6}, {4-1, 5-1, 6-1});
                    REQUIRE(T_container{1,2,3,4,5,6,7,8,9} == test->C());
                    REQUIRE(T_indices  {3,4,5} == test->C_insertion_index);
                    REQUIRE(T_container{4,5,6} == test->C_insertion_will);
                    REQUIRE(T_container{4,5,6} == test->C_insertion_did);
                    
                    THEN("replace {4,5,6} at index {3,4,5} with {40,50,60} in the container")
                    {
                        test->C.replace({3, 4, 5}, {40,50,60});
                        REQUIRE(T_container{1,2,3,40,50,60,7,8,9} == test->C());
                        REQUIRE(T_indices  { 3, 4, 5} == test->C_replacement_index);
                        REQUIRE(T_container{ 4, 5, 6} == test->C_replacement_will);
                        REQUIRE(T_container{40,50,60} == test->C_replacement_did);
                    }
                    
                    THEN("remove {4,5,6} at index {3,4,5} in the container")
                    {
                        test->C.remove({3, 4, 5});
                        REQUIRE(T_container{1,2,3,7,8,9} == test->C());
                        REQUIRE(T_indices  { 3, 4, 5} == test->C_removal_index);
                        REQUIRE(T_container{ 4, 5, 6} == test->C_removal_will);
                        REQUIRE(T_container{ 4, 5, 6} == test->C_removal_did);
                    }
                }
            }
        }
    }
}
