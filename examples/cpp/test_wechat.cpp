#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

SCENARIO("test long key path", "")
{
    enum Gender { Unknown, Male, Female };
    class User
    {
        kvo::variable<std::string> name;
        kvo::variable<int> age;
        kvo::variable<Gender> gender;
    };
    class Message
    {
        kvo::variable<std::string> content;
        kvo::variable<bool> isReaded;
    };
    class WeChat
    {
        kvo::variable<int> unreadCount;
        kvo::collection<std::vector<std::shared_ptr<Message>>> messages;
    };
}
