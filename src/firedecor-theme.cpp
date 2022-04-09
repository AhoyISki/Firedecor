#include <wayfire/core.hpp>
#include <wayfire/opengl.hpp>
#include <wayfire/config.h>

#include <map>
#include <string>
#include <fstream>
#include <algorithm>

#include "firedecor-theme.hpp"

#include <filesystem>
#include <librsvg/rsvg.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>

namespace wf {
namespace firedecor {
/** Create a new theme with the default parameters */
decoration_theme_t::decoration_theme_t(wf::firedecor::theme_options options) :
    theme_options{options} {}

std::string decoration_theme_t::get_layout() const {
	return layout.get_value();
}

/* Size return functions */
std::string decoration_theme_t::get_border_size() const {
    return border_size.get_value();
}
int decoration_theme_t::get_outline_size() const {
    return outline_size.get_value();
}
int decoration_theme_t::get_font_size()const {
	return font_size.get_value();
}
int decoration_theme_t::get_corner_radius() const {
	return corner_radius.get_value();
}
int decoration_theme_t::get_button_size() const {
	return button_size.get_value();
}
int decoration_theme_t::get_icon_size() const {
	return icon_size.get_value();
}
int decoration_theme_t::get_padding_size() const {
	return padding_size.get_value();
}

/* Color return functions */
color_set_t decoration_theme_t::get_border_colors() const {
	return { active_border.get_value(), inactive_border.get_value() };
}
color_set_t decoration_theme_t::get_outline_colors() const {
	return { active_outline.get_value(), inactive_outline.get_value() };
}
color_set_t decoration_theme_t::get_title_colors() const {
	return { active_title.get_value(), inactive_title.get_value() };
}

/* Other return functions */
bool decoration_theme_t::has_title_orientation(orientation_t orientation) const {
	std::stringstream stream(layout.get_value());
	std::string current_symbol;
	
	edge_t current_edge = EDGE_TOP;

	while (stream >> current_symbol) {
		if (current_symbol == "-") {
	        if (current_edge == EDGE_TOP) {
		        current_edge = EDGE_LEFT;
	        } else if (current_edge == EDGE_LEFT) {
		        current_edge = EDGE_BOTTOM;
	        } else {
		        current_edge = EDGE_RIGHT;
	        }
		} else if (current_symbol == "title") {
			if (((current_edge == EDGE_TOP || current_edge == EDGE_BOTTOM) &&
			    orientation == HORIZONTAL) ||
			    ((current_edge == EDGE_LEFT || current_edge == EDGE_RIGHT) &&
			    orientation == VERTICAL)) {
				    return true;
			}
		}
	}
	return false;
}
bool decoration_theme_t::get_debug_mode() const {
    return debug_mode.get_value();
}
std::string decoration_theme_t::get_round_on() const {
    return round_on.get_value();
}

wf::dimensions_t decoration_theme_t::get_text_size(std::string text, int width) const {
    const auto format = CAIRO_FORMAT_ARGB32;
    auto surface = cairo_image_surface_create(format, width, font_size.get_value());
    auto cr = cairo_create(surface);
	
    PangoFontDescription *font_desc;
    PangoLayout *layout;
    PangoRectangle text_size;

    font_desc = pango_font_description_from_string(((std::string)font.get_value()).c_str());
    pango_font_description_set_absolute_size(font_desc, font_size.get_value() * PANGO_SCALE);
    layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(layout, font_desc);
    pango_layout_set_text(layout, text.c_str(), text.size());
    pango_layout_get_pixel_extents(layout, NULL, &text_size);
    pango_font_description_free(font_desc);
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return { text_size.width, text_size.height };
}

cairo_surface_t*decoration_theme_t::form_title(std::string text,
    wf::dimensions_t title_size, bool active, orientation_t orientation) const {
    const auto format = CAIRO_FORMAT_ARGB32;
    cairo_surface_t* surface;
    if (orientation == HORIZONTAL) {
	    surface = cairo_image_surface_create(
		    format, title_size.width, title_size.height);
    } else {
	    surface = cairo_image_surface_create(
		    format, title_size.height, title_size.width);
    }

    wf::color_t color = (active) ? active_title.get_value() : inactive_title.get_value();

    auto cr = cairo_create(surface);
    if (orientation == VERTICAL) { 
	    double radius = (double)title_size.width / 2;;
	    cairo_translate(cr, radius, radius);
	    cairo_rotate(cr, -M_PI / 2);
	    cairo_translate(cr, -radius, -radius);
    }

    PangoFontDescription *font_desc;
    PangoLayout *layout;
    
    // render text
    font_desc = pango_font_description_from_string(((std::string)font.get_value()).c_str());
    pango_font_description_set_absolute_size(font_desc, font_size.get_value() * PANGO_SCALE);

    layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(layout, font_desc);
    pango_layout_set_text(layout, text.c_str(), text.size());
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    pango_cairo_show_layout(cr, layout);
    pango_font_description_free(font_desc);
    g_object_unref(layout);
    cairo_destroy(cr);

    return surface;
}

cairo_surface_t *decoration_theme_t::form_corner(bool active, double scale) const {
    double corner_radius = this->corner_radius.get_value() * scale;
	double outline_radius = corner_radius - scale * (double)outline_size.get_value() / 2;

    const auto format = CAIRO_FORMAT_ARGB32;
    cairo_surface_t *surface = cairo_image_surface_create(format, corner_radius,
                                                          corner_radius);
    auto cr = cairo_create(surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    /* Border */
	wf::color_t color = active ? active_border.get_value() :
                        inactive_border.get_value();
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    cairo_arc(cr, 0, 0, corner_radius, 0, M_PI / 2);
    cairo_line_to(cr, 0, 0);
    cairo_fill(cr);

    /* Outline */
	color = active ? active_outline.get_value() : inactive_outline.get_value();
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    cairo_set_line_width(cr, outline_size.get_value() * scale);
    cairo_arc(cr, 0, 0, outline_radius, 0, M_PI / 2);
    cairo_stroke(cr);
    cairo_destroy(cr);

    return surface;
}

cairo_surface_t *decoration_theme_t::form_button(button_type_t button, double hover,
                                                 bool active, bool maximized) const {
	if ((std::string)button_style.get_value() != "wayfire" &&
		(std::string)button_style.get_value() != "firedecor" &&
	    (std::string)button_style.get_value() != "minimal") {
		std::string directory = (std::string)getenv("HOME") +
								"/.config/firedecor/button-styles/" +
						   	    (std::string)button_style.get_value() + "/";
		std::string status;
		std::string full_path;

		if (hover == 0.0) {
			if (!active && inactive_buttons.get_value()) {
				status = "-inactive.png";
			} else {
				status = ".png";
			}
		} else if (hover < 0.0) {
			status = "-pressed.png";
		} else {
			status = "-hovered.png";
		}
			
        switch (button) {
          case BUTTON_CLOSE:
	        full_path = directory + "close" + status;
            break;
          case BUTTON_TOGGLE_MAXIMIZE:
	        full_path = directory + "toggle-maximize" + status;
            break;
          case BUTTON_MINIMIZE:
	        full_path = directory + "minimize" + status;
            break;
          default:
            assert(false);
        }
        return surface_from_png(full_path);
	}

	auto border_color = (active) ? active_border.get_value() : inactive_border.get_value();
		
    cairo_surface_t *button_surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, button_size.get_value(), button_size.get_value());

    auto cr = cairo_create(button_surface);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

    /* Clear the button background */
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_rectangle(cr, 0, 0, button_size.get_value(), button_size.get_value());
    cairo_fill(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	color_t base;
    double line;
    double base_qty;

    /** Coloured base on hover/press. Don't compare float to 0 */
    if (fabs(hover) > 1e-3 || (inactive_buttons.get_value() && active)) {
        switch (button) {
          case BUTTON_CLOSE:
            base = { 242.0 / 255.0, 80.0 / 255.0, 86.0 / 255.0, 1.0 };
            break;
          case BUTTON_TOGGLE_MAXIMIZE:
            base = { 57.0 / 255.0, 234.0 / 255.0, 73.0 / 255.0, 1.0 };
            break;
          case BUTTON_MINIMIZE:
            base = { 250.0 / 255.0, 198.0 / 255.0, 54.0 / 255.0, 1.0 };
            break;
          default:
            assert(false);
        }
        line = 0.54;
        base_qty = 0.6;
    } else {
	    base = { 0.40, 0.40, 0.43, 1.0 };
	    line = 0.27;
	    base_qty = 1.0;
    }
	    

    /** Draw the base */
    cairo_set_source_rgba(cr,
        base.r * (base_qty + (1.0 - base_qty) * hover),
        base.g * (base_qty + (1.0 - base_qty) * hover),
        base.b * (base_qty + (1.0 - base_qty) * hover),
        base.a);
    cairo_arc(cr, (double)button_size.get_value() / 2, (double)button_size.get_value() / 2,
              (double)button_size.get_value() / 2, 0, 2 * M_PI);
    cairo_fill(cr);

    /** Draw the border */
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, border_color.r, border_color.g, border_color.b, line / 2);
    double r = (double)button_size.get_value() / 2 - 0.5 * 1.0;
    cairo_arc(cr, (double)button_size.get_value() / 2, (double)button_size.get_value() / 2, r, 0, 2 * M_PI);
    cairo_stroke(cr);

    /** Draw the icon  */
    cairo_set_source_rgba(cr, 0.00, 0.00, 0.00, line / 2);
    //cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    if (hover != 0) {
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    }
    if ((std::string)button_style.get_value() == "wayfire") {
	    switch (button) {
	      case BUTTON_CLOSE:
	        cairo_set_line_width(cr, 1.5 * 1.0);
	        cairo_move_to(cr, 1.0 * button_size.get_value() / 4.0, 1.0 * button_size.get_value() / 4.0);
	        cairo_line_to(cr, 3.0 * button_size.get_value() / 4.0, 3.0 * button_size.get_value() / 4.0);
	        cairo_move_to(cr, 3.0 * button_size.get_value() / 4.0, 1.0 * button_size.get_value() / 4.0);
	        cairo_line_to(cr, 1.0 * button_size.get_value() / 4.0, 3.0 * button_size.get_value() / 4.0);
	        cairo_stroke(cr);
	        break;
	      case BUTTON_TOGGLE_MAXIMIZE:
	        cairo_set_line_width(cr, 1.5 * 1.0);
	        cairo_rectangle(cr, button_size.get_value() / 4.0, button_size.get_value() / 4.0,
	            			button_size.get_value() / 2.0, button_size.get_value() / 2.0);
	        cairo_stroke(cr);
	        break;
	      case BUTTON_MINIMIZE:
	        cairo_set_line_width(cr, 1.75);
	        cairo_move_to(cr, 1.0 * button_size.get_value() / 4.0, button_size.get_value() / 2.0);
	        cairo_line_to(cr, 3.0 * button_size.get_value() / 4.0, button_size.get_value() / 2.0);
	        cairo_stroke(cr);
	        break;
	      default:
	        assert(false);
	    }
    } else if ((std::string)button_style.get_value() == "firedecor") {
	    switch (button) {
		  case BUTTON_CLOSE:
			cairo_set_line_width(cr, 1.5);
			/* Line from top left to bottom right */
			cairo_move_to(cr, (double)button_size.get_value() / 2, (double)button_size.get_value() / 2);
			cairo_rel_move_to(cr, -0.25 * button_size.get_value() * hover,
			                  -0.25 * button_size.get_value() * hover);
	        cairo_rel_line_to(cr, 0.5 * button_size.get_value() * hover,
	                      	  0.5 * button_size.get_value() * hover);
	        /* Line from bottom left to top right */
			cairo_move_to(cr, (double)button_size.get_value() / 2, (double)button_size.get_value() / 2);
			cairo_rel_move_to(cr, -0.25 * button_size.get_value() * hover,
			                  0.25 * button_size.get_value() * hover);
	        cairo_rel_line_to(cr, 0.5 * button_size.get_value() * hover,
	                      	  -0.5 * button_size.get_value() * hover);
	        cairo_stroke(cr);
	        break;
	      case BUTTON_TOGGLE_MAXIMIZE:
			cairo_set_line_width(cr, 1.5);
		    wf::pointf_t north_east_arrow_pos, south_west_arrow_pos;
		    if (maximized) {
			    north_east_arrow_pos = { 0.28 * button_size.get_value(), 0.72 * button_size.get_value() };
			    south_west_arrow_pos = { 0.72 * button_size.get_value(), 0.28 * button_size.get_value() };
		    } else {
			    north_east_arrow_pos = { 0.563 * button_size.get_value(), 0.437 * button_size.get_value() };
			    south_west_arrow_pos = { 0.437 * button_size.get_value(), 0.563 * button_size.get_value() };
		    }
			    
		    /* Top right arrow */
			cairo_move_to(cr, north_east_arrow_pos.x, north_east_arrow_pos.y);
			cairo_rel_move_to(cr, -0.175 * button_size.get_value() * hover,
			                  -0.175 * button_size.get_value() * hover);
			cairo_rel_line_to(cr, 0.35 * button_size.get_value() * hover, 0);
			cairo_rel_line_to(cr, 0, 0.35 * button_size.get_value() * hover);
			cairo_stroke(cr);
			/* Bottom left arrow */
			cairo_move_to(cr, south_west_arrow_pos.x, south_west_arrow_pos.y);
			cairo_rel_move_to(cr, -0.175 * button_size.get_value() * hover,
			                  -0.175 * button_size.get_value() * hover);
			cairo_rel_line_to(cr, 0, 0.35 * button_size.get_value() * hover);
			cairo_rel_line_to(cr, 0.35 * button_size.get_value() * hover, 0);
			cairo_stroke(cr);
			break;
		  case BUTTON_MINIMIZE:
	        cairo_set_line_width(cr, 2.0);
			cairo_move_to(cr, (double)button_size.get_value() / 2, (double)button_size.get_value() / 2);
			cairo_rel_move_to(cr, -0.25 * button_size.get_value() * hover, 0);
			cairo_rel_line_to(cr, 0.5 * button_size.get_value() * hover, 0);
			cairo_stroke(cr);
			break;
	    }
    }

    cairo_fill(cr);
    cairo_destroy(cr);

    return button_surface;
}

bool exists(std::string path) {
    if (path.back() == '/') {
        return std::filesystem::exists(path);
    }

    std::string path_head = path.substr(0, path.rfind('/'));

    if (!std::filesystem::exists(path_head)) {
        return false;
    }

    for (auto& dir_entry : std::filesystem::directory_iterator(path_head)) {
        if (boost::iequals(path, (std::string)dir_entry.path())) {
            return true;
        }
    }

    return false;
}

std::string get_real_name(std::string path) {
    std::string path_head = path.substr(0, path.rfind('/'));

    for (auto& dir_entry : std::filesystem::directory_iterator(path_head)) {
        if (boost::iequals(path, (std::string)dir_entry.path())) {
            return (std::string)dir_entry.path();
        }
    }

    return path;
}

cairo_surface_t *decoration_theme_t::surface_from_svg(std::string path) const {
	GFile *svg_file = g_file_new_for_path(path.c_str());
	RsvgHandle *svg = rsvg_handle_new_from_gfile_sync(svg_file, RSVG_HANDLE_FLAGS_NONE,
	                                                  NULL, NULL);
	cairo_surface_t *icon = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, icon_size.get_value(),
	                                                   icon_size.get_value());
	auto crsvg = cairo_create(icon);
	RsvgRectangle rect { 0, 0, (double)icon_size.get_value(), (double)icon_size.get_value() };
	rsvg_handle_render_document(svg, crsvg, &rect, nullptr);
	cairo_destroy(crsvg);

	g_object_unref(svg);
	g_object_unref(svg_file);

	return icon;
}

cairo_surface_t * decoration_theme_t::surface_from_png(std::string path) const {
    double len = icon_size.get_value();
    auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, len, len);
    auto cr = cairo_create(surface);

    auto image = cairo_image_surface_create_from_png(path.c_str());
    double width  = cairo_image_surface_get_width(image);
    double height = cairo_image_surface_get_height(image);

    cairo_translate(cr, len / 2, len / 2);
    cairo_scale(cr, len / width, len / height);
    cairo_translate(cr, -len / 2, -len / 2);

    cairo_set_source_surface(cr, image, (len - width) / 2, (len - height) / 2);
    cairo_paint(cr);
    cairo_surface_destroy(image);
    cairo_destroy(cr);

    return surface;
}

std::vector<std::string> get_desktops( std::string path ) {
    std::vector<std::string> desktops;

    if ( not exists( path ) ) {
        return desktops;
    }

    for (const auto& entry : std::filesystem::directory_iterator( path ) ) {
        if ( entry.is_regular_file() and entry.path().extension() == ".desktop" ) {
            desktops.push_back( entry.path() );
        }
    }

    return desktops;
}

std::string get_from_desktop(std::string path, std::string var) {
	std::ifstream input_file(path);
	std::string line;
	while(std::getline(input_file, line)) {
		if (auto index = line.find(var); index != std::string::npos) {
			return (line.substr(index + var.length() + 1));
		}
	}
	if (var == "Icon") {
		return "application-x-executable";
	} else {
		return "This_name_is_not_supposed_to_work.1234lol";
	}
}

cairo_surface_t *decoration_theme_t::form_icon(std::string app_id) const {
	std::string line;
	std::string icons = (std::string)getenv("HOME") + "/.local/share/firedecor_icons";
	std::ofstream icon_file_out(icons, std::ofstream::out | std::ofstream::app);

	while (true) {
    	/** 
         * First, check if the icon has already been found,
         * this will be true the vast majority of the time,
         * drastically improving speed.
         */
    	std::ifstream icon_file_in(icons);
		while (std::getline(icon_file_in, line)) {
			if (line.find(app_id + " ") == 0) {
				std::string path = line.substr(line.find(' ') + 1);
				if (line.rfind(".svg") != std::string::npos) {
					return surface_from_svg(path);
				} else if (line.rfind(".png") != std::string::npos) {
    				return surface_from_png(path);
				}
			}
		}
		icon_file_in.close();

		std::string icon_name = (std::string)getenv("HOME") +
							    ".config/firedecor/executable.svg";
		bool found = false;

		std::vector<std::string> app_dirs = {
			(std::string)getenv("HOME") + "/.local/share/applications/",
			"/usr/local/share/applications/",
			"/usr/share/applications/"
		};

        /** Helpful specific case for some steam games */
		if (app_id.substr(0, 10) == "steam_app_") {
    		icon_name = "steam_icon_" + app_id.substr(10);
    		found = true;
		}

		for (auto path : app_dirs) {
    		if (found) { break; }
			if (auto icon_path = path + app_id + ".desktop"; exists(icon_path)) {
    			/* This definition is here mostly to filter for capitalized names */
    			icon_path = get_real_name(icon_path);
				icon_name = get_from_desktop(icon_path, "Icon");
				found = true;
				break;
			}
		}

		if (!found) {
			for (auto path : app_dirs) {
				std::vector<std::string> desktops = get_desktops(path);
				for (auto desktop : desktops) {
					/* Check the executable name */
					std::stringstream ess(get_from_desktop(desktop, "Exec"));
					std::string exec;
					ess >> exec;

					if (boost::iequals((std::string)std::filesystem::path(exec)
					                   .filename(), app_id)) {
						icon_name = get_from_desktop(desktop, "Icon");
						break;
					}

					/* Check the name */
					std::stringstream nss(get_from_desktop(desktop, "Name"));
					std::string name;
					nss >> name;
					if (boost::iequals(name, app_id)) {
						icon_name = get_from_desktop(desktop, "Icon");
						break;
					}

					/* Check StartupWMClass for electron apps */
					std::stringstream sss(get_from_desktop(desktop,
														   "StartupWMClas"));
					std::string startupWMClass;
					sss >> startupWMClass;
					if (boost::iequals(startupWMClass, app_id)) {
						icon_name = get_from_desktop(desktop, "Icon");
						break;
					}
				}
			}
		}

        /** Case for absolute paths */
		if ((icon_name.at(0) == '/') && exists(icon_name)) {
			 icon_file_out << app_id + " " + icon_name << std::endl;
			 continue;
		}

		std::vector<std::string> icon_names;

		icon_names.push_back(icon_name);
		if (auto name = boost::to_lower_copy(icon_name);
		    name != icon_name) {
    		icon_names.push_back(name);
		}
		icon_names.push_back(app_id);
		if (auto name = boost::to_lower_copy(app_id);
		    name != app_id) {
    		icon_names.push_back(name);
		}

		std::vector<std::string> icon_themes;

		/* Locations where the icon_theme.get_value() would be expected to be */
		std::vector<std::string> icon_dirs = {
    		(std::string)getenv("HOME") + "/.local/share/icons/",
    		"/usr/local/share/icons/",
    		"/usr/share/icons/"
		};

		std::vector<std::string> default_icon_themes = {
    		"/usr/share/icons/hicolor/",
    		"/usr/share/icons/Adwaita/",
    		"/usr/share/icons/breeze/"
		};

		/* Check for the existance of the icon_theme.get_value() on all reasonable locations */
		for (auto icon_dir : icon_dirs) {
    		if (auto dir = icon_dir + (std::string)icon_theme.get_value();
    		    std::count(default_icon_themes.begin(), default_icon_themes.end(),
    		               dir) == 0) {
        		if (exists(dir)) {
            		icon_themes.push_back(dir + "/"); 
        		}
    		}
		}

		for (auto icon_theme : default_icon_themes) {
    		if (exists(icon_theme)) {
        		icon_themes.push_back(icon_theme);
    		}
		}

        /** Look for the icons on every folder, on every resolution */
		bool icon_found = false;
		for (auto path : icon_themes) {
    		for (auto res : { 
        			"scalable/", "32x32/", "48x48/", "72x72/", "96x96/",
    				"128x128/", "256x256/"
    		}) {
        		for (auto icon_name : icon_names) {
            		for (auto e : { ".svg", ".exe" }) {
                		if (auto icon_path = path + res + "apps/" + icon_name + e;
                			exists(icon_path)) {
                    		icon_found = true;
                    		icon_file_out << app_id + " " + icon_path << std::endl;
                    		break;
                		} 
            		}
        		}
        		if (icon_found) { break; }
    		}
    		if (icon_found) { break; }
		}

        /** Absolute last resorts */
		for (auto icon_name : icon_names) {
    		for (auto e : { ".svg", ".exe" }) {
        		if (auto icon_path = "/usr/share/pixmaps" + icon_name + e;
        			exists(icon_path)) {
            		icon_found = true;
            		icon_file_out << app_id + " " + icon_path << std::endl;
            		break;
        		} 
    		}
		}
    		

		if (!icon_found) {
			icon_file_out << app_id + " " + (std::string)getenv("HOME") +
						   "/.config/firedecor/executable.svg" << std::endl;
			return surface_from_svg((std::string)getenv("HOME") +
						   			"/.config/firedecor/executable.svg");
		}
	}
}
}
}
