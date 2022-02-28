#pragma once

#include <string>
#include <sstream>

namespace wf
{
namespace log
{
/**
 * Convert the given parameter to a string which can be logged.
 * This function can be specialized for custom types.
 */
template<class T>
std::string to_string(T arg)
{
    std::ostringstream out;
    out << arg;
    return out.str();
}

/** Specialization for boolean arguments - print true or false. */
template<>
std::string to_string(bool arg);

/* Specialization for pointers - print the address */
template<class T>
std::string to_string(T *arg)
{
    if (!arg)
    {
        return "(null)";
    }

    return to_string<T*>(arg);
}

namespace detail
{
/**
 * Convert each argument to a string and then concatenate them.
 */
template<class First>
std::string format_concat(First arg)
{
    return wf::log::to_string(arg);
}

template<class First, class... Args>
std::string format_concat(First first, Args... args)
{
    return format_concat(first) + format_concat(args...);
}
}
}
}
