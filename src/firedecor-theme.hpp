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

/**
 * Checks if a file exists in storage
 * @param path The path of the file to find
 */
bool exists(std::string path);

/**
 * Gets the real name of a file, dealing with capitalization, for example.
 * @param path The path to be corrected.
 */
std::string get_real_name(std::string path);

/**
 * Gets a vector containing al the .desktop files in a specifi path.
 * @param path The path where the .desktop files will be searched.
 */
std::vector<std::string> get_desktops(std::string path);

/**
 * Gets a value from a .desktop file.
 * @param path The path to the .desktop file.
 * @var The variable to be returned.
 */
std::string get_from_desktop(std::string path, std::string var);

template<typename T>
struct theme_option_t {
  public:
    theme_option_t(T value) : 
        value{value} {}

    T get_value() const {
        return value;
    }

  private:
    T value;  
};

struct theme_options {
  public:
    theme_option_t<std::string> font;
    theme_option_t<int> font_size;
	theme_option_t<wf::color_t> active_title;
	theme_option_t<wf::color_t> inactive_title;

    theme_option_t<std::string> border_size;
    theme_option_t<wf::color_t> active_border;
    theme_option_t<wf::color_t> inactive_border;
    theme_option_t<int> corner_radius;

    theme_option_t<int> outline_size;
    theme_option_t<wf::color_t> active_outline;
    theme_option_t<wf::color_t> inactive_outline;

	theme_option_t<int> button_size;
    theme_option_t<std::string> button_style;
    theme_option_t<bool> inactive_buttons;

	theme_option_t<int> icon_size;
	theme_option_t<std::string> icon_theme;
	
	theme_option_t<int> padding_size;
    theme_option_t<std::string> layout;

	theme_option_t<std::string> ignore_views;
    theme_option_t<bool> debug_mode;
    theme_option_t<std::string> round_on;
};

class decoration_theme_t : private theme_options {
  public:
	decoration_theme_t(theme_options extra_options);

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
	/** @return True if debug_mode is on */
	bool get_debug_mode() const;
	/** @return Where corners should be drawn */
	std::string get_round_on() const;

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

    /**
     * Get the icon for the given button.
     * The caller is responsible for freeing the memory afterwards.
     *
     * @param button The button type.
     * @param state The button state.
     */
    cairo_surface_t *form_button(button_type_t button, double hover,
                                 bool active, bool maximized) const;

	/**
	 * Gets a cairo surface with an svg texture.
	 * @param path The path to said the svg file, must contain .svg at the end.
	 */
	cairo_surface_t *surface_from_svg(std::string path) const;

    /**
     * Get the icon for the given application icon.
     * @param title The icon for the window.
     */
    cairo_surface_t *form_icon(std::string app_id) const;
};
}
}
