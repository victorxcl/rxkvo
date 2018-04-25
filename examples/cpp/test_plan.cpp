#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

#define SHOULE_DUMP_XML    0

SCENARIO("the first plan case for rxkvo", "")
{
    GIVEN("three numbers: a, b, c")
    {
        struct Plan
        {
            std::atomic<int> t{0};
            kvo::variable<int> a{1};
            kvo::variable<int> b{2};
            kvo::variable<int> c{0};
            kvo::variable<std::string> statement;
            
            Plan()
            {
#if SHOULE_DUMP_XML
                using namespace std::literals::string_literals;
                statement.subject.get_observable()
                .start_with("<plan expression=\"c=a+b\">"s)
                .finally([](){ std::cout<<"</plan>"<<std::endl; })
                .subscribe([](auto x){ std::cout<<x<<std::endl; });
                
                a.subject.get_observable().subscribe([this](auto x){
                    statement="<report variable=\"a\" t=\""+std::to_string(t)+"\">"+std::to_string(x)+"</report>";
                });
                b.subject.get_observable().subscribe([this](auto x){
                    statement="<report variable=\"b\" t=\""+std::to_string(t)+"\">"+std::to_string(x)+"</report>";
                });
                c.subject.get_observable().subscribe([this](auto x){
                    statement="<report variable=\"c\" t=\""+std::to_string(t)+"\">"+std::to_string(x)+"</report>";
                });
#endif
            }
            ~Plan() { statement.subject.get_subscriber().on_completed(); }
            
            void create_expression()
            {
                t++;
                auto expr = u8R"===(<plus><variable name="a"/><variable name="b"/></plus>)===";
                statement = "<assign variable=\"c\" t=\""+ std::to_string(t) + "\">" + expr + "</assign>";
                a.subject.get_observable()
                .combine_latest(b.subject.get_observable())
                .subscribe([this](auto&&x){
                    c = std::get<0>(x) + std::get<1>(x);
                });
            }
            
            void assign_a_with(int v)
            {
                t++;
                statement = "<assign variable=\"a\" t=\""+ std::to_string(t) + "\">" + std::to_string(v) + "</assign>";
                a = v;
            }
            void assign_b_with(int v)
            {
                t++;
                statement = "<assign variable=\"b\" t=\""+ std::to_string(t) + "\">" + std::to_string(v) + "</assign>";
                b = v;
            }
        }plan;

        REQUIRE(1 == plan.a);
        REQUIRE(2 == plan.b);
        REQUIRE(0 == plan.c);
        
        THEN("c subscribe to a+b")
        {
            plan.create_expression();
            REQUIRE(1+2 == plan.c);
            THEN("assign 10 to a")
            {
                plan.assign_a_with(10);
                REQUIRE(10+2 == plan.c);
                AND_THEN("assign 20 to b")
                {
                    plan.assign_b_with(20);
                    REQUIRE(10+20 == plan.c);
                }
            }
        }
    }
}

