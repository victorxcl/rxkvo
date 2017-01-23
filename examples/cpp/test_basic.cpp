#include <rxcpp/rx.hpp>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)


SCENARIO("test kvo_varibale", "")
{
    struct Person
    {
        kvo::variable<std::string> firstName;
        kvo::variable<std::string> lastName;
        kvo::variable<std::string> fullName;
        
        Person()
        {
            firstName.subject.get_observable()
            .combine_latest(lastName.subject.get_observable())
            .subscribe([this](std::tuple<std::string,std::string>x){
                this->fullName = std::get<0>(x) + " " + std::get<1>(x);
            });
        }
        Person(std::string _firstName, std::string _lastName):Person()
        {
            this->firstName = _firstName;
            this->lastName = _lastName;
        }
    };
    GIVEN("a person")
    {
        Person person;
        REQUIRE(person.firstName.get() == "");
        REQUIRE(person.lastName.get() == "");
        REQUIRE(person.fullName.get() == " ");
        WHEN("modify firstName")
        {
            person.firstName = "Hello";
            REQUIRE(person.firstName.get() == "Hello");
            REQUIRE(person.lastName.get() == "");
            REQUIRE(person.fullName.get() == "Hello ");
            THEN("modify lastName")
            {
                person.lastName = "World";
                REQUIRE(person.firstName.get() == "Hello");
                REQUIRE(person.lastName.get() == "World");
                REQUIRE(person.fullName.get() == "Hello World");
            }
        }
        WHEN("modify lastName")
        {
            person.lastName = "World";
            REQUIRE(person.firstName.get() == "");
            REQUIRE(person.lastName.get() == "World");
            REQUIRE(person.fullName.get() == " World");
            THEN("modify firstName")
            {
                person.firstName = "Hello";
                REQUIRE(person.firstName.get() == "Hello");
                REQUIRE(person.lastName.get() == "World");
                REQUIRE(person.fullName.get() == "Hello World");
            }
        }
    }
    GIVEN("a leader")
    {
        struct Leader
        {
			Leader() {}
            kvo::variable<Person> member;
        };
        AND_THEN("given a team with rx key path")
        {
            struct Team
            {
                kvo::variable<Leader> leader;
                kvo::variable<std::string> leader_fullName;
                Team()
                {
                    leader.subject.get_observable()
                    .map([](const Leader&x){ return x.member.subject.get_observable(); }).switch_on_next()
                    .map([](const Person&x){ return x.fullName.subject.get_observable(); }).switch_on_next()
                    .subscribe([this](const std::string&x){ this->leader_fullName = x; });
                }
            }team;
            
            REQUIRE(team.leader_fullName() == " ");
            
            REQUIRE(team.leader().member().firstName() == "");
            REQUIRE(team.leader().member().lastName() == "");
            REQUIRE(team.leader().member().fullName() == " ");
            
            WHEN("modify leader's member")
            {
                team.leader().member = Person("Hello", "World");
                REQUIRE(team.leader().member().firstName() == "Hello");
                REQUIRE(team.leader().member().lastName() == "World");
                REQUIRE(team.leader().member().fullName() == "Hello World");
                REQUIRE(team.leader_fullName() == "Hello World");
                
                AND_WHEN("modify leader's member's firstName")
                {
                    team.leader().member().firstName = "Good";
                    REQUIRE(team.leader().member().firstName() == "Good");
                    REQUIRE(team.leader().member().lastName() == "World");
                    REQUIRE(team.leader().member().fullName() == "Good World");
                    REQUIRE(team.leader_fullName() == "Good World");
                }
                AND_WHEN("modify leader's member's lastName")
                {
                    team.leader().member().lastName = "Luck";
                    REQUIRE(team.leader().member().firstName() == "Hello");
                    REQUIRE(team.leader().member().lastName() == "Luck");
                    REQUIRE(team.leader().member().fullName() == "Hello Luck");
                    REQUIRE(team.leader_fullName() == "Hello Luck");
                }
            }
        }
        AND_THEN("given a team with simplify key path")
        {
            struct Team
            {
                kvo::variable<Leader> leader;
                kvo::variable<std::string> leader_fullName;
                Team()
                {
                    leader([](const Leader&x){ return x.member.subject.get_observable(); },
                           [](const Person&x){ return x.fullName.subject.get_observable(); })
                    .subscribe([this](const std::string&x){
                        this->leader_fullName = x;
                    }, [](std::exception_ptr){});
                }
            }team;
            
            REQUIRE(team.leader_fullName() == " ");
            
            REQUIRE(team.leader().member().firstName() == "");
            REQUIRE(team.leader().member().lastName() == "");
            REQUIRE(team.leader().member().fullName() == " ");
            
            WHEN("modify leader's member")
            {
                team.leader().member = Person("Hello", "World");
                REQUIRE(team.leader().member().firstName() == "Hello");
                REQUIRE(team.leader().member().lastName() == "World");
                REQUIRE(team.leader().member().fullName() == "Hello World");
                REQUIRE(team.leader_fullName() == "Hello World");
                
                AND_WHEN("modify leader's member's firstName")
                {
                    team.leader().member().firstName = "Good";
                    REQUIRE(team.leader().member().firstName() == "Good");
                    REQUIRE(team.leader().member().lastName() == "World");
                    REQUIRE(team.leader().member().fullName() == "Good World");
                    REQUIRE(team.leader_fullName() == "Good World");
                }
                AND_WHEN("modify leader's member's lastName")
                {
                    team.leader().member().lastName = "Luck";
                    REQUIRE(team.leader().member().firstName() == "Hello");
                    REQUIRE(team.leader().member().lastName() == "Luck");
                    REQUIRE(team.leader().member().fullName() == "Hello Luck");
                    REQUIRE(team.leader_fullName() == "Hello Luck");
                }
            }
        }
    }
}

SCENARIO("test basic kvo_collection operations", "")
{
    GIVEN("a kvo_collection with std::vector")
    {
        typedef std::vector<int> collection_t;
        kvo::collection<collection_t> IDs;
        std::size_t ID_count = 0;
        THEN("watch IDs' count")
        {
            typedef decltype(IDs)::rx_notify_value rx_notify_value;
            rxcpp::observable<>::empty<rx_notify_value>()
            .merge(IDs.subject_setting.get_observable(),
                   IDs.subject_insertion.get_observable(),
                   IDs.subject_removal.get_observable(),
                   IDs.subject_replacement.get_observable())
            .subscribe([&ID_count, &IDs](const rx_notify_value&x){
                ID_count = IDs().size();
            });
            REQUIRE(ID_count == 0);
            
            WHEN("set with std::vector container type")
            {
                IDs.set({100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set({100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
                
                THEN("insert {99,88} at {1, 3}")
                {
                    IDs.insert({99, 88}, {1, 3});
                    REQUIRE(IDs() == collection_t({100,99,200,88,300,400}));
                    REQUIRE(ID_count == 6);
                }
                THEN("insert {99,88} at {4, 5}")
                {
                    IDs.insert({99, 88}, {4, 5});
                    REQUIRE(IDs() == collection_t({100,200,300,400,99,88}));
                    REQUIRE(ID_count == 6);
                }
                THEN("insert {99,88,77} at {1, 2, 4}")
                {
                    IDs.insert({99, 88, 77}, {1, 2, 4});
                    REQUIRE(IDs() == collection_t({100,99,88,200,77,300,400}));
                    REQUIRE(ID_count == 7);
                }
                
                THEN("insert {99,88,77} at end")
                {
                    IDs.insert({99, 88, 77});
                    REQUIRE(IDs() == collection_t({100,200,300,400,99,88,77}));
                    REQUIRE(ID_count == 7);
                }
                
                THEN("remove objects at indices {1,3}")
                {
                    IDs.remove({1, 3});
                    REQUIRE(IDs() == collection_t({100,300}));
                    REQUIRE(ID_count == 2);
                }
                
                THEN("replace objects at indices {1,3}")
                {
                    IDs.replace({1, 3}, {99, 88});
                    REQUIRE(IDs() == collection_t({100,99,300,88}));
                    REQUIRE(ID_count == 4);
                }
                
                THEN("remove all items")
                {
                    IDs.set({});
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
                
                THEN("remove all items")
                {
                    IDs.remove_all();
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
            }
            WHEN("set with std::list container type")
            {
                IDs.set(std::list<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::list<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            
            WHEN("set with std::set container type")
            {
                IDs.set(std::set<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::set<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::unordered_set container type")
            {
                IDs.set(std::unordered_set<int>{100,200,300,400,500,600,700,800,900});
                
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 100) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 200) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 300) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 400) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 500) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 600) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 600) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 800) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 900) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 910) == 0);
                REQUIRE(ID_count == 9);
                
                IDs.set(std::unordered_set<int>{100,200,300,400});
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 100) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 200) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 300) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 400) == 1);
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::initializer_list container type")
            {
                IDs.set(std::initializer_list<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::initializer_list<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
        }
    }
    GIVEN("a kvo_collection with std::list")
    {
        typedef std::list<int> collection_t;
        kvo::collection<collection_t> IDs;
        std::size_t ID_count = 0;
        THEN("watch IDs' count")
        {
            typedef decltype(IDs)::rx_notify_value rx_notify_value;
            rxcpp::observable<>::empty<rx_notify_value>()
            .merge(IDs.subject_setting.get_observable(),
                   IDs.subject_insertion.get_observable(),
                   IDs.subject_removal.get_observable(),
                   IDs.subject_replacement.get_observable())
            .subscribe([&ID_count, &IDs](const rx_notify_value&x){
                ID_count = IDs().size();
            });
            REQUIRE(ID_count == 0);
            
            WHEN("set with std::list container type")
            {
                IDs.set({100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set({100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
                
                THEN("insert {99,88} at {1, 3}")
                {
                    IDs.insert({99, 88}, {1, 3});
                    REQUIRE(IDs() == collection_t({100,99,200,88,300,400}));
                    REQUIRE(ID_count == 6);
                }
                THEN("insert {99,88} at {4, 5}")
                {
                    IDs.insert({99, 88}, {4, 5});
                    REQUIRE(IDs() == collection_t({100,200,300,400,99,88}));
                    REQUIRE(ID_count == 6);
                }
                THEN("insert {99,88,77} at {1, 2, 4}")
                {
                    IDs.insert({99, 88, 77}, {1, 2, 4});
                    REQUIRE(IDs() == collection_t({100,99,88,200,77,300,400}));
                    REQUIRE(ID_count == 7);
                }
                
                THEN("insert {99,88,77} at end")
                {
                    IDs.insert({99, 88, 77});
                    REQUIRE(IDs() == collection_t({100,200,300,400,99,88,77}));
                    REQUIRE(ID_count == 7);
                }
                
                THEN("remove objects at indices {1,3}")
                {
                    IDs.remove({1, 3});
                    REQUIRE(IDs() == collection_t({100,300}));
                    REQUIRE(ID_count == 2);
                }
                
                THEN("replace objects at indices {1,3}")
                {
                    IDs.replace({1, 3}, {99, 88});
                    REQUIRE(IDs() == collection_t({100,99,300,88}));
                    REQUIRE(ID_count == 4);
                }
                
                THEN("remove all items")
                {
                    IDs.set({});
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
                
                THEN("remove all items")
                {
                    IDs.remove_all();
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
            }
            
            WHEN("set with std::vector container type")
            {
                IDs.set(std::vector<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::vector<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::set container type")
            {
                IDs.set(std::set<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::set<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::unordered_set container type")
            {
                IDs.set(std::unordered_set<int>{100,200,300,400,500,600,700,800,900});
                
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 100) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 200) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 300) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 400) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 500) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 600) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 600) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 800) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 900) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 910) == 0);
                REQUIRE(ID_count == 9);
                
                IDs.set(std::unordered_set<int>{100,200,300,400});
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 100) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 200) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 300) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 400) == 1);
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::initializer_list container type")
            {
                IDs.set(std::initializer_list<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::initializer_list<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
        }
    }
    GIVEN("a kvo_collection with std::set")
    {
        typedef std::set<int> collection_t;
        kvo::collection<collection_t> IDs;
		std::size_t ID_count = 0;
        THEN("watch IDs' count")
        {
            typedef decltype(IDs)::rx_notify_value rx_notify_value;
            rxcpp::observable<>::empty<rx_notify_value>()
            .merge(IDs.subject_setting.get_observable(),
                   IDs.subject_insertion.get_observable(),
                   IDs.subject_removal.get_observable(),
                   IDs.subject_replacement.get_observable())
            .subscribe([&ID_count, &IDs](const rx_notify_value&x){
                ID_count = IDs().size();
            });
            REQUIRE(ID_count == 0);
            
            WHEN("set with std::set container type")
            {
                IDs.set({100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set({100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
                
                THEN("insert {99,88}")
                {
                    IDs.insert({99, 88});
                    REQUIRE(IDs() == collection_t({100,99,200,88,300,400}));
                    REQUIRE(ID_count == 6);
                }
                
                THEN("remove objects {99, 88, 300}")
                {
                    IDs.remove({99, 88, 300});
                    REQUIRE(IDs() == collection_t({100,200,400}));
                    REQUIRE(ID_count == 3);
                }
                
                THEN("replace objects {200, 400} with {99, 88}")
                {
                    IDs.replace({200, 400}, {99, 88});
                    REQUIRE(IDs() == collection_t({100,99,300,88}));
                    REQUIRE(ID_count == 4);
                }
                
                THEN("remove all items")
                {
                    IDs.set({});
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
                
                THEN("remove all items")
                {
                    IDs.remove_all();
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
            }
            
            WHEN("set with std::vector container type")
            {
                IDs.set(std::vector<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::vector<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::list container type")
            {
                IDs.set(std::list<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::list<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::unordered_set container type")
            {
                IDs.set(std::unordered_set<int>{100,200,300,400,500,600,700,800,900});
                
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 100) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 200) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 300) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 400) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 500) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 600) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 600) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 800) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 900) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 910) == 0);
                REQUIRE(ID_count == 9);
                
                IDs.set(std::unordered_set<int>{100,200,300,400});
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 100) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 200) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 300) == 1);
                REQUIRE(std::count(std::begin(IDs()), std::end(IDs()), 400) == 1);
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::initializer_list container type")
            {
                IDs.set(std::initializer_list<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::initializer_list<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
        }
    }
    GIVEN("a kvo_collection with std::unordered_set")
    {
        typedef std::unordered_set<int> collection_t;
        kvo::collection<collection_t> IDs;
		std::size_t ID_count = 0;
        THEN("watch IDs' count")
        {
            typedef decltype(IDs)::rx_notify_value rx_notify_value;
            rxcpp::observable<>::empty<rx_notify_value>()
            .merge(IDs.subject_setting.get_observable(),
                   IDs.subject_insertion.get_observable(),
                   IDs.subject_removal.get_observable(),
                   IDs.subject_replacement.get_observable())
            .subscribe([&ID_count, &IDs](const rx_notify_value&x){
                ID_count = IDs().size();
            });
            REQUIRE(ID_count == 0);
            
            WHEN("set with std::unordered_set container type")
            {
                IDs.set({100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set({100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
                
                THEN("insert {99,88}")
                {
                    IDs.insert({99, 88});
                    REQUIRE(IDs() == collection_t({100,99,200,88,300,400}));
                    REQUIRE(ID_count == 6);
                }
                
                THEN("remove objects {99, 88, 300}")
                {
                    IDs.remove({99, 88, 300});
                    REQUIRE(IDs() == collection_t({100,200,400}));
                    REQUIRE(ID_count == 3);
                }
                
                THEN("replace objects {200, 400} with {99, 88}")
                {
                    IDs.replace({200, 400}, {99, 88});
                    REQUIRE(IDs() == collection_t({100,99,300,88}));
                    REQUIRE(ID_count == 4);
                }
                
                THEN("remove all items")
                {
                    IDs.set({});
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
                
                THEN("remove all items")
                {
                    IDs.remove_all();
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
            }
            
            WHEN("set with std::vector container type")
            {
                IDs.set(std::vector<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::vector<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::list container type")
            {
                IDs.set(std::list<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::list<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::set container type")
            {
                IDs.set(std::set<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::set<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
            WHEN("set with std::initializer_list container type")
            {
                IDs.set(std::initializer_list<int>{100,200,300,400,500,600,700,800,900});
                REQUIRE(IDs.get() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(IDs() == collection_t({100,200,300,400,500,600,700,800,900}));
                REQUIRE(ID_count == 9);
                
                IDs.set(std::initializer_list<int>{100,200,300,400});
                REQUIRE(IDs.get() == collection_t({100,200,300,400}));
                REQUIRE(IDs() == collection_t({100,200,300,400}));
                REQUIRE(ID_count == 4);
            }
        }
    }
    GIVEN("a kvo_collection with std::map")
    {
        typedef std::map<std::string,int> collection_t;
        kvo::collection<collection_t> IDs;
        std::size_t ID_count = 0;
        THEN("watch IDs' count")
        {
            typedef decltype(IDs)::rx_notify_value rx_notify_value;
            rxcpp::observable<>::empty<rx_notify_value>()
            .merge(IDs.subject_setting.get_observable(),
                   IDs.subject_insertion.get_observable(),
                   IDs.subject_removal.get_observable(),
                   IDs.subject_replacement.get_observable())
            .subscribe([&ID_count, &IDs](const rx_notify_value&x){
                ID_count = IDs().size();
            });
            REQUIRE(ID_count == 0);
            
            WHEN("set a set of data")
            {
                IDs.set({{"a",100},{"b",200},{"c",300},{"d",400},{"e",500},{"f",600},{"g",700},{"h",800},{"i",900}});
                REQUIRE(IDs.get() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400},{"e",500},{"f",600},{"g",700},{"h",800},{"i",900}}));
                REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400},{"e",500},{"f",600},{"g",700},{"h",800},{"i",900}}));
                REQUIRE(ID_count == 9);
                
                IDs.set({{"a",100},{"b",200},{"c",300},{"d",400}});
                REQUIRE(IDs.get() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400}}));
                REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400}}));
                REQUIRE(ID_count == 4);
                
                THEN(u8R"==(insert {"x", 99}, {"y", 88})==")
                {
                    IDs.insert({{"x", 99}, {"y", 88}});
                    REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400},{"x",99},{"y",88}}));
                    REQUIRE(ID_count == 6);
                }
                
                THEN(u8R"==(remove objects for key {"A","B","c"})==")
                {
                    IDs.remove({"A","B","c"});
                    REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"d",400}}));
                    REQUIRE(ID_count == 3);
                }
                
                THEN(u8R"==(replace objects {"b",99}, {"d", 88})==")
                {
                    IDs.replace({{"b",99}, {"d", 88}});
                    REQUIRE(IDs() == collection_t({{"a",100},{"b",99},{"c",300},{"d",88}}));
                    REQUIRE(ID_count == 4);
                }
                
                THEN("remove all items")
                {
                    IDs.set({});
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
                
                THEN("remove all items")
                {
                    IDs.remove_all();
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
            }
        }
    }
    GIVEN("a kvo_collection with std::unordered_map")
    {
        typedef std::unordered_map<std::string,int> collection_t;
        kvo::collection<collection_t> IDs;
        std::size_t ID_count = 0;
        THEN("watch IDs' count")
        {
            typedef decltype(IDs)::rx_notify_value rx_notify_value;
            rxcpp::observable<>::empty<rx_notify_value>()
            .merge(IDs.subject_setting.get_observable(),
                   IDs.subject_insertion.get_observable(),
                   IDs.subject_removal.get_observable(),
                   IDs.subject_replacement.get_observable())
            .subscribe([&ID_count, &IDs](const rx_notify_value&x){
                ID_count = IDs().size();
            });
            REQUIRE(ID_count == 0);
            
            WHEN("set a set of data")
            {
                IDs.set({{"a",100},{"b",200},{"c",300},{"d",400},{"e",500},{"f",600},{"g",700},{"h",800},{"i",900}});
                REQUIRE(IDs.get() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400},{"e",500},{"f",600},{"g",700},{"h",800},{"i",900}}));
                REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400},{"e",500},{"f",600},{"g",700},{"h",800},{"i",900}}));
                REQUIRE(ID_count == 9);
                
                IDs.set({{"a",100},{"b",200},{"c",300},{"d",400}});
                REQUIRE(IDs.get() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400}}));
                REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400}}));
                REQUIRE(ID_count == 4);
                
                THEN(u8R"==(insert {"x", 99}, {"y", 88})==")
                {
                    IDs.insert({{"x", 99}, {"y", 88}});
                    REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"c",300},{"d",400},{"x",99},{"y",88}}));
                    REQUIRE(ID_count == 6);
                }
                
                THEN(u8R"==(remove objects for key {"A","B","c"})==")
                {
                    IDs.remove({"A","B","c"});
                    REQUIRE(IDs() == collection_t({{"a",100},{"b",200},{"d",400}}));
                    REQUIRE(ID_count == 3);
                }
                
                THEN(u8R"==(replace objects {"b",99}, {"d", 88})==")
                {
                    IDs.replace({{"b",99}, {"d", 88}});
                    REQUIRE(IDs() == collection_t({{"a",100},{"b",99},{"c",300},{"d",88}}));
                    REQUIRE(ID_count == 4);
                }
                
                THEN("remove all items")
                {
                    IDs.set({});
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
                
                THEN("remove all items")
                {
                    IDs.remove_all();
                    REQUIRE(IDs() == collection_t({}));
                    REQUIRE(ID_count == 0);
                }
            }
        }
    }
}

SCENARIO("test long key path", "")
{
    GIVEN("a simple key path")
    {
        struct Student
        {
            kvo::variable<std::string> name;
            Student():Student("default name"){ }
            Student(const std::string&name){
                this->name = name;
            }
        };
        struct Class
        {
            kvo::variable<std::shared_ptr<Student>> monitor;
            Class(){
                monitor = std::make_shared<Student>();
            }
        };
        struct School
        {
            kvo::variable<std::shared_ptr<Class>> best;
            School(){
                best = std::make_shared<Class>();
            }
        };
        
        struct Test
        {
            kvo::variable<std::shared_ptr<School>> school;
            kvo::variable<std::string> best_class_monitor_name;
            Test(){
                school = std::make_shared<School>();
                
                this->school.subject.get_observable()
                .map([](std::shared_ptr<School>x){ return x->best.subject.get_observable(); }).switch_on_next()
                .map([](std::shared_ptr<Class>x){ return x->monitor.subject.get_observable(); }).switch_on_next()
                .map([](std::shared_ptr<Student>x){ return x->name.subject.get_observable(); }).switch_on_next()
                .subscribe([this](const std::string&x){
                    this->best_class_monitor_name = x;
                });
            }
        };
        
        auto test = std::make_shared<Test>();
        REQUIRE(test->best_class_monitor_name() == "default name");
        
        THEN("test keypath with operator ()")
        {
            WHEN("modify name")
            {
                test->school()->best()->monitor()->name = "Hello";
                REQUIRE(test->best_class_monitor_name() == "Hello");
                REQUIRE(*test->best_class_monitor_name == "Hello");
            }
            WHEN("modify student")
            {
                auto student = std::make_shared<Student>("World");
                student->name = "World";
                test->school()->best()->monitor = student;
                REQUIRE(test->best_class_monitor_name() == "World");
                REQUIRE(*test->best_class_monitor_name == "World");
            }
            WHEN("modify best class")
            {
                auto best = std::make_shared<Class>();
                {
                    auto student = std::make_shared<Student>("World");
                    student->name = "World";
                    best->monitor = student;
                }
                test->school()->best = best;
                REQUIRE(test->best_class_monitor_name() == "World");
                REQUIRE(*test->best_class_monitor_name == "World");
            }
            WHEN("modify school")
            {
                auto school = std::make_shared<School>();
                {
                    auto best = std::make_shared<Class>();
                    auto student = std::make_shared<Student>("World");
                    student->name = "World";
                    best->monitor = student;
                    school->best = best;
                }
                test->school = school;
                REQUIRE(test->best_class_monitor_name() == "World");
                REQUIRE(*test->best_class_monitor_name == "World");
            }
        }
        THEN("test keypath with operator ->")
        {
            WHEN("modify name")
            {
                test->school->best->monitor->name = "Hello";
                REQUIRE(test->best_class_monitor_name() == "Hello");
                REQUIRE(*test->best_class_monitor_name == "Hello");
            }
            WHEN("modify student")
            {
                auto student = std::make_shared<Student>("World");
                student->name = "World";
                test->school->best->monitor = student;
                REQUIRE(test->best_class_monitor_name() == "World");
                REQUIRE(*test->best_class_monitor_name == "World");
            }
            WHEN("modify best class")
            {
                auto best = std::make_shared<Class>();
                {
                    auto student = std::make_shared<Student>("World");
                    student->name = "World";
                    best->monitor = student;
                }
                test->school->best = best;
                REQUIRE(test->best_class_monitor_name() == "World");
                REQUIRE(*test->best_class_monitor_name == "World");
            }
            WHEN("modify school")
            {
                auto school = std::make_shared<School>();
                {
                    auto best = std::make_shared<Class>();
                    auto student = std::make_shared<Student>("World");
                    student->name = "World";
                    best->monitor = student;
                    school->best = best;
                }
                test->school = school;
                REQUIRE(test->best_class_monitor_name() == "World");
                REQUIRE(*test->best_class_monitor_name == "World");
            }
        }
    }
}
