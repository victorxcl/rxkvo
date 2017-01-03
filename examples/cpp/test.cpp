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
    GIVEN("a leader with team members")
    {
        struct Leader
        {
            kvo_variable<Person> member;
        };
        struct Team
        {
            kvo_variable<Leader> leader;
            Person members[3];
            kvo_variable<std::string> leader_fullName;
            Team()
            {
                //leader
            }
        }team;
        
        REQUIRE(team.leader_fullName.get() == "");
        
        team.members[0].firstName = "Hello"; team.members[0].lastName = "World";
        team.members[1].firstName = "Good" ; team.members[1].lastName = "Luck";
        team.members[2].firstName = "Happy"; team.members[2].lastName = "New Year";
        
        REQUIRE(team.leader.get().member.get().firstName.get() == "");
        REQUIRE(team.leader.get().member.get().lastName.get() == "");
        REQUIRE(team.leader.get().member.get().fullName.get() == " ");
        
        WHEN("0 as leader")
        {
            team.leader.get().member = team.members[0];
            REQUIRE(team.leader.get().member.get().firstName.get() == "Hello");
            REQUIRE(team.leader.get().member.get().lastName.get() == "World");
            REQUIRE(team.leader.get().member.get().fullName.get() == "Hello World");
        }
    }
}
