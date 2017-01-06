
namespace detail
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


template<typename T>
class kvo_variable
{
    typedef kvo_variable self_t;
public:
    rxcpp::subjects::behavior<T> subject = rxcpp::subjects::behavior<T>(T());
    void set(const T&x)
    {
        this->subject.get_subscriber().on_next(x);
    }
    T get()const
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
    auto operator()(F f, FN ... fn)
    ->decltype(detail::switch_map(this->subject.get_observable(), f, fn ...))
    {
        return detail::switch_map(this->subject.get_observable(), f, fn ...);
    }
};

namespace detail
{
    template<typename Collection>
    struct worker_with_index;
    
    template<typename ...Args, template<typename...> class Container>
    struct worker_with_index<Container<Args...>>
    {
    public:
        typedef Container<Args...>                          collection_type;
        typedef typename collection_type::value_type        value_type;
        typedef typename collection_type::difference_type   difference_type;
        typedef Container<value_type>                       rx_notify_value;
        typedef Container<difference_type>                  rx_notify_index;
        typedef typename collection_type::iterator          iterator;
        typedef typename collection_type::const_iterator    const_iterator;
    private:
        collection_type                                     _c;
    public:
        
        rx_notify_index indices_for_append_items(const rx_notify_value&x)
        {
            rx_notify_index indices;
            for (difference_type i=0; i<x.size(); i++)
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
                auto it = _c.begin(); std::advance(it, i);
                items.push_back(*it);
            }
            return std::move(items);
        }
        
        collection_type&get() { return _c; }
        
        void set(const rx_notify_value&x)
        {
            _c = x;
        }
        
        void insert(const rx_notify_value&x, const rx_notify_index&indices)
        {
            for (difference_type i=0;i<x.size();i++)
            {
                auto it_i = indices.begin(); std::advance(it_i, i);
                auto it_x = x.begin(); std::advance(it_x, i);
                auto it_c = _c.begin(); std::advance(it_c, *it_i);
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
                auto it_x = _c.begin(); std::advance(it_x, *it_i);
                _c.erase(it_x);
            }
        }
        
        void replace(const rx_notify_index&indices, const rx_notify_value&x)
        {
            for (auto i=0;i<x.size();i++)
            {
                auto it_i = indices.begin(); std::advance(it_i, i);
                auto it_x = x.begin(); std::advance(it_x, i);
                auto it_c = _c.begin(); std::advance(it_c, *it_i);
                *it_c = *it_x;
            }
        }
    };
}

template<typename Collection> class worker;

template<typename T> class worker<std::vector<T>> :public detail::worker_with_index<std::vector<T>> { };
template<typename T> class worker<std::list<T>> :public detail::worker_with_index<std::list<T>> { };

template<typename Collection, typename Worker=worker<Collection>>
class kvo_collection
{
public:
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
    
    void set(const rx_notify_value&x)
    {
        if (this->get().size() > 0 || (this->get().size() == 0 && x.size() > 0))
        {
            this->subject_setting_will.get_subscriber().on_next(x);
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
            
            subject_replacement_will.get_subscriber().on_next(x);
            
            this->worker.replace(indices, x);
            
            subject_replacement_did.get_subscriber().on_next(x);
        }
    }
};
