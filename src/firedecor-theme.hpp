#pragma once
#include <wayfire/render-manager.hpp>

#include "firedecor-buttons.hpp"

namespace wf {
namespace firedecor {
class decoration_theme_t {
  public:
	decoration_theme_t();

    /** @return The available height for displaying the title */
    int get_titlebar_height() const;
    /** @return The available border for resizing */
    int get_border_size() const;
    /** @return The corner radius */
    int get_corner_radius() const;
	/** @return The available outline for resizing */
	int get_outline_size() const;

    /**
     * Render the given text on a cairo_surface_t with the given size.
     * The caller is responsible for freeing the memory afterwards.
     */
    cairo_surface_t *form_text(std::string text, int width,
        int height, bool active) const;

    /**
     * Render one corner for active and inactive windows. 
     * It will be used for all 4 corners of the decoration.
     */
    cairo_surface_t *form_corner(bool active) const;

    struct edge_colors_t {
	    color_t active_border;
	    color_t inactive_border;
	    color_t active_outline;
	    color_t inactive_outline;
    };

    /** @return The updated color list for the edges of the decoration */
    edge_colors_t get_edge_colors() const;

    struct button_state_t {
        /** Button width */
        double width;
        /** Button height */
        double height;
        /** Button outline size */
        double border;
        /** Progress of button hover, in range [-1, 1].
         * Negative numbers are usually used for pressed state. */
        double hover_progress;
    };

    /**
     * Get the icon for the given button.
     * The caller is responsible for freeing the memory afterwards.
     *
     * @param button The button type.
     * @param state The button state.
     */
    cairo_surface_t *get_button_surface(button_type_t button,
        const button_state_t& state) const;

  private:
    wf::option_wrapper_t<std::string> font{"firedecor/font"};
    wf::option_wrapper_t<int> font_size{"firedecor/font_size"};
	wf::option_wrapper_t<wf::color_t> active_title{"firedecor/active_title"};
	wf::option_wrapper_t<wf::color_t> inactive_title{"firedecor/inactive_title"};
	wf::option_wrapper_t<bool> show_title_icon{"firedecor/show_title_icon"};

    wf::option_wrapper_t<int> titlebar_height{"firedecor/titlebar_height"};
    wf::option_wrapper_t<std::string> titlebar_position{"firedecor/titlebar_position"};

    wf::option_wrapper_t<int> border_size{"firedecor/border_size"};
    wf::option_wrapper_t<wf::color_t> active_border{"firedecor/active_border"};
    wf::option_wrapper_t<wf::color_t> inactive_border{"firedecor/inactive_border"};
    wf::option_wrapper_t<int> corner_radius{"firedecor/corner_radius"};

    wf::option_wrapper_t<int> outline_size{"firedecor/outline_size"};
    wf::option_wrapper_t<wf::color_t> active_outline{"firedecor/active_outline"};
    wf::option_wrapper_t<wf::color_t> inactive_outline{"firedecor/inactive_outline"};

    wf::option_wrapper_t<std::string> button_order{"firedecor/button_order"};
    wf::option_wrapper_t<std::string> button_style{"firedecor/button_style"};
    wf::option_wrapper_t<bool> inactive_buttons{"firedecor/inactive_buttons"};
    /* Eventually */
    // wf::option_wrapper_t<std::string> layout{"firedecor/layout"};
};
}
}
