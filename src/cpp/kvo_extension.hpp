
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
    typedef Collection collection_type;
    typedef typename collection_type::value_type value_type;
    typedef typename collection_type::difference_type difference_type;
    collection_type collection;
public:
    rxcpp::subjects::subject<std::vector<value_type>> subject_setting;
    rxcpp::subjects::subject<std::vector<value_type>> subject_insertion;
    rxcpp::subjects::subject<std::vector<value_type>> subject_removal;
    rxcpp::subjects::subject<std::vector<value_type>> subject_replacement;
    
    rxcpp::subjects::subject<std::vector<difference_type>> subject_insertion_index;
    rxcpp::subjects::subject<std::vector<difference_type>> subject_removal_index;
    rxcpp::subjects::subject<std::vector<difference_type>> subject_replacement_index;
    
    collection_type&get()const
    {
        return this->collection;
    }
    
    collection_type& operator()()const
    {
        return this->get();
    }
    
    void set(const collection_type&x)
    {
        if (x.size() > 0)
        {
            this->subject_setting.get_subscriber().on_next(x);
        }
    }
    
    void insert(const std::vector<value_type>&x, const std::vector<difference_type>&indices)
    {
        assert(x.size() == indices.size());
        if (x.size() > 0 && indices.size() > 0 && x.size() == indices.size())
        {
            subject_insertion_index.get_subscriber().on_next(indices);
            subject_insertion.get_subscriber().on_next(x);
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
            subject_removal.get_subscriber().on_next(items);
        }
    }
    void replace(const std::vector<difference_type>&indices, const std::vector<value_type>&x)
    {
        assert(x.size() == indices.size());
        if (x.size() > 0 && indices.size() > 0 && x.size() == indices.size())
        {
            subject_replacement_index.get_subscriber().on_next(x);
            subject_replacement.get_subscriber().on_next(x);
        }
    }
};
