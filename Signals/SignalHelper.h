#ifndef SLOT_HELPER_H
#define SLOT_HELPER_H

#include <functional>
#include <string>

template<typename T>
struct SignalHelper 
{
    using Callback = std::function<void(const T&)>;

    static void invoke(const Callback& callback, const T& value) 
    {
        callback(value);
    }
};

template<>
struct SignalHelper<void> 
{
    using Callback = std::function<void()>;

    static void invoke(const Callback& callback, void* = nullptr) 
    {
        callback();
    }
};

struct AnyComparator
{
    bool operator()(const std::any& a, const std::any& b) const
    {
        if (a.type() != b.type())
        {
            return a.type().before(b.type());
        }

        if (a.type() == typeid(int))
        {
            return std::any_cast<int>(a) < std::any_cast<int>(b);
        }
        else if (a.type() == typeid(std::string))
        {
            return std::any_cast<std::string>(a) < std::any_cast<std::string>(b);
        }

        return false;
    }
};

template<typename T>
struct FunctionComparator {
    bool operator()(const std::function<void(const T&)>& a, const std::function<void(const T&)>& b) const
    {
        return a.target<void(const T&)>() != b.target<void(const T&)>();
    }
};

template <typename T>
class SignalContext {
public:
    T context;

    SignalContext() : context() {}
    SignalContext(T ctx) : context(std::move(ctx)) {}

    bool operator<(const SignalContext& other) const {
        return context < other.context;
    }
};

#endif