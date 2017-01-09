#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

namespace wechat{
    
    namespace model
    {
        enum Gender { Unknown, Male, Female };
        
        class User
        {
            kvo::variable<std::string> name;
        };
        
        class HumanUser:public User
        {
            kvo::variable<int> age;
            kvo::variable<Gender> gender;
        };
        
        class PublicUser:public User
        {
            
        };
        
        class SelfUser:public HumanUser
        {
            
        };
        
        class Message
        {
            kvo::variable<std::string> content;
            kvo::variable<bool> isReaded;
            kvo::variable<std::shared_ptr<User>> fromUser;
        };
        
        class Chat
        {
            kvo::collection<std::vector<std::shared_ptr<Message>>> messages;
        };
        
        class ToOneChat:public Chat
        {
            kvo::variable<std::shared_ptr<SelfUser>> selfUser;
            kvo::variable<std::shared_ptr<User>> withUser;
        };
        
        class GroupChat:public Chat
        {
            kvo::variable<std::shared_ptr<SelfUser>> selfUser;
            kvo::collection<std::set<std::shared_ptr<User>>> users;
        };
        
        class WeChat
        {
            kvo::collection<std::set<std::shared_ptr<User>>> users;     // without order
            kvo::collection<std::vector<std::shared_ptr<Chat>>> chats;  // with order
        };
    }
    
    namespace viewmodel
    {
        class Register
        {
            
        };
        class Chat
        {
            kvo::variable<std::shared_ptr<Chat>> model;
            kvo::variable<int> unreadCount;
        };
        
        class ToOneChat:public Chat
        {
            
        };
        
        class GroupChat:public Chat
        {
            
        };
        
        class WeChat
        {
            kvo::variable<std::shared_ptr<model::WeChat>> model;
            kvo::variable<int> unreadCount;
            kvo::variable<int> iconCount;
        };
    }
}

SCENARIO("test long key path", "")
{
    wechat::viewmodel::WeChat A, B, C;
}
