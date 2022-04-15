#include <wayfire/core.hpp>
#include <wayfire/nonstd/reverse.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>
#include <wayfire/util.hpp>

#include "firedecor-layout.hpp"
#include "firedecor-theme.hpp"

#include <fstream>

namespace wf {
namespace firedecor {
/** Initialize a new decoration area holding a title or icon */
decoration_area_t::decoration_area_t(decoration_area_type_t type, wf::geometry_t g,
                                     edge_t edge) {
    this->type     = type;
    this->geometry = g;
    this->edge     = edge;
}

/** Initialize a new decoration area holding a button */
decoration_area_t::decoration_area_t(wf::geometry_t g,
                                     std::function<void(wlr_box)> damage_callback,
                                	 const decoration_theme_t& theme) {
    this->type     = DECORATION_AREA_BUTTON;
    this->geometry = g;

    this->button = std::make_unique<button_t>(theme, std::bind(damage_callback, g));
}

/** Initialize a new decoration area where the edge does not matter */
decoration_area_t::decoration_area_t(decoration_area_type_t type, wf::geometry_t g) {
    this->type     = type;
    this->geometry = g;
}

decoration_area_t::decoration_area_t(decoration_area_type_t type, wf::geometry_t g,
                                     std::string c) {
    this->type = type;
    this->geometry = g;
    this->corners = c;
}

decoration_area_type_t decoration_area_t::get_type() const {
    return type;
}

wf::geometry_t decoration_area_t::get_geometry() const {
    return geometry;
}

edge_t decoration_area_t::get_edge() const {
	return edge;
}

std::string decoration_area_t::get_corners() const {
    return corners;
}

button_t& decoration_area_t::as_button() {
    assert(button);

    return *button;
}

border_size_t decoration_layout_t::parse_border(const std::string border_size_str) {
	std::stringstream stream((std::string)border_size_str);
	int current_size;
	int border_size[4];
	int indices = 0;

	while (stream >> current_size) {
		border_size[indices] = current_size;
		indices++;
	}

	if (indices == 1 || indices == 3) {
		return { border_size[0], border_size[0], border_size[0], border_size[0] };
	} else if (indices == 2) {
		return { border_size[0], border_size[1], border_size[1], border_size[1] };
	} else {
		return { border_size[0], border_size[1], border_size[2], border_size[3] };
	}
}

decoration_layout_t::decoration_layout_t(const decoration_theme_t& theme,
    std::function<void(wlr_box)> callback) :

	layout(theme.get_layout()),
	border_size_str(theme.get_border_size()),
	border_size(parse_border(border_size_str)),
	corner_radius(theme.get_corner_radius()),
	outline_size(theme.get_outline_size()),
	button_size(theme.get_button_size()),
	icon_size(theme.get_icon_size()),
	padding_size(theme.get_padding_size()),
    theme(theme),
    damage_callback(callback)
	{}

void decoration_layout_t::create_areas(int width, int height,
                                       wf::dimensions_t title_size) {
    int count = std::count(layout.begin(), layout.end(), '-');
    std::string layout_str = layout;
    for (int i = 4; i > count; i--) {
        layout_str.append(" -");
    }
    std::stringstream stream(layout_str);
    std::vector<std::string> left, center, right;
    std::string current_symbol;

    edge_t current_edge = EDGE_TOP;
    std::string current_position = "left";
    wf::point_t o = { 0, border_size.top - max_height };

	/** The values that are used to determine the updated geometry for areas */
    int shift = 0, out_padding = 0;

    /**** Elements that can be transformed to work on any edge */
    auto p = [&]() -> wf::point_t { return { shift, out_padding }; };
    const wf::point_t &l = { width, height - border_size.top - border_size.bottom };
    const wf::point_t &title = { title_size.width, title_size.height };

    /** Matrix that transforms said elements */
	struct { int x1 = 1, y1 = 0 , x2 = 0, y2 = 1; } m;

    /** Transformation lambda */
	auto trans = [&](wf::point_t v) -> wf::point_t {
    	return { v.x * m.x1 + v.y * m.y1, v.x * m.x2 + v.y * m.y2 };
	};
    /****/

    /**** For background geometry calculations */
    /** Background origin and the background's final point */
    wf::point_t b_o = { 0, 0 }, b_f = { width - corner_radius, border_size.top };

    /** Background points 1 and 2 */
	wf::point_t b_p1 = { corner_radius, 0 }, b_p2;

	/** Minimum shift needed for regular background, so it doesn't overlap corner */
	int min_shift = corner_radius;

	/** "Height" of the current edge, that is, the border_size */
	int edge_height = border_size.top;

    /** The cutoff lenghts for the background */ 
    int corner_h = std::max({ border_size.top, border_size.bottom, corner_radius });
    auto width_cut = [&](int len) { return std::max(corner_radius, len); };
	/****/

    while (stream >> current_symbol) {
        if (current_symbol == "|") {
	        current_position = (current_position == "left") ? "center" : "right";
        } else if (current_symbol == "-") {
        	/** Variables for background and accent definition */
            std::string last_accent;
        	int counter = 0;
	        for (auto vec : { left, center, right }) {
				if (vec != left) {
			        int region_lenght = 0;

			        for (auto type : vec) {
				       if (type == "title")	{
					       region_lenght += title_size.width;
				       } else if (type == "icon") {
					       region_lenght += icon_size;
				       } else if (type == "p") {
					       region_lenght += padding_size;
				       } else if (type[0] == 'P') {
					       int delta;
					       std::stringstream num;
					       num << type.substr(1);
					       num >> delta;
					       region_lenght += delta;
				       } else if (type != "a" && type[0] != 'A') {
					       region_lenght += button_size;
				       }
			        }

					if (vec == center) {
				        shift = (trans(l).x - region_lenght) / 2;
					} else {
				        shift = trans(l).x - region_lenght;
					}
				} else {
					shift = 0;
				}

		        wf::geometry_t cur_g;
		        for (auto type : vec) {
			        int delta = 0;

			        if (type == "title") {
				        delta = title_size.width;
				        out_padding = (max_height - title_size.height) / 2;
				        cur_g = { 
					        o.x + trans(p()).x, o.y + trans(p()).y,
				            trans(title).x, trans(title).y
				        };

				        layout_areas.push_back(std::make_unique<decoration_area_t>(
						        DECORATION_AREA_TITLE, cur_g, current_edge));
			        } else if (type == "icon") {
				        delta = icon_size;
				        out_padding = (max_height - icon_size) / 2;
				        cur_g = {
				        	o.x + trans(p()).x, o.y + trans(p()).y,
				        	(m.x1 + m.y1) * icon_size, (m.x2 + m.y2) * icon_size
				        };
				        layout_areas.push_back(std::make_unique<decoration_area_t>(
						        DECORATION_AREA_ICON, cur_g, current_edge));
			        } else if (type == "p") {
				        delta = padding_size;
			        } else if (type[0] == 'P') {
					    std::stringstream num;
					    num << type.substr(1);
					    num >> delta;
				        if (current_edge == EDGE_LEFT) { shift += delta; }
			        } else if (type == "a"|| type[0] == 'A' ||
		                       type == "d" || type == "D") {
				        counter = (counter + 1) % 2;
				        out_padding = counter * edge_height;
				        b_p2 = { b_o.x + trans(p()).x, b_o.y + trans(p()).y };
				        cur_g = {
    				        std::min(b_p1.x, b_p2.x), std::min(b_p1.y, b_p2.y),
    				        abs(b_p2.x - b_p1.x), abs(b_p2.y - b_p1.y)
				        };
				        auto bg = (counter == 0) ? DECORATION_AREA_ACCENT :
			                     DECORATION_AREA_BACKGROUND;
	                    if (cur_g.width > 0 && cur_g.height > 0 &&
	                        (shift > min_shift || counter == 0)) {
    				        background_areas.push_back(
        				        std::make_unique<decoration_area_t>(bg, cur_g, type));
	                    }
				        b_p1 = b_p2;
				        last_accent = type;
			        } else {
				        delta = button_size;
				        out_padding = (max_height - button_size) / 2;
				        if (current_edge == EDGE_LEFT) { shift += delta; }
				        cur_g = { 
					       	o.x + trans(p()).x, o.y + trans(p()).y,
					       	button_size, button_size
				       	};

				        button_type_t button = (type == "minimize") ?
				                      BUTTON_MINIMIZE : ((type == "maximize") ?
			        	              BUTTON_TOGGLE_MAXIMIZE : BUTTON_CLOSE);

				        layout_areas.push_back(std::make_unique<decoration_area_t>(
						        cur_g, damage_callback, theme));
				        layout_areas.back()->as_button().set_button_type(button);
					}			        

					shift += delta;
		        }
	        }

	        shift = 0;
	        out_padding = counter * edge_height;
	        b_p2 = { b_f.x + trans(p()).x, b_f.y + trans(p()).y };
	        wf::geometry_t final_g = {
		        std::min(b_p1.x, b_p2.x), std::min(b_p1.y, b_p2.y),
		        (m.x1 + m.y1) * (b_p2.x - b_p1.x), (m.x2 + m.y2) * (b_p2.y - b_p1.y)
	        };
	        if (final_g.width > 0 && final_g.height > 0) {
    	        auto type =  DECORATION_AREA_BACKGROUND;
    	        background_areas.push_back(
    		        std::make_unique<decoration_area_t>(type, final_g, ""));
	        }
	        counter = 0;

	        if (current_edge == EDGE_TOP) {
		        current_edge = EDGE_LEFT;
		        m = { 0, 1, -1, 0 };
		        o = { border_size.left - max_height, height - border_size.bottom };
                b_o = { 0, height - border_size.bottom };
		        b_p1 = { 0, height - corner_h };
		        b_f = { border_size.left, corner_h };
		        edge_height = border_size.left;
		        min_shift = corner_radius - border_size.bottom;
	        } else if (current_edge == EDGE_LEFT) {
		        current_edge = EDGE_BOTTOM;
		        m = { 1, 0, 0, 1 };
		        o = b_o = { 0, height - border_size.bottom };
		        b_p1 = { width_cut(border_size.left), height - border_size.bottom };
		        b_f = { width - width_cut(border_size.right), height };
		        edge_height = border_size.bottom;
		        min_shift = corner_radius;
	        } else {
		        current_edge = EDGE_RIGHT;
		        m = { 0, -1, 1, 0 };
		        o = { width - border_size.right + max_height, border_size.top };
		        b_o = { width, border_size.top };
		        b_p1 = { width, corner_h };
		        b_f = { width - border_size.right, height - corner_h };
		        edge_height = border_size.right;
		        min_shift = corner_radius - border_size.top;
	        }

	        left.clear();
	        center.clear();
	        right.clear();
	        current_position = "left";
		        
        } else {
	        if (current_position == "left") {
		        left.push_back(current_symbol);
	        } else if (current_position == "center") {
		        center.push_back(current_symbol);
	        } else {
		        right.push_back(current_symbol);
	        }
        }
    }
}

/** Regenerate layout using the new size */
void decoration_layout_t::resize(int width, int height, wf::dimensions_t title_size) {
    max_height = std::max({ title_size.height, icon_size, button_size });
    this->background_areas.clear();
    this->layout_areas.clear();

    create_areas(width, height, title_size);

	/* Areas for resizing only, used for movement area calculation */
    int top_resize    = std::min(std::max(border_size.top - max_height, 7),
                              border_size.top);
    int left_resize   = std::min(std::max(border_size.left - max_height, 7),
                                border_size.left);
    int bottom_resize = std::min(std::max(border_size.bottom - max_height, 7),
                                 border_size.bottom);
    int right_resize  = std::min(std::max(border_size.right - max_height, 7),
                                border_size.right);
    /* Moving edges */
    for (wf::geometry_t g : {
	    (wf::geometry_t){ left_resize, top_resize, width - left_resize - right_resize,
	                      std::max(border_size.top - top_resize, 0) },
	    (wf::geometry_t){ left_resize, border_size.top,
	                      std::max(border_size.left - left_resize, 0),
	                      height - border_size.top - border_size.bottom },
	    (wf::geometry_t){ left_resize, height - border_size.bottom,
	                      width - left_resize - right_resize,
	      				  std::max(border_size.bottom - bottom_resize, 0) },
	    (wf::geometry_t){ width - border_size.right, border_size.top,
	      				  std::max(border_size.right - right_resize, 0),
	      				  height - border_size.top - border_size.bottom }
    	} ) {
	    this->layout_areas.push_back(std::make_unique<decoration_area_t>(
		    DECORATION_AREA_MOVE, g));
    }
    
    /* Resizing edges - top */
    wf::geometry_t border_geometry = { 0, 0, width, top_resize };
    this->layout_areas.push_back(std::make_unique<decoration_area_t>(
        DECORATION_AREA_RESIZE_TOP, border_geometry, EDGE_TOP));

    /* Resizing edges - left */
    border_geometry = { 0, 0, left_resize, height };
    this->layout_areas.push_back(std::make_unique<decoration_area_t>(
        DECORATION_AREA_RESIZE_LEFT, border_geometry, EDGE_LEFT));

    /* Resizing edges - bottom */
    border_geometry = { 0, height - bottom_resize, width, bottom_resize };
    this->layout_areas.push_back(std::make_unique<decoration_area_t>(
        DECORATION_AREA_RESIZE_BOTTOM, border_geometry, EDGE_BOTTOM));

    /* Resizing edges - right */
    border_geometry = { width - right_resize, 0, right_resize, height };
    this->layout_areas.push_back(std::make_unique<decoration_area_t>(
        DECORATION_AREA_RESIZE_RIGHT, border_geometry, EDGE_RIGHT));
}

/**
 * @return The decoration areas which need to be rendered, in top to bottom
 *  order.
 */
std::vector<nonstd::observer_ptr<decoration_area_t>> decoration_layout_t::
get_renderable_areas() {
    std::vector<nonstd::observer_ptr<decoration_area_t>> renderable;
    for (auto& area : layout_areas) {
        if (area->get_type() & AREA_RENDERABLE_BIT) {
            renderable.push_back({area});
        }
    }

    return renderable;
}

std::vector<nonstd::observer_ptr<decoration_area_t>> decoration_layout_t::
get_background_areas() {
    std::vector<nonstd::observer_ptr<decoration_area_t>> areas;
    for (auto& area : background_areas) {
        areas.push_back({area});
    }
    return areas;
}

wf::region_t decoration_layout_t::calculate_region() const {
    wf::region_t r{};
    for (auto& area : layout_areas) {
        r |= area->get_geometry();
    }

    return r;
}

void decoration_layout_t::unset_hover(wf::point_t position) {
    auto area = find_area_at(position);
    if (area && (area->get_type() == DECORATION_AREA_BUTTON)) {
        area->as_button().set_hover(false);
    }
}

/** Handle motion event to (x, y) relative to the decoration */
decoration_layout_t::action_response_t decoration_layout_t::handle_motion(
    int x, int y) {
    auto previous_area = find_area_at(current_input);
    auto current_area  = find_area_at({x, y});

    if (previous_area == current_area) {
        if (is_grabbed && current_area && (current_area->get_type() & AREA_MOVE_BIT)) {
            is_grabbed = false;
            return {DECORATION_ACTION_MOVE, 0};
        }
    } else {
        unset_hover(current_input);
        if (current_area && (current_area->get_type() == DECORATION_AREA_BUTTON)) {
            current_area->as_button().set_hover(true);
        }
    }

    this->current_input = {x, y};
    update_cursor();

    return { DECORATION_ACTION_NONE, 0 };
}

/**
 * Handle press or release event.
 * @param pressed Whether the event is a press(true) or release(false)
 *  event.
 * @return The action which needs to be carried out in response to this
 *  event.
 * */
decoration_layout_t::action_response_t decoration_layout_t::handle_press_event(
    bool pressed) {
    if (pressed) {
        auto area = find_area_at(current_input);
        if (area && (area->get_type() & AREA_MOVE_BIT)) {
            if (timer.is_connected()) {
                double_click_at_release = true;
            } else {
                timer.set_timeout(300, [] () { return false; });
            }
        }

        if (area && (area->get_type() & AREA_RESIZE_BIT)) {
            return {DECORATION_ACTION_RESIZE, calculate_resize_edges()};
        }

        if (area && (area->get_type() == DECORATION_AREA_BUTTON)) {
            area->as_button().set_pressed(true);
        }

        is_grabbed  = true;
        grab_origin = current_input;
    }

    if (!pressed && double_click_at_release) {
        double_click_at_release = false;
        return { DECORATION_ACTION_TOGGLE_MAXIMIZE, 0 };
    } else if (!pressed && is_grabbed) {
        is_grabbed = false;
        auto begin_area = find_area_at(grab_origin);
        auto end_area   = find_area_at(current_input);

        if (begin_area && (begin_area->get_type() == DECORATION_AREA_BUTTON)) {
            begin_area->as_button().set_pressed(false);
            if (end_area && (begin_area == end_area)) {
                switch (begin_area->as_button().get_button_type()) {
                  case BUTTON_CLOSE:
                    return { DECORATION_ACTION_CLOSE, 0 };

                  case BUTTON_TOGGLE_MAXIMIZE:
                    return { DECORATION_ACTION_TOGGLE_MAXIMIZE, 0 };

                  case BUTTON_MINIMIZE:
                    return { DECORATION_ACTION_MINIMIZE, 0 };

                  default:
                    break;
                }
            }
        }
    }

    return { DECORATION_ACTION_NONE, 0};
}

/**
 * Find the layout area at the given coordinates, if any
 * @return The layout area or null on failure
 */
nonstd::observer_ptr<decoration_area_t> decoration_layout_t::find_area_at( wf::point_t point) {
    for (auto& area : this->layout_areas) {
        if (area->get_geometry() & point) {
            return {area};
        }
    }

    return nullptr;
}

/** Calculate resize edges based on @current_input */
uint32_t decoration_layout_t::calculate_resize_edges() const {
    uint32_t edges = 0;
    for (auto& area : layout_areas) {
        if (area->get_geometry() & this->current_input) {
            if (area->get_type() & AREA_RESIZE_BIT) {
                edges |= (area->get_type() & ~AREA_RESIZE_BIT);
            }
        }
    }

    return edges;
}

/** Update the cursor based on @current_input */
void decoration_layout_t::update_cursor() const {
    uint32_t edges   = calculate_resize_edges();
    auto cursor_name = edges > 0 ?
        wlr_xcursor_get_resize_name((wlr_edges)edges) : "default";
    wf::get_core().set_cursor(cursor_name);
}

void decoration_layout_t::handle_focus_lost() {
    if (is_grabbed) {
        this->is_grabbed = false;
        auto area = find_area_at(grab_origin);
        if (area && (area->get_type() == DECORATION_AREA_BUTTON)) {
            area->as_button().set_pressed(false);
        }
    }

    this->unset_hover(current_input);
}
}
}
