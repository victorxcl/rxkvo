#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

namespace wechat{
    
    namespace model
    {
        enum Gender { Unknown, Male, Female };
        
        struct User
        {
            kvo::variable<std::string> name;
            kvo::variable<std::string> account;
            kvo::variable<std::string> password;
        };
        
        struct HumanUser:public User
        {
            kvo::variable<int> age;
            kvo::variable<Gender> gender;
        };
        
        struct PublicUser:public User
        {
            
        };
        
        struct SelfUser:public HumanUser
        {
            
        };
        
        struct Message
        {
            kvo::variable<std::string> content;
            kvo::variable<bool> isReaded;
            kvo::variable<std::shared_ptr<User>> fromUser;
        };
        
        struct Chat
        {
            kvo::collection<std::vector<std::shared_ptr<Message>>> messages;
        };
        
        struct ToOneChat:public Chat
        {
            kvo::variable<std::shared_ptr<SelfUser>> selfUser;
            kvo::variable<std::shared_ptr<User>> withUser;
        };
        
        struct GroupChat:public Chat
        {
            kvo::variable<std::shared_ptr<SelfUser>> selfUser;
            kvo::collection<std::set<std::shared_ptr<User>>> users;
        };
        
        struct WeChat
        {
            kvo::variable<std::shared_ptr<SelfUser>> selfUser;
            kvo::collection<std::set<std::shared_ptr<User>>> users;     // without order
            kvo::collection<std::vector<std::shared_ptr<Chat>>> chats;  // with order
            
            WeChat()
            {
                selfUser = std::make_shared<SelfUser>();
            }
        };
    }
    
    namespace viewmodel
    {
        struct Register
        {
            kvo::variable<std::shared_ptr<model::User>> model;
            kvo::variable<std::string> textBox_account;
            kvo::variable<std::string> textBox_password;
            kvo::variable<bool> buttonRegister_enabled;
            rxcpp::subjects::subject<bool> subject_buttonRegisterClicked;
            
            Register()
            {
                textBox_account.subject.get_observable()
                .combine_latest(textBox_password.subject.get_observable())
                .map([](const std::tuple<std::string,std::string>&x){
                    auto&account = std::get<0>(x);
                    auto&password = std::get<1>(x);
                    return account.size() > 8 && password.size() > 6;
                })
                .subscribe([this](bool isEnabled){
                    this->buttonRegister_enabled = isEnabled;
                });
            }
        };
        struct Login
        {
            kvo::variable<std::string> textBox_account;
            kvo::variable<std::string> textBox_password;
            kvo::variable<bool> buttonLogin_enabled;
            rxcpp::subjects::subject<bool> subject_buttonLoginClicked;
            
            Login()
            {
                textBox_account.subject.get_observable()
                .combine_latest(textBox_password.subject.get_observable())
                .map([](const std::tuple<std::string,std::string>&x){
                    auto&account = std::get<0>(x);
                    auto&password = std::get<1>(x);
                    return account.size() > 8 && password.size() > 6;
                })
                .subscribe([this](bool isEnabled){
                    this->buttonLogin_enabled = isEnabled;
                });
            }
        };
        struct Chat
        {
            kvo::variable<std::shared_ptr<Chat>> model;
            kvo::variable<int> unreadCount;
        };
        
        struct ToOneChat:public Chat
        {
            
        };
        
        struct GroupChat:public Chat
        {
            
        };
        
        struct WeChat
        {
            kvo::variable<std::shared_ptr<model::WeChat>> model;
            kvo::variable<int> unreadCount;
            kvo::variable<int> iconCount;
        };
    }
}

SCENARIO("test Register", "")
{
    GIVEN("a wechat user A")
    {
        std::shared_ptr<wechat::model::WeChat> A = std::make_shared<wechat::model::WeChat>();
        
        THEN("initialize viewmodel::Register")
        {
            wechat::viewmodel::Register vm_Register;
            vm_Register.model = A->selfUser();
            REQUIRE(vm_Register.textBox_account() == "");
            REQUIRE(vm_Register.textBox_password() == "");
            REQUIRE(vm_Register.buttonRegister_enabled() == false);
            
            GIVEN("account with hello")
            {
                vm_Register.textBox_account = "hello";
                GIVEN("password with word")
                {
                    vm_Register.textBox_password = "word";
                    REQUIRE(vm_Register.buttonRegister_enabled() == false);
                }
                GIVEN("password with wordword")
                {
                    vm_Register.textBox_password = "worldword";
                    REQUIRE(vm_Register.buttonRegister_enabled() == false);
                }
            }
            GIVEN("account with hellohello")
            {
                vm_Register.textBox_account = "hellohello";
                GIVEN("password with word")
                {
                    vm_Register.textBox_password = "world";
                    REQUIRE(vm_Register.buttonRegister_enabled() == false);
                }
                GIVEN("password with wordword")
                {
                    vm_Register.textBox_password = "worldword";
                    REQUIRE(vm_Register.buttonRegister_enabled() == true);
                }
            }
        }
    }
}
