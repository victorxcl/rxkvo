
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

template<typename Collection>
class operation;

template<typename T>
class operation<std::vector<T>>
{
public:
    typedef std::vector<T>                              collection_type;
    typedef typename collection_type::difference_type   difference_type;
    typedef typename collection_type::const_iterator    const_iterator;
    typedef typename collection_type::iterator          iterator;
    typedef std::vector<T>                              rx_notify_value;
    typedef std::vector<difference_type>                rx_notify_index;
    
    static rx_notify_index indices_for_append_items(collection_type&c, const rx_notify_value&x)
    {
        rx_notify_index indices;
        for (difference_type i=0; i<x.size(); i++)
        {
            indices.push_back(c.size() + i);
        }
        return std::move(indices);
    }
    
    static rx_notify_value items_at_indices(collection_type&c, const rx_notify_index&indices)
    {
        rx_notify_value items;
        for (const auto&i:indices)
        {
            auto it = c.begin(); std::advance(it, i);
            items.push_back(*it);
        }
        return std::move(items);
    }
    
    static void set(collection_type&c, const rx_notify_value&x)
    {
        c = x;
    }
    static void insert(collection_type&c, const rx_notify_value&x, const rx_notify_index&indices)
    {
        for (difference_type i=0;i<x.size();i++)
        {
            auto it = c.begin(); std::advance(it, indices.at(i));
            c.insert(it, x.at(i));
        }
    }
    
    static void remove(collection_type&c, const rx_notify_index&indices)
    {
        for (auto it_i=indices.crbegin(); it_i!=indices.crend(); it_i++)
        {
            auto it_x = c.begin(); std::advance(it_x, *it_i);
            c.erase(it_x);
        }
    }
    
    static void replace(collection_type&c, const rx_notify_index&indices, const rx_notify_value&x)
    {
        for (difference_type i=0;i<x.size();i++)
        {
            auto it = c.begin(); std::advance(it, indices.at(i));
            *it = x.at(i);
        }
    }
};

template<typename Collection, typename Operation=operation<Collection>>
class kvo_collection
{
public:
    typedef Collection                                  collection_type;
    typedef Operation                                   operation_type;
    typedef typename collection_type::value_type        value_type;
    typedef typename collection_type::difference_type   difference_type;
    typedef typename collection_type::const_iterator    const_iterator;
    typedef typename collection_type::iterator          iterator;
    typedef std::vector<value_type>                     rx_notify_value;
    typedef std::vector<difference_type>                rx_notify_index;
private:
    collection_type collection;
public:
    rxcpp::subjects::subject<rx_notify_value> subject_setting_will;
    rxcpp::subjects::subject<rx_notify_value> subject_insertion_will;
    rxcpp::subjects::subject<rx_notify_value> subject_removal_will;
    rxcpp::subjects::subject<rx_notify_value> subject_replacement_will;
    
    rxcpp::subjects::subject<rx_notify_value> subject_setting_did;
    rxcpp::subjects::subject<rx_notify_value> subject_insertion_did;
    rxcpp::subjects::subject<rx_notify_value> subject_removal_did;
    rxcpp::subjects::subject<rx_notify_value> subject_replacement_did;
    
    rxcpp::subjects::subject<rx_notify_value>&subject_setting = subject_setting_did;
    rxcpp::subjects::subject<rx_notify_value>&subject_insertion = subject_insertion_did;
    rxcpp::subjects::subject<rx_notify_value>&subject_removal = subject_removal_did;
    rxcpp::subjects::subject<rx_notify_value>&subject_replacement = subject_replacement_did;
    
    rxcpp::subjects::subject<rx_notify_index> subject_insertion_index;
    rxcpp::subjects::subject<rx_notify_index> subject_removal_index;
    rxcpp::subjects::subject<rx_notify_index> subject_replacement_index;
    
    collection_type&get() { return this->collection; }
    
    collection_type& operator()() { return this->get(); }
    
    void set(const rx_notify_value&x)
    {
        if (this->collection.size() > 0 || (this->collection.size() == 0 && x.size() > 0))
        {
            this->subject_setting_will.get_subscriber().on_next(x);
            operation_type::set(this->collection, x);
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
            
            operation_type::insert(this->collection, x, indices);
            
            subject_insertion_did.get_subscriber().on_next(x);
        }
    }
    
    void insert(const rx_notify_value&x)
    {
        if (x.size() > 0)
        {
            rx_notify_index indices = operation_type::indices_for_append_items(this->collection, x);
            
            subject_insertion_index.get_subscriber().on_next(indices);
            
            subject_insertion_will.get_subscriber().on_next(x);
            
            this->collection.insert(this->collection.end(), x.begin(), x.end());
            
            subject_insertion_did.get_subscriber().on_next(x);
        }
    }
    
    void remove(const rx_notify_index&indices)
    {
        if (indices.size() > 0)
        {
            subject_removal_index.get_subscriber().on_next(indices);
            
            rx_notify_value items = operation_type::items_at_indices(this->collection, indices);
            
            subject_removal_will.get_subscriber().on_next(items);
            
            operation_type::remove(this->collection, indices);
            
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
            
            operation_type::replace(this->collection, indices, x);
            
            subject_replacement_did.get_subscriber().on_next(x);
        }
    }
};
