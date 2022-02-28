#pragma once

#include <memory>
#include <vector>
#include <wayfire/config/option.hpp>

namespace wf
{
namespace config
{
/**
 * Represents a section in the config file.
 * Each section has a list of options.
 */
class section_t
{
  public:
    /**
     * Create a new empty section.
     *
     * The section name can be arbitrary, however when reading config from a
     * file there are additional restrictions.
     */
    section_t(const std::string& name);
    virtual ~section_t();

    /** @return The name of the config section. */
    std::string get_name() const;

    /** @return A deep copy of the config section with a new name. */
    std::shared_ptr<section_t> clone_with_name(const std::string name) const;

    /**
     * @return The option with the given name, or nullptr if no such option
     * has been added yet.
     */
    std::shared_ptr<option_base_t> get_option_or(const std::string& name);

    /**
     * @return The option with the given name.
     * @throws std::invalid_argument if the option hasn't been added.
     */
    std::shared_ptr<option_base_t> get_option(const std::string& name);

    using option_list_t = std::vector<std::shared_ptr<option_base_t>>;
    /**
     * @return A list of all available options in this config section.
     */
    option_list_t get_registered_options() const;

    /**
     * Register a new option, which means it is marked as belonging to this
     * section and it will show up in the list of get_registered_options().
     *
     * If an option with the same name already exists, it will be overwritten.
     */
    void register_new_option(std::shared_ptr<option_base_t> option);

    /**
     * Remove an option from the registered options in this section.
     * No-op if the option is not part of the section, or if option is null.
     */
    void unregister_option(std::shared_ptr<option_base_t> option);

    struct impl;
    std::unique_ptr<impl> priv;
};
}
}
