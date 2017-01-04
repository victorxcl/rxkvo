#include <rxcpp/rx.hpp>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "kvo_extension.hpp"

SCENARIO("test kvo_varibale", "")
{
    struct Person
    {
        kvo_variable<std::string> firstName;
        kvo_variable<std::string> lastName;
        kvo_variable<std::string> fullName;
        
        Person()
        {
            firstName.subject.get_observable()
            .combine_latest(lastName.subject.get_observable())
            .subscribe([this](std::pair<std::string,std::string>x){
                this->fullName = x.first + " " + x.second;
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
            kvo_variable<Person> member;
        };
        AND_THEN("given a team with rx key path")
        {
            struct Team
            {
                kvo_variable<Leader> leader;
                kvo_variable<std::string> leader_fullName;
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
                kvo_variable<Leader> leader;
                kvo_variable<std::string> leader_fullName;
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

SCENARIO("test kvo_collection", "")
{
    GIVEN("a vector")
    {
        kvo_collection<std::vector<int>> IDs;
        IDs.set({100,200,300,400,500,600,700,800,900});
    }
}
