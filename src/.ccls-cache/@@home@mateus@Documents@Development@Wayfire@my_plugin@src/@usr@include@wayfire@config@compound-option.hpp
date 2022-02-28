#pragma once

#include <wayfire/config/types.hpp>
#include <wayfire/config/option.hpp>
#include <wayfire/util/log.hpp>
#include <vector>
#include <map>
#include <cassert>

namespace wf
{
namespace config
{
template<class... Args>
using compound_list_t =
    std::vector<std::tuple<std::string, Args...>>;

/**
 * A base class containing information about an entry in a tuple.
 */
class compound_option_entry_base_t
{
  public:

    virtual ~compound_option_entry_base_t() = default;

    /** @return The prefix of the tuple entry. */
    virtual std::string get_prefix() const
    {
        return prefix;
    }

    /**
     * Try to parse the given value.
     *
     * @param update Whether to override the stored value.
     */
    virtual bool is_parsable(const std::string&) const = 0;

    /** Clone this entry */
    virtual compound_option_entry_base_t *clone() const = 0;

  protected:
    compound_option_entry_base_t() = default;
    std::string prefix;
};

template<class Type>
class compound_option_entry_t : public compound_option_entry_base_t
{
  public:
    compound_option_entry_t(const std::string& prefix)
    {
        this->prefix = prefix;
    }

    compound_option_entry_base_t *clone() const override
    {
        return new compound_option_entry_t<Type>(this->get_prefix());
    }

    /**
     * Try to parse the given value.
     *
     * @param update Whether to override the stored value.
     */
    bool is_parsable(const std::string& str) const override
    {
        return option_type::from_string<Type>(str).has_value();
    }
};

/**
 * Compound options are a special class of options which can hold multiple
 * string-tagged tuples. They are constructed from multiple untyped options
 * in the config file.
 */

class compound_option_t : public option_base_t
{
  public:
    using entries_t = std::vector<std::unique_ptr<compound_option_entry_base_t>>;
    /**
     * Construct a new compound option, with the types given in the template
     * arguments.
     *
     * @param name The name of the option.
     * @param prefixes The prefixes used for grouping in the config file.
     *   Example: Consider a compound option with type <int, double> and two
     *   prefixes {"prefix1_", "prefix2_"}. In the config file, the options are:
     *
     *   prefix1_key1 = v11
     *   prefix2_key1 = v21
     *   prefix1_key2 = v12
     *   prefix2_key2 = v22
     *
     *   Options are grouped by suffixes (key1 and key2), and the tuples then
     *   are formed by taking the values of the options with each prefix.
     *   So, the tuples contained in the compound option in the end are:
     *
     *   (key1, v11, v21)
     *   (key2, v12, v22)
     */
    compound_option_t(const std::string& name, entries_t&& entries);

    /**
     * Parse the compound option with the given types.
     *
     * Throws an exception in case of wrong template types.
     */
    template<class... Args>
    compound_list_t<Args...> get_value() const
    {
        compound_list_t<Args...> result;
        result.resize(value.size());
        build_recursive<0, Args...>(result);
        return result;
    }

    /**
     * Set the value of the option.
     *
     * Throws an exception in case of wrong template types.
     */
    template<class... Args>
    void set_value(const compound_list_t<Args...>& value)
    {
        assert(sizeof...(Args) == this->entries.size());
        this->value.assign(value.size(), {});
        push_recursive<0>(value);
        notify_updated();
    }

    using stored_type_t = std::vector<std::vector<std::string>>;
    /**
     * Get the string data stored in the compound option.
     */
    stored_type_t get_value_untyped();

    /**
     * Set the data contained in the option, from a vector containing
     * strings which describe the individual elements.
     *
     * @return True if the operation was successful.
     */
    bool set_value_untyped(stored_type_t value);

    /**
     * Get the type information about entries in the option.
     */
    const entries_t& get_entries() const;

  private:
    /**
     * Current value stored in the option.
     * The first element is the name of the tuple, followed by the string values
     * of each element.
     */
    stored_type_t value;

    /** Entry types with which the option was created. */
    entries_t entries;

    /**
     * Set the n-th element in the result tuples by reading from the stored
     * values in this option.
     */
    template<size_t n, class... Args>
    void build_recursive(compound_list_t<Args...>& result) const
    {
        for (size_t i = 0; i < result.size(); i++)
        {
            using type_t = typename std::tuple_element<n,
                std::tuple<std::string, Args...>>::type;

            std::get<n>(result[i]) = option_type::from_string<type_t>(
                this->value[i][n]).value();
        }

        // Recursively build the (N+1)'th entries
        if constexpr (n < sizeof...(Args))
        {
            build_recursive<n + 1>(result);
        }
    }

    template<size_t n, class... Args>
    void push_recursive(const compound_list_t<Args...>& new_value)
    {
        for (size_t i = 0; i < new_value.size(); i++)
        {
            using type_t = typename std::tuple_element<n,
                std::tuple<std::string, Args...>>::type;

            this->value[i].push_back(option_type::to_string<type_t>(
                std::get<n>(new_value[i])));
        }

        // Recursively build the (N+1)'th entries
        if constexpr (n < sizeof...(Args))
        {
            push_recursive<n + 1>(new_value);
        }
    }

  public: // Implementation of option_base_t
    std::shared_ptr<option_base_t> clone_option() const override;
    bool set_value_str(const std::string&) override;
    void reset_to_default() override;
    bool set_default_value_str(const std::string&) override;
    std::string get_value_str() const override;
    std::string get_default_value_str() const override;
};
}
}
