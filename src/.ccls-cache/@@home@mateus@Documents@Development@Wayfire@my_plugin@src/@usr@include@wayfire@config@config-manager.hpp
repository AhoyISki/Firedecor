#pragma once

#include <wayfire/config/section.hpp>

namespace wf
{
namespace config
{
/**
 * Manages the whole configuration of a program.
 * The configuration consists of a list of sections with their options.
 */
class config_manager_t
{
  public:
    /**
     * Add the given config section to the configuration.
     * If the section already exists, each new option will be added to the
     * existing section, and already existing options will be overwritten.
     *
     * @param section The section to add, must be non-null.
     */
    void merge_section(std::shared_ptr<section_t> section);

    /**
     * Find the configuration section with the given name.
     * @return nullptr if the section doesn't exist.
     */
    std::shared_ptr<section_t> get_section(const std::string& name) const;

    /**
     * @return A list of all sections currently in the config manager.
     */
    std::vector<std::shared_ptr<section_t>> get_all_sections() const;

    /**
     * Get the option with the given name.
     * The name consists of the name of the option section, followed by a '/',
     * then followed by the actual name of the option in the option section,
     * for example 'core/plugins'.
     *
     * If the option doesn't exist, nullptr is returned.
     */
    std::shared_ptr<option_base_t> get_option(const std::string& name) const;

    /**
     * Get the option with the given name. Same semantics as
     * get_option(std::string), but casts the result to the appropriate type.
     */
    template<class T>
    std::shared_ptr<option_t<T>> get_option(const std::string& name) const
    {
        return std::dynamic_pointer_cast<option_t<T>>(get_option(name));
    }

    config_manager_t();
    config_manager_t(config_manager_t&& other);
    config_manager_t& operator =(config_manager_t&& other);

    virtual ~config_manager_t();

  private:
    struct impl;
    std::unique_ptr<impl> priv;
};
}
}
