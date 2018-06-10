#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
//#include <string>
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
                .start_with("<keynote expression=\"c=a+b\">"s)
                .finally([](){ std::cout<<"</keynote>"<<std::endl; })
                .filter([](auto x){ return !x.empty(); })
                .subscribe([](auto x){ std::cout<<x<<std::endl; });
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
        
        using namespace rxcpp::operators;
        using namespace std::literals::string_literals;
        
        (keynote.line.subject.get_observable()|with_latest_from([](auto x, auto time){ return std::make_tuple(std::to_string(time),std::to_string(x)); }, keynote.time.subject.get_observable()))
        .subscribe(rxcpp::util::apply_to([&keynote](auto time, auto x){ keynote.statement = "<line time=\""+time+"\" value=\""+x+"\"/>"s; }));
        
        (a.subject.get_observable()|with_latest_from([](auto x, auto time){ return std::make_tuple(std::to_string(time),std::to_string(x)); }, keynote.time.subject.get_observable()))
        .subscribe(rxcpp::util::apply_to([&keynote](auto time, auto x){ keynote.statement = "<a time=\""+time+"\" value=\""+x+"\"/>"s; }));
        
        (a.subject.get_observable()|with_latest_from([](auto x, auto time){ return std::make_tuple(std::to_string(time),std::to_string(x)); }, keynote.time.subject.get_observable()))
        .subscribe(rxcpp::util::apply_to([&keynote](auto time, auto x){ keynote.statement = "<b time=\""+time+"\" value=\""+x+"\"/>"s; }));
        
        (c.subject.get_observable()|with_latest_from([](auto x, auto time){ return std::make_tuple(std::to_string(time),std::to_string(x)); }, keynote.time.subject.get_observable()))
        .subscribe(rxcpp::util::apply_to([&keynote](auto time, auto x){ keynote.statement = "<c time=\""+time+"\" value=\""+x+"\"/>"s; }));
        
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
            
#pragma push(_)
#define _(n) (keynote.line = n, keynote.time++)
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

