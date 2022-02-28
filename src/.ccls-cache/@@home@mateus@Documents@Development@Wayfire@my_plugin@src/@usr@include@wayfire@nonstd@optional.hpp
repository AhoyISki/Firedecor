#pragma once

#if __has_include(<optional>)
    #include <optional>
namespace stdx
{
template<class T>
using optional = std::optional<T>;
}

#else
    #include <experimental/optional>
namespace stdx
{
template<class T>
using optional = std::experimental::optional<T>;
}

#endif
