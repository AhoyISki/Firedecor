#pragma once

#include <wayfire/nonstd/optional.hpp>
#include <string>

namespace wf
{
namespace option_type
{
/**
 * To create an option of a given type, from_string must be specialized for
 * parsing the type.
 *
 * @param string The string representation of the value.
 * @return The parsed value, if the string was valid.
 */
template<class Type>
stdx::optional<Type> from_string(
    const std::string& string);

/**
 * To create an option of a given type, to_string must be specialized for
 * converting the type to string.
 * @return The string representation of a value.
 *   It is expected that from_string(to_string(value)) == value.
 */
template<class Type>
std::string to_string(const Type& value);
}
}
