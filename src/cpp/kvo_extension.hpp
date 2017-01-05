
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
class kvo_collection
{
public:
    typedef Collection                                  collection_type;
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
    
    collection_type&get()
    {
        return this->collection;
    }
    
    collection_type& operator()()
    {
        return this->get();
    }
    
    void set(const collection_type&x)
    {
        if (x.size() > 0)
        {
            this->subject_setting_will.get_subscriber().on_next(x);
            this->collection = x;
            this->subject_setting_did.get_subscriber().on_next(x);
        }
    }
    
    const_iterator const_iterator_at(difference_type i)const
    {
        auto it = this->collection.begin();
        std::advance(it, i);
        return it;
    }
    
    iterator iterator_at(difference_type i)
    {
        auto it = this->collection.begin();
        std::advance(it, i);
        return it;
    }
    
    void insert(const std::vector<value_type>&x, const std::vector<difference_type>&indices)
    {
        assert(x.size() == indices.size());
        if (x.size() > 0 && indices.size() > 0 && x.size() == indices.size())
        {
            subject_insertion_index.get_subscriber().on_next(indices);
            
            subject_insertion_will.get_subscriber().on_next(x);
            
            for (difference_type i=0;i<x.size();i++)
            {
                auto it = const_iterator_at(indices.at(i));
                this->collection.insert(it, x.at(i));
            }
            
            subject_insertion_did.get_subscriber().on_next(x);
        }
    }
    
    void remove(const std::vector<difference_type>&indices)
    {
        if (indices.size() > 0)
        {
            subject_removal_index.get_subscriber().on_next(indices);
            
            std::vector<value_type> items;
            for (const auto&i:indices)
            {
                auto it = this->collection.begin(); std::advance(it, i);
                items.push_back(*it);
            }
            
            subject_removal_will.get_subscriber().on_next(items);
            
            for (auto it=indices.crbegin(); it!=indices.crend(); it++)
            {
                this->collection.erase(const_iterator_at(*it));
            }
            
            subject_removal_did.get_subscriber().on_next(items);
        }
    }
    
    void replace(const std::vector<difference_type>&indices, const std::vector<value_type>&x)
    {
        assert(x.size() == indices.size());
        if (x.size() > 0 && indices.size() > 0 && x.size() == indices.size())
        {
            subject_replacement_index.get_subscriber().on_next(indices);
            
            subject_replacement_will.get_subscriber().on_next(x);
            
            for (difference_type i=0;i<x.size();i++)
            {
                *iterator_at(indices.at(i)) = x.at(i);
            }
            
            subject_replacement_did.get_subscriber().on_next(x);
        }
    }
};
