#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

namespace wechat{
    
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
    
    class WeChatModel
    {
        kvo::collection<std::set<std::shared_ptr<User>>> users;     // without order
        kvo::collection<std::vector<std::shared_ptr<Chat>>> chats;  // with order
    };
    
    class ChatViewModel
    {
        kvo::variable<std::shared_ptr<Chat>> model;
        kvo::variable<int> unreadCount;
    };
    
    class ToOneChatViewModel:public ChatViewModel
    {
        
    };
    
    class GroupChatViewModel:public ChatViewModel
    {
        
    };
    
    class WeChatViewModel
    {
        kvo::collection<std::vector<std::shared_ptr<ChatViewModel>>> chatViewModel;
        kvo::variable<int> unreadCount;
        kvo::variable<int> iconCount;
    };
    
    class WeChatApp
    {
        kvo::variable<std::shared_ptr<WeChatModel>> model;
        kvo::variable<std::shared_ptr<WeChatViewModel>> viewModel;
        kvo::variable<std::shared_ptr<SelfUser>> selfUser;
    };
}

SCENARIO("test long key path", "")
{
    wechat::WeChatApp wechat;
}
