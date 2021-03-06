#include <wayfire/singleton-plugin.hpp>
#include <wayfire/view.hpp>
#include <wayfire/matcher.hpp>
#include <wayfire/workspace-manager.hpp>
#include <wayfire/output.hpp>
#include <wayfire/signal-definitions.hpp>

#include "firedecor-subsurface.hpp"

namespace {
struct wayfire_decoration_global_cleanup_t {
    wayfire_decoration_global_cleanup_t() = default;
    ~wayfire_decoration_global_cleanup_t() {
        for (auto view : wf::get_core().get_all_views()) {
            wf::firedecor::deinit_view(view);
        }
    }

    wayfire_decoration_global_cleanup_t(const wayfire_decoration_global_cleanup_t &)
    = delete;
    wayfire_decoration_global_cleanup_t(wayfire_decoration_global_cleanup_t &&) =
    delete;
    wayfire_decoration_global_cleanup_t& operator =(
        const wayfire_decoration_global_cleanup_t&) = delete;
    wayfire_decoration_global_cleanup_t& operator =(
        wayfire_decoration_global_cleanup_t&&) = delete;
};

class wayfire_firedecor_t : 
	public wf::singleton_plugin_t<wayfire_decoration_global_cleanup_t, true> {

    wf::view_matcher_t ignore_views{"firedecor/ignore_views"};
    wf::option_wrapper_t<std::string> extra_themes{"firedecor/extra_themes"};

    wf::signal_connection_t view_updated{ [=] (wf::signal_data_t *data) {
	        update_view_decoration(get_signaled_view(data));
	    }
    };

    wf::config::config_manager_t& config = wf::get_core().config;

  public:

    void init() override {
        grab_interface->name = "firedecor";
        grab_interface->capabilities = wf::CAPABILITY_VIEW_DECORATOR;

        output->connect_signal("view-mapped", &view_updated);
        output->connect_signal("view-decoration-state-updated", &view_updated);
        for (auto& view : output->workspace->get_views_in_layer(wf::ALL_LAYERS)) {
            update_view_decoration(view);
        }
    }

    wf::wl_idle_call idle_deactivate;

    template<typename T>
    T get_option(std::string theme, std::string option_name) {

        auto option = config.get_option<std::string>(theme + "/" + option_name);
        if (option == nullptr || theme == "invalid") {
            return config.get_option<T>("firedecor/" + option_name)->get_value();
        } else {
            return wf::option_type::from_string<T>(option->get_value()).value();
        }
    }

    wf::firedecor::theme_options get_options(std::string theme) {
        wf::firedecor::theme_options options = {
            get_option<std::string>(theme, "font"),
            get_option<int>(theme, "font_size"),
        	get_option<wf::color_t>(theme, "active_title"),
        	get_option<wf::color_t>(theme, "inactive_title"),
        	get_option<int>(theme, "max_title_size"),

            get_option<std::string>(theme, "border_size"),
            get_option<wf::color_t>(theme, "active_border"),
            get_option<wf::color_t>(theme, "inactive_border"),
            get_option<int>(theme, "corner_radius"),

            get_option<int>(theme, "outline_size"),
            get_option<wf::color_t>(theme, "active_outline"),
            get_option<wf::color_t>(theme, "inactive_outline"),

        	get_option<int>(theme, "button_size"),
            get_option<std::string>(theme, "button_style"),
            get_option<wf::color_t>(theme, "normal_min"),
            get_option<wf::color_t>(theme, "hovered_min"),
            get_option<wf::color_t>(theme, "normal_max"),
            get_option<wf::color_t>(theme, "hovered_max"),
            get_option<wf::color_t>(theme, "normal_close"),
            get_option<wf::color_t>(theme, "hovered_close"),
            get_option<bool>(theme, "inactive_buttons"),

        	get_option<int>(theme, "icon_size"),
        	get_option<std::string>(theme, "icon_theme"),

            get_option<wf::color_t>(theme, "active_accent"),
            get_option<wf::color_t>(theme, "inactive_accent"),

        	get_option<int>(theme, "padding_size"),
            get_option<std::string>(theme, "layout"),

        	get_option<std::string>(theme, "ignore_views"),
            get_option<bool>(theme, "debug_mode"),
            get_option<std::string>(theme, "round_on")
        };
        return options;
    }

    void update_view_decoration(wayfire_view view) {
	    if (view->should_be_decorated() && !ignore_views.matches(view)) {
		    if (output->activate_plugin(grab_interface)) {
			    idle_deactivate.run_once([this] () {
				    output->deactivate_plugin(grab_interface);
			    });
    		    std::stringstream themes{extra_themes.value()};
    		    std::string theme;
    		    while (themes >> theme) {
        		    try {
            		    wf::view_matcher_t matcher{theme + "/uses_if"};
            		    if (matcher.matches(view)) {
                		    wf::firedecor::init_view(view, get_options(theme));
                		    return;
            		    }
         		    } catch (...) {
         		    }
    		    }
			    wf::firedecor::init_view(view, get_options("invalid"));
		    }
	    } else {
		    wf::firedecor::deinit_view(view);
	    }
    }

    void fini() override {
        for (auto& view : output->workspace->get_views_in_layer(wf::ALL_LAYERS)) {
            wf::firedecor::deinit_view(view);
        }
    }
};
}

DECLARE_WAYFIRE_PLUGIN(wayfire_firedecor_t);
