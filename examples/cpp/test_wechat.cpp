#include <rxcpp/rx.hpp>
#include <catch.hpp>
#include <unordered_map>
#include "kvo_extension.hpp"

#pragma warning(disable:4503)

namespace wechat{
    
    namespace protocol
    {
        struct Register
        {
            struct Request
            {
                std::string account;
                std::string password;
            };
            struct Respond
            {
                std::size_t userID;
                std::string account;
                std::string password;
            };
        };
        struct Login
        {
            struct Request
            {
                std::string account;
                std::string password;
            };
            struct Respond
            {
                std::string userID;
            };
        };
        struct SendMessage
        {
            struct Request
            {
                std::string userID;
                std::string content;
            };
            struct Respond
            {
                
            };
        };
    }
    namespace server
    {
        typedef std::size_t user_id;
        struct User
        {
            user_id ID;
            std::string account;
            std::string password;
        };
        struct Message
        {
            user_id userID;
            std::string content;
        };
        struct Server
        {
            std::unordered_map<user_id, std::shared_ptr<User>> users;
            
            rxcpp::subjects::subject<std::tuple<protocol::Register::Request, std::function<void(protocol::Register::Respond)>>> subject_register;
            rxcpp::subjects::subject<protocol::Login> subject_login;
            rxcpp::subjects::subject<protocol::SendMessage> subject_send_message;
            
            void api_user_register(const protocol::Register::Request &x, std::function<void(protocol::Register::Respond)> f)
            {
                this->subject_register.get_subscriber().on_next(std::make_tuple(x, f));
            }

            Server()
            {
                {
                    struct Local
                    {
                        user_id userID = 0;
                    };
                    auto local = std::make_shared<Local>();
                    subject_register.get_observable()
                    .subscribe([this,local](const std::tuple<protocol::Register::Request, std::function<void(protocol::Register::Respond)>> x){
                        auto&req = std::get<0>(x);
                        auto&f = std::get<1>(x);
                        auto it = std::find_if(std::begin(users), std::end(users), [req](const decltype(users)::value_type&y){
                            return y.second->account == req.account;
                        });
                        
                        if (this->users.end() == it)
                        {
                            auto user = std::make_shared<User>();
                            {
                                user->ID = ++local->userID;
                                user->account = req.account;
                                user->password = req.password;
                            }
                            this->users.insert(std::make_pair(user->ID, user));
                            protocol::Register::Respond respond;
                            {
                                respond.userID = user->ID;
                                respond.account = user->account;
                                respond.password = user->password;
                            }
                            f(respond);
                        }
                    });
                }
            }
        };
    }
    
    namespace model
    {
        enum Gender { Unknown, Male, Female };
        
        struct User
        {
            kvo::variable<std::size_t> userID;
            kvo::variable<std::string> account;
            kvo::variable<std::string> password;
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
            kvo::variable<std::shared_ptr<User>> selfUser;
            kvo::variable<std::shared_ptr<User>> withUser;
            kvo::variable<int> unreadCount;
            Chat()
            {
                messages.subject_setting.get_observable()
                .subscribe([this](const decltype(messages)::collection_type&x){
                    int n = 0;
                    for (auto message:x)
                        n += message->isReaded()?0:1;
                    this->unreadCount = n;
                });
                messages.subject_insertion.get_observable()
                .subscribe([this](const decltype(messages)::collection_type&x){
                    int n = this->unreadCount();
                    for (auto message:x)
                        n += message->isReaded()?0:1;
                    this->unreadCount = n;
                });
            }
            
            void send(const std::string&msg)
            {
                auto message = std::make_shared<model::Message>();
                message->fromUser = this->selfUser();
                message->content = msg;
                message->isReaded = false;
                this->messages.insert({message});
            }
            void recv(const std::string&msg)
            {
                auto message = std::make_shared<model::Message>();
                message->fromUser = this->withUser();
                message->content = msg;
                message->isReaded = false;
                this->messages.insert({message});
            }
        };
        
        struct WeChat
        {
            kvo::variable<std::shared_ptr<User>> selfUser;
            kvo::collection<std::set<std::shared_ptr<User>>> users;     // without order
            kvo::collection<std::vector<std::shared_ptr<Chat>>> chats;  // with order
            
            kvo::variable<int> iconCount;
            
            WeChat()
            {
                selfUser = std::make_shared<User>();
                
                rxcpp::observable<>::empty<decltype(chats)::collection_type>()
                .merge(chats.subject_setting.get_observable(),
                       chats.subject_insertion.get_observable())
                .map([this](const decltype(chats)::collection_type&x){
                    return rxcpp::observable<>::iterate(x)
                    .flat_map([](std::shared_ptr<Chat>x){
                        return x->unreadCount.subject.get_observable();
                    }, [](std::shared_ptr<Chat>, int x){
                        return x;
                    });
                }).switch_on_next()
                .scan(0, [](int sum, int x) { return sum + x; })
                .subscribe([this](int n){
                    this->iconCount = n;
                });
            }
            void addChat(std::shared_ptr<Chat>x)
            {
                chats().push_back(x);
            }
        };
    }
    
    namespace viewmodel
    {
        struct API
        {
            std::shared_ptr<server::Server> server;
            rxcpp::observable<protocol::Register::Respond> signal_requestRegister(const std::string&account, const std::string&password)
            {
                return rxcpp::observable<>::create<protocol::Register::Respond>([this,account,password](rxcpp::subscriber<protocol::Register::Respond> subscriber){
                    server->api_user_register(protocol::Register::Request{account, password}, [subscriber](const protocol::Register::Respond&x){
                        subscriber.on_next(x);
                        subscriber.on_completed();
                    });
                });
            }
        };
        
        struct Register:public API
        {
            kvo::variable<std::shared_ptr<model::User>> model;
            kvo::variable<std::string> textBox_account;
            kvo::variable<std::string> textBox_password;
            kvo::variable<bool> buttonRegister_enabled;
            rxcpp::subjects::subject<bool> subject_buttonRegisterClicked;
            
            void click_buttonRegister()
            {
                subject_buttonRegisterClicked.get_subscriber().on_next(true);
            }
            
            Register()
            {
                {
                    textBox_account.subject.get_observable()
                    .combine_latest(textBox_password.subject.get_observable())
                    .map([](const std::tuple<std::string,std::string>&x){
                        auto&account = std::get<0>(x);
                        auto&password = std::get<1>(x);
                        return account.size() >= 8 && password.size() >= 6;
                    })
                    .subscribe([this](bool isEnabled){
                        this->buttonRegister_enabled = isEnabled;
                    });
                }
                {
                    subject_buttonRegisterClicked.get_observable()
                    .with_latest_from(buttonRegister_enabled.subject.get_observable())
                    .filter([](auto x){ return std::get<1>(x) == true; })
                    .map([](auto x){ return std::get<0>(x); })
                    .flat_map([this](bool){
                        return API::signal_requestRegister(textBox_account(),
                                                           textBox_password());
                    }, [](bool, auto x){ return x; })
                    .subscribe([this](auto x){
                        auto user = this->model();
                        user->userID = x.userID;
                        user->account = x.account;
                        user->password = x.password;
                    }, [](std::exception_ptr){});
                }
            }
        };
        
        struct Chat
        {
            kvo::variable<std::shared_ptr<model::Chat>> model;
            kvo::variable<int> unreadCount;
            Chat()
            {
                model.subject.get_observable()
                .filter([](std::shared_ptr<model::Chat>x){ return nullptr != x; })
                .map([](std::shared_ptr<model::Chat>x){ return x->unreadCount.subject.get_observable(); }).switch_on_next()
                .subscribe([this](int x){
                    this->unreadCount = x;
                });
            }
            void send(const std::string&msg)
            {
                model()->send(msg);
            }
            void recv(const std::string&msg)
            {
                model()->recv(msg);
            }
            void readAll()
            {
                for (auto message:model()->messages())
                    message->isReaded = true;
            }
        };
        
        struct WeChat
        {
            kvo::variable<std::shared_ptr<model::WeChat>> model;
            kvo::variable<int> iconCount;
            
            WeChat()
            {
                model.subject.get_observable()
                .filter([](std::shared_ptr<model::WeChat>x){ return nullptr != x; })
                .map([](std::shared_ptr<model::WeChat>x){ return x->iconCount.subject.get_observable(); }).switch_on_next()
                .subscribe([this](int x){
                    this->iconCount = x;
                });
            }
        };
    }
}

SCENARIO("test Register", "")
{
    auto server = std::make_shared<wechat::server::Server>();
    GIVEN("a wechat user A")
    {
        auto A = std::make_shared<wechat::model::WeChat>();
        
        THEN("initialize viewmodel::Register")
        {
            wechat::viewmodel::Register vm_Register;
            vm_Register.server = server;
            
            vm_Register.model = A->selfUser();
            REQUIRE(vm_Register.textBox_account() == "");
            REQUIRE(vm_Register.textBox_password() == "");
            REQUIRE(vm_Register.buttonRegister_enabled() == false);
            
            THEN("test the enabled status of button register")
            {
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
                        vm_Register.textBox_password = "worldworld";
                        REQUIRE(vm_Register.buttonRegister_enabled() == true);
                    }
                }
            }
            THEN("test register to server")
            {
                GIVEN("invalid account or password")
                {
                    vm_Register.textBox_account = "hello";
                    vm_Register.textBox_password = "world";
                    THEN("click button register")
                    {
                        vm_Register.click_buttonRegister();
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        REQUIRE(server->users.empty());
                    }
                }
                GIVEN("valid account or password")
                {
                    vm_Register.textBox_account = "pandaxcl@gmail.com"; // length >= 8
                    vm_Register.textBox_password = "123456";            // length >= 6
                    
                    REQUIRE(vm_Register.buttonRegister_enabled() == true);
                    
                    THEN("click button register")
                    {
                        vm_Register.click_buttonRegister();
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        REQUIRE(server->users.size() == 1);
                        auto user = vm_Register.model();
                        REQUIRE(user->userID() != 0);
                        REQUIRE(user->account() == "pandaxcl@gmail.com");
                        REQUIRE(user->password() == "123456");
                    }
                }
            }
        }
    }
}

SCENARIO("test conversation", "")
{
    GIVEN("two users A and B")
    {
        auto vm_A = std::make_shared<wechat::viewmodel::WeChat>();
        auto vm_B = std::make_shared<wechat::viewmodel::WeChat>();
        
        vm_A->model = std::make_shared<wechat::model::WeChat>();
        vm_B->model = std::make_shared<wechat::model::WeChat>();
        
        THEN("A build a chat to B")
        {
            {
                auto chat = std::make_shared<wechat::model::Chat>();
                chat->selfUser = vm_A->model()->selfUser();
                chat->withUser = vm_B->model()->selfUser();
                vm_A->model()->addChat(chat);
            }
            {
                auto chat = std::make_shared<wechat::model::Chat>();
                chat->selfUser = vm_B->model()->selfUser();
                chat->withUser = vm_A->model()->selfUser();
                vm_B->model()->addChat(chat);
            }
            WHEN("A send a message to B")
            {
                {
                    auto vm = std::make_shared<wechat::viewmodel::Chat>();
                    vm->model = vm_A->model()->chats().front();
                    REQUIRE(vm->unreadCount() == 0);
                    vm->send("Hello");
                    REQUIRE(vm->unreadCount() == 1);
                }
                {
                    auto vm = std::make_shared<wechat::viewmodel::Chat>();
                    vm->model = vm_B->model()->chats().front();
                    REQUIRE(vm->unreadCount() == 0);
                    vm->recv("Hello");
                    REQUIRE(vm->unreadCount() == 1);
                }
//                REQUIRE(vm_A->iconCount() == 1);
//                REQUIRE(vm_B->iconCount() == 1);
            }
        }
    }
}
