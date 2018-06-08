#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

#define SHOULE_DUMP_XML    1

SCENARIO("the first plan case for rxkvo", "")
{
    GIVEN("three numbers: a, b, c")
    {
        struct Keynote
        {
            kvo::variable<int> line{0};
            kvo::variable<int> time{0};
            kvo::variable<std::string> statement;
            
            Keynote()
            {
#if SHOULE_DUMP_XML
                using namespace std::literals::string_literals;
                statement.subject.get_observable()
                .start_with("<plan expression=\"c=a+b\">"s)
                .finally([](){ std::cout<<"</plan>"<<std::endl; })
                .subscribe([](auto x){ std::cout<<x<<std::endl; });
                
                line.subject.get_observable().subscribe([this](auto x){ statement = "line = " + std::to_string(x); });
                time.subject.get_observable().subscribe([this](auto x){ statement = "time = " + std::to_string(x); });
#endif
            }
            ~Keynote() { statement.subject.get_subscriber().on_completed(); }
        }keynote;
        
        
        kvo::variable<int> a{1};
        kvo::variable<int> b{2};
        kvo::variable<int> c{0};

        REQUIRE(1 == a);
        REQUIRE(2 == b);
        REQUIRE(0 == c);
        
        a.subject.get_observable().subscribe([&keynote](auto x){ keynote.statement = "a = " + std::to_string(x); });
        b.subject.get_observable().subscribe([&keynote](auto x){ keynote.statement = "b = " + std::to_string(x); });
        c.subject.get_observable().subscribe([&keynote](auto x){ keynote.statement = "c = " + std::to_string(x); });
        
        THEN("build expression for c = a + b")
        {
            a.subject.get_observable()
            .combine_latest(b.subject.get_observable())
            .subscribe([&c](auto&&x){
                c = std::get<0>(x) + std::get<1>(x);
            });
            
            REQUIRE(1 == a);
            REQUIRE(2 == b);
            REQUIRE(3 == c);
            
            float x = 1.0;
            x++;
            x--;
            
#pragma push(_)
#define _(n) (keynote.line = n, keynote.time=keynote.time+1)
            THEN("build the plan for keynote")
            {
                _(1); a = 0;
                _(2); b = 0;
                for (int i=0; _(3),i<5; i++)
                { _(4); a = a + 1; }
                for (int i=0; _(5),i<5; i++)
                { _(6); b = b + 1; }
                _(7);
            }
#pragma pop(_)
        }
    }
}

