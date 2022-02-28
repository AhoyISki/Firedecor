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

    /**
     * Fill the given rectangle with the background color(s).
     *
     * @param fb The target framebuffer, must have been bound already.
     * @param rectangle The rectangle to redraw.
     * @param scissor The GL scissor rectangle to use.
     * @param active Whether to use active or inactive colors
     */
    void render_rectangles(const wf::framebuffer_t& fb, wf::geometry_t rectangle, const wf::geometry_t& scissor, bool active) const;

    /**
     * Render the given text on a cairo_surface_t with the given size.
     * The caller is responsible for freeing the memory afterwards.
     */
    cairo_surface_t *form_text(std::string text, int width, int height) const;

    /**
     * Render one corner for active and inactive windows. 
     * It will be used for all 4 corners of the decoration.
     */
    cairo_surface_t *form_corner(bool active) const;

    struct edge_colors_t {
	    color_t active_color;
	    color_t inactive_color;
	    color_t active_outline_color;
	    color_t inactive_outline_color;
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
	wf::option_wrapper_t<wf::color_t> active_title_color{"firedecor/active_title_color"};
	wf::option_wrapper_t<wf::color_t> inactive_title_color{"firedecor/inactive_title_color"};
	wf::option_wrapper_t<bool> show_application_icon{"firedecor/show_application_icon"};

    wf::option_wrapper_t<int> titlebar_height{"firedecor/titlebar_height"};
    wf::option_wrapper_t<std::string> titlebar_position{"firedecor/titlebar_position"};

    wf::option_wrapper_t<int> border_size{"firedecor/border_size"};
    wf::option_wrapper_t<wf::color_t> active_color{"firedecor/active_color"};
    wf::option_wrapper_t<wf::color_t> inactive_color{"firedecor/inactive_color"};
    wf::option_wrapper_t<int> corner_radius{"firedecor/corner_radius"};
    wf::option_wrapper_t<bool> pad_corner_radius{"firedecor/pad_corner_radius"};

    wf::option_wrapper_t<int> outline_size{"firedecor/outline_size"};
    wf::option_wrapper_t<wf::color_t> active_outline_color{"firedecor/active_outline_color"};
    wf::option_wrapper_t<wf::color_t> inactive_outline_color{"firedecor/inactive_outline_color"};

    wf::option_wrapper_t<std::string> button_order{"firedecor/button_order"};
    wf::option_wrapper_t<std::string> button_style{"firedecor/button_style"};
    wf::option_wrapper_t<bool> inactive_buttons{"firedecor/inactive_buttons"};
};
}
}
