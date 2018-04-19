namespace kvo
{
    namespace __keypath__
    {
        template<typename Observable>
        auto switch_map(Observable o)->Observable
        {
            return o;
        }
        
        template<typename Observable, typename F, typename ... FN>
        auto switch_map(Observable o, F f, FN ... fn)
        ->decltype(switch_map(f(std::declval<typename Observable::value_type>()), fn ...));
        
        template<typename Observable, typename F, typename ... FN>
        auto switch_map(Observable o, F f, FN ... fn)
        ->decltype(switch_map(f(std::declval<typename Observable::value_type>()), fn ...))
        {
            return switch_map(o.map([f](typename Observable::value_type&x){ return f(x); }).switch_on_next(), fn ...);
        }
    }
    
    template<typename Observable, typename F, typename ... FN>
    auto keypath(Observable&&o, F&&f, FN&&... fn)
    ->decltype(__keypath__::switch_map(std::forward<Observable>(o), std::forward<F>(f), std::forward<FN>(fn)...))
    {
        return __keypath__::switch_map(std::forward<Observable>(o), std::forward<F>(f), std::forward<FN>(fn)...);
    }
    
    template<typename T>
    class variable
    {
        typedef variable<T> self_t;
    public:
        rxcpp::subjects::subject <T> subject_will;
        rxcpp::subjects::behavior<T>&subject_did = subject;
        rxcpp::subjects::behavior<T> subject = rxcpp::subjects::behavior<T>(T());
        variable() = default;
        variable(const self_t&o)
        {
            this->subject_will = o.subject_will;
            this->subject = o.subject;
        }
        
        self_t&operator=(const self_t&o)
        {
            if (this != &o)
            {
                this->subject_will = o.subject_will;
                this->subject = o.subject;
            }
            return *this;
        }
        
        void set(const T&x)
        {
            this->subject_will.get_subscriber().on_next(this->get());
            this->subject_did.get_subscriber().on_next(x);
        }
        T get() const
        {
            return this->subject.get_value();
        }
        T operator -> () const
        {
            return this->subject.get_value();
        }
        T operator * () const
        {
            return this->subject.get_value();
        }
        operator T () const
        {
            return this->subject.get_value();
        }
        self_t&operator = (const T&x)
        {
            set(x);
            return *this;
        }
        
        T operator()()const
        {
            return this->subject.get_value();
        }
        
        template<typename F, typename ... FN>
        auto operator()(F&&f, FN&&... fn)
        ->decltype(keypath(this->subject.get_observable(), f, fn ...))
        {
            return keypath(this->subject.get_observable(), f, fn ...);
        }
    };
    
    namespace workers
    {
        struct tag_worker_as_array {};
        struct tag_worker_as_set   {};
        struct tag_worker_as_map   {};
        
        template<typename Collection, typename RxContainer=Collection>
        struct worker_as_array;
        
        template<
        typename ...Args, template<typename...> class Container,
        typename ...Item, template<typename...> class RxContainer
        >
        struct worker_as_array<Container<Args...>, RxContainer<Item...>>
        {
        public:
            typedef tag_worker_as_array                         worker_tag;
            typedef Container<Args...>                          collection_type;
            typedef typename collection_type::value_type        value_type;
            typedef typename collection_type::difference_type   difference_type;
            typedef RxContainer<value_type>                     rx_notify_value;
            typedef RxContainer<difference_type>                rx_notify_index;
            typedef typename collection_type::iterator          iterator;
            typedef typename collection_type::const_iterator    const_iterator;
        private:
            collection_type                                     _c;
        public:
            
            rx_notify_index indices_for_append_items(const rx_notify_value&x)
            {
                rx_notify_index indices;
                for (std::size_t i=0; i<x.size(); i++)
                {
                    indices.push_back(_c.size() + i);
                }
                return std::move(indices);
            }
            
            rx_notify_value items_at_indices(const rx_notify_index&indices)
            {
                rx_notify_value items;
                for (const auto&i:indices)
                {
                    auto it = std::next(_c.begin(), i);
                    items.push_back(*it);
                }
                return std::move(items);
            }
            
            collection_type&get() { return _c; }
            
            void set(const collection_type&x)
            {
                _c = x;
            }
            
            void insert(const rx_notify_value&x, const rx_notify_index&indices)
            {
                for (std::size_t i=0;i<x.size();i++)
                {
                    auto it_i = std::next(indices.begin(), i);
                    auto it_x = std::next(x.begin(), i);
                    auto it_c = std::next(_c.begin(), *it_i);
                    _c.insert(it_c, *it_x);
                }
            }
            
            void insert(const rx_notify_value&x)
            {
                _c.insert(_c.end(), x.begin(), x.end());
            }
            
            void remove(const rx_notify_index&indices)
            {
                for (auto it_i=indices.crbegin(); it_i!=indices.crend(); it_i++)
                {
                    auto it_x = std::next(_c.begin(), *it_i);
                    _c.erase(it_x);
                }
            }
            
            void replace(const rx_notify_index&indices, const rx_notify_value&x)
            {
                for (auto i=0;i<x.size();i++)
                {
                    auto it_i = std::next(indices.begin(), i);
                    auto it_x = std::next(x.begin(), i);
                    auto it_c = std::next(_c.begin(), *it_i);
                    *it_c = *it_x;
                }
            }
        };
        
        template<typename Collection, typename RxContainer=Collection>
        struct worker_as_set;
        
        template<
        typename ...Args, template<typename...> class Container,
        typename ...Item, template<typename...> class RxContainer
        >
        struct worker_as_set<Container<Args...>, RxContainer<Item...>>
        {
        public:
            typedef tag_worker_as_set                           worker_tag;
            typedef Container<Args...>                          collection_type;
            typedef typename collection_type::value_type        value_type;
            typedef RxContainer<value_type>                     rx_notify_value;
        private:
            collection_type                                     _c;
        public:
            collection_type&get() { return _c; }
            
            void set(const collection_type&x)
            {
                _c = x;
            }
            
            void insert(const collection_type&x)
            {
                _c.insert(x.begin(), x.end());
            }
            
            void remove(const collection_type&values)
            {
                for (auto it=values.begin(); it!=values.end(); it++)
                {
                    _c.erase(*it);
                }
            }
            
            void replace(const collection_type&x, const collection_type&y)
            {
                remove(x);
                insert(y);
            }
        };
        
        template<typename Collection, typename RxContainer>
        struct worker_as_map;
        
        template<
        typename ...Args, template<typename...> class Container,
        typename ...Item, template<typename...> class RxContainer
        >
        struct worker_as_map<Container<Args...>, RxContainer<Item...>>
        {
        public:
            typedef tag_worker_as_map                           worker_tag;
            typedef Container<Args...>                          collection_type;
            typedef typename collection_type::key_type          index_type;
            typedef typename collection_type::value_type        value_type;
            typedef typename collection_type::difference_type   difference_type;
            typedef collection_type                             rx_notify_value;
            typedef RxContainer<index_type>                     rx_notify_index;
            typedef typename collection_type::iterator          iterator;
            typedef typename collection_type::const_iterator    const_iterator;
        private:
            collection_type                                     _c;
        public:
            
            rx_notify_value items_at_indices(const rx_notify_index&indices)
            {
                rx_notify_value items;
                for (const auto&i:indices)
                {
                    auto it_c = _c.find(i);
                    if (_c.end() != it_c)
                        items.insert(*it_c);
                }
                return std::move(items);
            }
            
            collection_type&get() { return _c; }
            
            void set(const collection_type&x)
            {
                _c = x;
            }
            
            void insert(const collection_type&x)
            {
                _c.insert(x.begin(), x.end());
            }
            
            void remove(const rx_notify_index&indices)
            {
                for (auto it_i=indices.begin(); it_i!=indices.end(); it_i++)
                {
                    _c.erase(*it_i);
                }
            }
            
            void replace(const collection_type&x)
            {
                for (auto it=x.begin(); it!=x.end(); it++)
                {
                    auto it_c = _c.find(it->first);
                    if (it_c != _c.end())
                        it_c->second = it->second;
                }
            }
        };
    }
    
    template<typename Collection, typename RxContainer=Collection> class worker;
    
    template<typename T> class worker<std::vector<T>> :public workers::worker_as_array<std::vector<T>> { };
    template<typename T> class worker<std::list<T>> :public workers::worker_as_array<std::list<T>> { };
    
    template<typename T> class worker<std::set<T>> :public workers::worker_as_set<std::set<T>> { };
    template<typename T> class worker<std::unordered_set<T>> :public workers::worker_as_set<std::unordered_set<T>> { };
    
    template<typename K,typename V> class worker<std::map<K,V>> :public workers::worker_as_map<std::map<K,V>,std::unordered_set<K>> { };
    template<typename K,typename V> class worker<std::unordered_map<K,V>> :public workers::worker_as_map<std::unordered_map<K,V>,std::unordered_set<K>> { };
    
    
    template<typename KVOCollectionType, typename CollectionType>
    struct collection_base
    {
        typedef CollectionType                      collection_type;
        template<typename C>
        std::enable_if_t<!std::is_same<C,collection_type>()> set(C&&c)
        {
            collection_type x;
            std::copy(std::begin(c), std::end(c), std::inserter(x, std::end(x)));
            static_cast<KVOCollectionType*>(this)->set(std::forward<collection_type>(x));
        }
        
        template<typename C>
        std::enable_if_t<!std::is_same<C,collection_type>()> insert(C&&c)
        {
            collection_type x;
            std::copy(std::begin(c), std::end(c), std::inserter(x, std::end(x)));
            static_cast<KVOCollectionType*>(this)->insert(std::forward<collection_type>(x));
        }
    };
    
    template<
    typename Collection,
    typename Worker=worker<Collection>,
    typename Tag=typename Worker::worker_tag
    >
    class collection;
    
    template<typename Collection, typename Worker>
    class collection<Collection, Worker, workers::tag_worker_as_array>
    : public collection_base<collection<Collection, Worker, workers::tag_worker_as_array>, Collection>
    {
    public:
        typedef collection                                  self_type;
        typedef Collection                                  collection_type;
        typedef Worker                                      worker_type;
        typedef typename worker_type::rx_notify_value       rx_notify_value;
        typedef typename worker_type::rx_notify_index       rx_notify_index;
    private:
        worker_type                                         worker;
    public:
        rxcpp::subjects::subject<rx_notify_value>           subject_setting_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_insertion_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_removal_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_replacement_will;
        
        rxcpp::subjects::subject<rx_notify_value>           subject_setting_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_insertion_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_removal_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_replacement_did;
        
        rxcpp::subjects::subject<rx_notify_value>&          subject_setting     = subject_setting_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_insertion   = subject_insertion_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_removal     = subject_removal_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_replacement = subject_replacement_did;
        
        rxcpp::subjects::subject<rx_notify_index>           subject_insertion_index;
        rxcpp::subjects::subject<rx_notify_index>           subject_removal_index;
        rxcpp::subjects::subject<rx_notify_index>           subject_replacement_index;
        
        collection_type&get() { return this->worker.get(); }
        
        collection_type& operator()() { return this->get(); }
        
        collection_type* operator -> () { return &this->get(); }
        collection_type& operator * () { return this->get(); }
        
        using collection_base<self_type, Collection>::set;
        using collection_base<self_type, Collection>::insert;
        
        
        void set(const collection_type&x)
        {
            if (this->get().size() > 0 || (this->get().size() == 0 && x.size() > 0))
            {
                this->subject_setting_will.get_subscriber().on_next(this->worker.get());
                this->worker.set(x);
                this->subject_setting_did.get_subscriber().on_next(x);
            }
        }
        
        void insert(const rx_notify_value&x, const rx_notify_index&indices)
        {
            assert(x.size() == indices.size());
            if (x.size() > 0 && indices.size() > 0 && x.size() == indices.size())
            {
                subject_insertion_index.get_subscriber().on_next(indices);
                subject_insertion_will.get_subscriber().on_next(x);
                this->worker.insert(x, indices);
                subject_insertion_did.get_subscriber().on_next(x);
            }
        }
        
        void insert(const rx_notify_value&x)
        {
            if (x.size() > 0)
            {
                rx_notify_index indices = this->worker.indices_for_append_items(x);
                subject_insertion_index.get_subscriber().on_next(indices);
                subject_insertion_will.get_subscriber().on_next(x);
                this->worker.insert(x);
                subject_insertion_did.get_subscriber().on_next(x);
            }
        }
        
        void remove(const rx_notify_index&indices)
        {
            if (indices.size() > 0)
            {
                subject_removal_index.get_subscriber().on_next(indices);
                rx_notify_value items = this->worker.items_at_indices(indices);
                subject_removal_will.get_subscriber().on_next(items);
                this->worker.remove(indices);
                subject_removal_did.get_subscriber().on_next(items);
            }
        }
        
        void remove_all()
        {
            this->set({});
        }
        
        void replace(const rx_notify_index&indices, const rx_notify_value&x)
        {
            assert(x.size() == indices.size());
            if (x.size() > 0 && indices.size() > 0 && x.size() == indices.size())
            {
                subject_replacement_index.get_subscriber().on_next(indices);
                subject_replacement_will.get_subscriber().on_next(this->worker.items_at_indices(indices));
                this->worker.replace(indices, x);
                subject_replacement_did.get_subscriber().on_next(x);
            }
        }
    };
    
    template<typename Collection, typename Worker>
    class collection<Collection, Worker, workers::tag_worker_as_set>
    : public collection_base<collection<Collection, Worker, workers::tag_worker_as_set>, Collection>
    {
    public:
        typedef collection                                  self_type;
        typedef Collection                                  collection_type;
        typedef Worker                                      worker_type;
        typedef typename worker_type::rx_notify_value       rx_notify_value;
    private:
        worker_type                                         worker;
    public:
        rxcpp::subjects::subject<rx_notify_value>           subject_setting_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_insertion_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_removal_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_replacement_will;
        
        rxcpp::subjects::subject<rx_notify_value>           subject_setting_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_insertion_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_removal_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_replacement_did;
        
        rxcpp::subjects::subject<rx_notify_value>&          subject_setting     = subject_setting_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_insertion   = subject_insertion_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_removal     = subject_removal_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_replacement = subject_replacement_did;
        
        collection_type&get() { return this->worker.get(); }
        
        collection_type& operator()() { return this->get(); }
        
        collection_type* operator -> () { return &this->get(); }
        collection_type& operator * () { return this->get(); }
        
        using collection_base<self_type, Collection>::set;
        using collection_base<self_type, Collection>::insert;
        
        void set(const rx_notify_value&x)
        {
            if (this->get().size() > 0 || (this->get().size() == 0 && x.size() > 0))
            {
                this->subject_setting_will.get_subscriber().on_next(x);
                this->worker.set(x);
                this->subject_setting_did.get_subscriber().on_next(x);
            }
        }
        
        void insert(const rx_notify_value&x)
        {
            if (x.size() > 0)
            {
                subject_insertion_will.get_subscriber().on_next(x);
                this->worker.insert(x);
                subject_insertion_did.get_subscriber().on_next(x);
            }
        }
        
        void remove(const rx_notify_value&x)
        {
            if (x.size() > 0)
            {
                subject_removal_will.get_subscriber().on_next(x);
                this->worker.remove(x);
                subject_removal_did.get_subscriber().on_next(x);
            }
        }
        
        void remove_all()
        {
            this->set({});
        }
        
        void replace(const rx_notify_value&x, const rx_notify_value&y)
        {
            if (x.size() > 0 || y.size() > 0)
            {
                subject_replacement_will.get_subscriber().on_next(x);
                this->worker.replace(x, y);
                subject_replacement_did.get_subscriber().on_next(y);
            }
        }
    };
    
    template<typename Collection, typename Worker>
    class collection<Collection, Worker, workers::tag_worker_as_map>
    : public collection_base<collection<Collection, Worker, workers::tag_worker_as_map>, Collection>
    {
    public:
        typedef collection                                  self_type;
        typedef Collection                                  collection_type;
        typedef Worker                                      worker_type;
        typedef typename worker_type::rx_notify_value       rx_notify_value;
        typedef typename worker_type::rx_notify_index       rx_notify_index;
    private:
        worker_type                                         worker;
    public:
        rxcpp::subjects::subject<rx_notify_value>           subject_setting_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_insertion_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_removal_will;
        rxcpp::subjects::subject<rx_notify_value>           subject_replacement_will;
        
        rxcpp::subjects::subject<rx_notify_value>           subject_setting_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_insertion_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_removal_did;
        rxcpp::subjects::subject<rx_notify_value>           subject_replacement_did;
        
        rxcpp::subjects::subject<rx_notify_value>&          subject_setting     = subject_setting_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_insertion   = subject_insertion_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_removal     = subject_removal_did;
        rxcpp::subjects::subject<rx_notify_value>&          subject_replacement = subject_replacement_did;
        
        collection_type&get() { return this->worker.get(); }
        
        collection_type& operator()() { return this->get(); }
        
        collection_type* operator -> () { return &this->get(); }
        collection_type& operator * () { return this->get(); }
        
        using collection_base<self_type, Collection>::set;
        using collection_base<self_type, Collection>::insert;
        
        void set(const collection_type&x)
        {
            if (this->get().size() > 0 || (this->get().size() == 0 && x.size() > 0))
            {
                this->subject_setting_will.get_subscriber().on_next(x);
                this->worker.set(x);
                this->subject_setting_did.get_subscriber().on_next(x);
            }
        }
        
        void insert(const collection_type&x)
        {
            if (x.size() > 0)
            {
                subject_insertion_will.get_subscriber().on_next(x);
                this->worker.insert(x);
                subject_insertion_did.get_subscriber().on_next(x);
            }
        }
        
        void remove(const rx_notify_index&indices)
        {
            if (indices.size() > 0)
            {
                rx_notify_value items = this->worker.items_at_indices(indices);
                subject_removal_will.get_subscriber().on_next(items);
                this->worker.remove(indices);
                subject_removal_did.get_subscriber().on_next(items);
            }
        }
        
        void remove_all()
        {
            this->set({});
        }
        
        void replace(const collection_type&x)
        {
            if (x.size() > 0)
            {
                subject_replacement_will.get_subscriber().on_next(x);
                this->worker.replace(x);
                subject_replacement_did.get_subscriber().on_next(x);
            }
        }
    };
}
