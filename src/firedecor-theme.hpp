#pragma once
#include <wayfire/render-manager.hpp>

#include "firedecor-buttons.hpp"

namespace wf {
namespace firedecor {

struct color_set_t {
	color_t active, inactive;

	bool operator ==(const color_set_t& other) const {
		return (this->active == other.active) && (this->inactive == other.inactive);
	};

	bool operator !=(const color_set_t& other) const {
		return !(this->active == other.active) || !(this->inactive == other.inactive);
	};
};

enum orientation_t {
	HORIZONTAL = 0,
	VERTICAL = 1
};

enum edge_t {
	EDGE_TOP    = 0,
	EDGE_LEFT   = 1,
	EDGE_BOTTOM = 2,
	EDGE_RIGHT  = 3
};

class decoration_theme_t {
  public:
	decoration_theme_t();

	/** @return The theme's layout */
	std::string get_layout() const;

	/* Size return functions */
    /** @return The available border for resizing */
    std::string get_border_size() const;
    /** @return The font size */
    int get_font_size() const;
	/** @return The available outline for resizing */
	int get_outline_size() const;
    /** @return The corner radius */
    int get_corner_radius() const;
    /** @return The equal width and height of the button */
    int get_button_size() const;
    /** @return The icon size */
    int get_icon_size() const;
    /** @return The padding size */
    int get_padding_size() const;

	/* Color return functions */
	/** @return The active and inactive colors for the border */
	color_set_t get_border_colors() const;
	/** @return The active and inactive colors for the outline */
	color_set_t get_outline_colors() const;
	/** @return The acntive and inactive colors for the title */
	color_set_t get_title_colors() const;

	/* Other return functions */
	/** @return True if there is a title of said orientation in the layout */
	bool has_title_orientation(orientation_t orientation) const;

	/**
     * Get what the title size should be, given a text for the title, useful for
     * centered and right positioned layouts on an edge.
     */
	wf::dimensions_t get_text_size(std::string title, int width) const;

    /**
     * Render the given text on a cairo_surface_t with the given size.
     * The caller is responsible for freeing the memory afterwards.
     */
    cairo_surface_t *form_title(std::string text, wf::dimensions_t title_size,
                                bool active, orientation_t orientation) const;

    /**
     * Render one corner for active and inactive windows. 
     * It will be used for all 4 corners of the decoration.
     */
    cairo_surface_t *form_corner(bool active) const;

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
                                        const button_state_t& state,
                                        bool active) const;

  private:
    wf::option_wrapper_t<std::string> font{"firedecor/font"};
    wf::option_wrapper_t<int> font_size{"firedecor/font_size"};
	wf::option_wrapper_t<wf::color_t> active_title{"firedecor/active_title"};
	wf::option_wrapper_t<wf::color_t> inactive_title{"firedecor/inactive_title"};

    wf::option_wrapper_t<std::string> border_size{"firedecor/border_size"};
    wf::option_wrapper_t<wf::color_t> active_border{"firedecor/active_border"};
    wf::option_wrapper_t<wf::color_t> inactive_border{"firedecor/inactive_border"};
    wf::option_wrapper_t<int> corner_radius{"firedecor/corner_radius"};

    wf::option_wrapper_t<int> outline_size{"firedecor/outline_size"};
    wf::option_wrapper_t<wf::color_t> active_outline{"firedecor/active_outline"};
    wf::option_wrapper_t<wf::color_t> inactive_outline{"firedecor/inactive_outline"};

	wf::option_wrapper_t<int> button_size{"firedecor/button_size"};
    wf::option_wrapper_t<std::string> button_style{"firedecor/button_style"};
    wf::option_wrapper_t<bool> inactive_buttons{"firedecor/inactive_buttons"};

	wf::option_wrapper_t<int> icon_size{"firedecor/icon_size"};
	wf::option_wrapper_t<int> padding_size{"firedecor/padding_size"};
	wf::option_wrapper_t<std::string> ignore_views{"firedecor/ignore_views"};
    wf::option_wrapper_t<std::string> layout{"firedecor/layout"};
};
}
}
