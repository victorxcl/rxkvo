
template<typename T>
class kvo_variable
{
    typedef kvo_variable self_t;
public:
    self_t&operator = (T&&x)
    {
        this->subject.get_subscriber().on_next(x);
        return *this;
    }
    operator T()const
    {
        return this->subject.get_value();
    }
    T get()const
    {
        return this->subject.get_value();
    }
    rxcpp::subjects::behavior<T> subject = rxcpp::subjects::behavior<T>(T());
};


class kvo_array
{
};
