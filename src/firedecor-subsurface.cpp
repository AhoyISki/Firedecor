#include <glm/gtc/matrix_transform.hpp>

#include <linux/input-event-codes.h>

#include <wayfire/nonstd/wlroots.hpp>
#include <wayfire/compositor-surface.hpp>
#include <wayfire/output.hpp>
#include <wayfire/opengl.hpp>
#include <wayfire/core.hpp>
#include <wayfire/decorator.hpp>
#include <wayfire/view-transform.hpp>
#include <wayfire/signal-definitions.hpp>

#include "firedecor-layout.hpp"
#include "firedecor-theme.hpp"

#include <wayfire/plugins/common/cairo-util.hpp>

//#include <cairo.h>

#define ACTIVE true
#define INACTIVE false
#define FORCE true
#define DONT_FORCE false

class simple_decoration_surface : public wf::surface_interface_t,
	public wf::compositor_surface_t {
	bool _mapped = true;
    wayfire_view view;

    wf::signal_connection_t title_set = [=] (wf::signal_data_t *data) {
        if (get_signaled_view(data) == view) {
            update_layout(FORCE);
            view->damage(); // trigger re-render
        }
    };

    void update_title(double scale) {
		cairo_surface_t *surface;
		wf::dimensions_t title_size = {
    		(int)(title.dims.width * scale), (int)(title.dims.height * scale)
        };

		auto o = wf::firedecor::HORIZONTAL;
        surface = theme.form_title(title.text, title_size, ACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.hor_active);
        surface = theme.form_title(title.text, title_size, INACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.hor_inactive);
		o = wf::firedecor::VERTICAL;
		surface = theme.form_title(title.text, title_size, ACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.ver_active);
        surface = theme.form_title(title.text, title_size, INACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.ver_inactive);
        cairo_surface_destroy(surface); 

        title_needs_update = false;
    }

    void update_icon() {
        if (view->get_app_id() != icon.app_id) {
	        icon.app_id = view->get_app_id();
	        auto surface = theme.form_icon(icon.app_id);
	        cairo_surface_upload_to_texture(surface, icon.texture);
            cairo_surface_destroy(surface);
        }
    }

    void update_layout(bool force) {
        auto title_colors = theme.get_title_colors();

        if ((title.colors != title_colors) || force) {
            /* Updating the cached variables */
            title.colors = title_colors;
	        title.text = (theme.get_debug_mode()) ? view->get_app_id() :
	        										view->get_title();

	        title.dims = theme.get_text_size(title.text, size.width);

		    title_needs_update = true;

            /* Necessary in order to immediately place areas correctly */
    		layout.resize(size.width, size.height, title.dims);
        }

		c.tr = c.tl = c.bl = c.br = 0;
		std::stringstream round_on_str(theme.get_round_on());
		std::string corner;
		while (round_on_str >> corner) {
    		if (corner == "all") {
        		c.tr = c.tl = c.bl = c.br = theme.get_corner_radius();
        		break;
    		} else if (corner == "tr") {
        		c.tr = theme.get_corner_radius();
    		} else if (corner == "tl") {
        		c.tl = theme.get_corner_radius();
    		} else if (corner == "bl") {
        		c.bl = theme.get_corner_radius();
    		} else if (corner == "br") {
        		c.br = theme.get_corner_radius();
    		}
		}
    }

    struct {
        wf::simple_texture_t hor_active, hor_inactive;
        wf::simple_texture_t ver_active, ver_inactive;
        std::string text = "";
        wf::firedecor::color_set_t colors;
        wf::dimensions_t dims;
    } title;

    struct {
	    wf::simple_texture_t texture;
	    std::string app_id = "Does not match";
    } icon;

    struct corner_textures_t {
	    wf::simple_texture_t active, inactive;
    } corners;

    struct edge_colors_t {
	    wf::firedecor::color_set_t border, outline;
    } edges;

    struct {
        int tr = 0, tl = 0, bl = 0, br = 0;
    } c;

    wf::firedecor::decoration_theme_t theme;
    wf::firedecor::decoration_layout_t layout;
    wf::region_t cached_region;

    bool title_needs_update = false;

    wf::dimensions_t size;

	void update_corners(edge_colors_t colors, int corner_radius) {
		if ((this->corner_radius != corner_radius) ||
			(edges.border != colors.border) ||
			(edges.outline != colors.outline)) {
			auto surface = theme.form_corner(ACTIVE);
			OpenGL::render_begin();
			cairo_surface_upload_to_texture(surface, corners.active);
			surface = theme.form_corner(INACTIVE);
			cairo_surface_upload_to_texture(surface, corners.inactive);
			OpenGL::render_end();
			edges.border.active    = colors.border.active;
			edges.border.inactive  = colors.border.inactive;
			edges.outline.active   = colors.outline.active;
			edges.outline.inactive = colors.outline.inactive;
			this->corner_radius    = corner_radius;
		}
	}

  public:
    wf::firedecor::border_size_t border_size;
    int corner_radius;

    simple_decoration_surface(wayfire_view view) : theme{}, 
    	layout{theme, [=] (wlr_box box) {this->damage_surface_box(box); }} {
        this->view = view;
        view->connect_signal("title-changed", &title_set);

        // make sure to hide frame if the view is fullscreen
        update_decoration_size();
    }
    
    virtual bool is_mapped() const final {
        return _mapped;
    }

    wf::point_t get_offset() final {
        return { -border_size.left, -border_size.top };
    }

    virtual wf::dimensions_t get_size() const final {
        return size;
    }

    void render_title(const wf::framebuffer_t& fb, wf::geometry_t geometry,
                      wf::firedecor::edge_t edge, wf::geometry_t scissor) {
	    wf::simple_texture_t *texture;
	    bool active = view->activated;

	    if (title_needs_update) {
    	    update_title(fb.scale);
	    }

	    uint32_t bits = 0;
	    if (edge == wf::firedecor::EDGE_TOP || edge == wf::firedecor::EDGE_BOTTOM) {
	        if (active) {
		        texture = &title.hor_active;
	        } else {
		        texture = &title.hor_inactive;
	        }
	        bits = OpenGL::TEXTURE_TRANSFORM_INVERT_Y;
	    } else if (edge == wf::firedecor::EDGE_LEFT) {
	        if (active) {
		        texture = &title.ver_active;
	        } else {
		        texture = &title.ver_inactive;
	        }
	        bits = OpenGL::TEXTURE_TRANSFORM_INVERT_Y;
	    } else {
	        if (active) {
		        texture = &title.ver_active;
	        } else {
		        texture = &title.ver_inactive;
	        }
	        bits = OpenGL::TEXTURE_TRANSFORM_INVERT_X;
	    } 
		OpenGL::render_begin(fb);
        fb.logic_scissor(scissor);
        OpenGL::render_texture(texture->tex, fb, geometry, glm::vec4(1.0f), bits);
		OpenGL::render_end();
    }

    void render_icon(const wf::framebuffer_t& fb, wf::geometry_t g, const wf::geometry_t& scissor) {
        update_icon();
		OpenGL::render_begin(fb);
		fb.logic_scissor(scissor);
		OpenGL::render_texture(icon.texture.tex, fb, g, glm::vec4(1.0f),
		                       OpenGL::TEXTURE_TRANSFORM_INVERT_Y);
		OpenGL::render_end();
    }

    wf::color_t alpha_transform(wf::color_t c) {
	    return { c.r * c.a, c.g * c.a, c.b * c.a, c.a };
    }

	void render_background(const wf::framebuffer_t& fb, 
		wf::geometry_t rect, const wf::geometry_t& scissor, bool active, wf::point_t origin) {
		edge_colors_t colors = {
			theme.get_border_colors(), theme.get_outline_colors()
		};

		colors.border.active = alpha_transform(colors.border.active);
		colors.border.inactive = alpha_transform(colors.border.inactive);

		int r = theme.get_corner_radius();
		update_corners(colors, r);

		auto& corner = active ? corners.active : corners.inactive;

		wf::geometry_t top_left     = { rect.x, rect.y, r, r };
		wf::geometry_t top_right    = { rect.width - r + origin.x, rect.y, r, r };
		wf::geometry_t bottom_left  = { rect.x, rect.height - r + origin.y, r, r };
		wf::geometry_t bottom_right = { rect.width - r + origin.x,
		                               	rect.height - r + origin.y, r, r };
		int outline_size            = theme.get_outline_size();

		OpenGL::render_begin(fb);
		fb.logic_scissor(scissor);

		/** Borders */
		wf::color_t color = (active) ? colors.border.active : colors.border.inactive;
		for (auto g : std::vector<wf::geometry_t>{
    		{ rect.x + r, rect.y, rect.width - 2 * r, rect.height },
    		{ rect.x, rect.y + c.tl, r, rect.height - (c.tl + c.bl) },
    		{ rect.width + origin.x - r, rect.y + c.tr,
    		  r, rect.height - (c.tr + c.br) } }) {
        	OpenGL::render_rectangle(g, color, fb.get_orthographic_projection());
		}

		/* Outlines */
		color = (active) ? colors.outline.active : colors.outline.inactive;
		for (auto g : std::vector<wf::geometry_t>{
    		{ rect.x + c.tl, rect.y, rect.width - (c.tl + c.tr), outline_size },
    		{ rect.x + c.tr, rect.height - outline_size + origin.y,
    		  rect.width - (c.bl + c.br), outline_size },
    		{ rect.x, rect.y + c.tl, outline_size, rect.height - (c.tl + c.bl) },
    		{ rect.width + origin.x - outline_size, rect.y + c.tr,
    		  outline_size, rect.height - (c.tr + c.br) } }) {
			OpenGL::render_rectangle(g, color, fb.get_orthographic_projection());
		}

		/** Top right corner */
		if (c.tr > 0) {
    		OpenGL::render_texture(corner.tex, fb, top_right, glm::vec4(1.0f));
		}
		/** Top left corner */
		if (c.tl > 0) {
    		OpenGL::render_texture(corner.tex, fb, top_left, glm::vec4(1.0f),
                        		   OpenGL::TEXTURE_TRANSFORM_INVERT_X);
		}
		/** Bottom left corner */
		if (c.bl > 0) {
    		OpenGL::render_texture(corner.tex, fb, bottom_left, glm::vec4(1.0f),
    							   OpenGL::TEXTURE_TRANSFORM_INVERT_X |
    		                       OpenGL::TEXTURE_TRANSFORM_INVERT_Y);
		}
		/** Bottom right corner */
		if (c.br > 0) {
    		OpenGL::render_texture(corner.tex, fb, bottom_right, glm::vec4(1.0f),
                        		   OpenGL::TEXTURE_TRANSFORM_INVERT_Y);
		}
		OpenGL::render_end();
	}

    void render_scissor_box(const wf::framebuffer_t& fb, wf::point_t origin,
        const wlr_box& scissor) {
	    /* Draw the background (corners and border) */
        wlr_box geometry{origin.x, origin.y, size.width, size.height};
        render_background(fb, geometry, scissor, view->activated, origin);

        auto renderables = layout.get_renderable_areas();
        for (auto item : renderables) {
	        if (item->get_type() == wf::firedecor::DECORATION_AREA_TITLE) {
                render_title(fb, item->get_geometry() + origin, 
                			 item->get_edge(), scissor);
            } else if (item->get_type() == wf::firedecor::DECORATION_AREA_BUTTON) {
	            item->as_button().set_active(view->activated);
	            item->as_button().set_maximized(view->tiled_edges);
                item->as_button().render(fb, item->get_geometry() + origin, scissor);
            } else if (item->get_type() == wf::firedecor::DECORATION_AREA_ICON) {
	            render_icon(fb, item->get_geometry() + origin, scissor);
            }
        }
    }
    
    virtual void simple_render(const wf::framebuffer_t& fb, int x, int y,
					           const wf::region_t& damage) override {
        wf::region_t frame = this->cached_region + (wf::point_t){x, y};
        frame &= damage;

    	update_layout(DONT_FORCE);

        for (const auto& box : frame) {
            render_scissor_box(fb, {x, y}, wlr_box_from_pixman_box(box));
        }
    }

    bool accepts_input(int32_t sx, int32_t sy) override {
        return pixman_region32_contains_point(cached_region.to_pixman(),
            sx, sy, NULL);
    }

    virtual void on_pointer_enter(int x, int y) override {
        layout.handle_motion(x, y);
    }

    virtual void on_pointer_leave() override {
        layout.handle_focus_lost();
    }

    virtual void on_pointer_motion(int x, int y) override {
        handle_action(layout.handle_motion(x, y));
    }

    virtual void on_pointer_button(uint32_t button, uint32_t state) override {
        if (button != BTN_LEFT) {
            return;
        }

        handle_action(layout.handle_press_event(state == WLR_BUTTON_PRESSED));
    }

	// TODO: implement a pinning button.
    void handle_action(wf::firedecor::decoration_layout_t::action_response_t action) {
        switch (action.action) {
          case wf::firedecor::DECORATION_ACTION_MOVE:
            return view->move_request();

          case wf::firedecor::DECORATION_ACTION_RESIZE:
            return view->resize_request(action.edges);

          case wf::firedecor::DECORATION_ACTION_CLOSE:
            return view->close();

          case wf::firedecor::DECORATION_ACTION_TOGGLE_MAXIMIZE:
            if (view->tiled_edges) {
                view->tile_request(0);
            } else {
                view->tile_request(wf::TILED_EDGES_ALL);
            }

            break;

          case wf::firedecor::DECORATION_ACTION_MINIMIZE:
            view->minimize_request(true);
            break;

          default:
            break;
        }
    }

    virtual void on_touch_down(int x, int y) override {
        layout.handle_motion(x, y);
        handle_action(layout.handle_press_event());
    }

    virtual void on_touch_motion(int x, int y) override {
        handle_action(layout.handle_motion(x, y));
    }

    virtual void on_touch_up() override {
        handle_action(layout.handle_press_event(false));
        layout.handle_focus_lost();
    }

    void unmap() {
        _mapped = false;
        wf::emit_map_state_change(this);
    }

    void resize(wf::dimensions_t dims) {
        view->damage();
        size = dims;
		layout.resize(size.width, size.height, title.dims);
        if (!view->fullscreen) {
            this->cached_region = layout.calculate_region();
        }

        view->damage();
    }

    void update_decoration_size() {
        if (view->fullscreen) {
            border_size = { 0, 0, 0, 0 };
            this->cached_region.clear();
        } else {
            border_size = layout.parse_border(theme.get_border_size());
            this->cached_region = layout.calculate_region();
        }
    }
};

class simple_decorator_t : public wf::decorator_frame_t_t {
    wayfire_view view;
    nonstd::observer_ptr<simple_decoration_surface> deco;

  public:
    simple_decorator_t(wayfire_view view) {
        this->view = view;

        auto sub = std::make_unique<simple_decoration_surface>(view);
        deco = {sub};
        view->add_subsurface(std::move(sub), true);
        view->damage();
        view->connect_signal("subsurface-removed", &on_subsurface_removed);
    }

    ~simple_decorator_t() {
        if (deco) {
            // subsurface_removed unmaps it
            view->remove_subsurface(deco);
        }
    }

    simple_decorator_t(const simple_decorator_t &) = delete;
    simple_decorator_t(simple_decorator_t &&) = delete;
    simple_decorator_t& operator =(const simple_decorator_t&) = delete;
    simple_decorator_t& operator =(simple_decorator_t&&) = delete;

    wf::signal_connection_t on_subsurface_removed = [&] (auto data) {
        auto ev = static_cast<wf::subsurface_removed_signal*>(data);
        if (ev->subsurface.get() == deco.get()) {
            deco->unmap();
            deco = nullptr;
        }
    };

    virtual wf::geometry_t expand_wm_geometry( wf::geometry_t contained_wm_geometry) override {
        contained_wm_geometry.x     -= deco->border_size.left;
        contained_wm_geometry.y     -= deco->border_size.top;
        contained_wm_geometry.width += deco->border_size.left +
        	deco->border_size.right;
        contained_wm_geometry.height += deco->border_size.top +
        	deco->border_size.bottom;

        return contained_wm_geometry;
    }

    // TODO: Minimum size must fit buttons, icon, a truncated title, and corners.
    virtual void calculate_resize_size( int& target_width, int& target_height) override {
        target_width  -= deco->border_size.left +
        	deco->border_size.right;
        target_height -= deco->border_size.top + deco->border_size.bottom;

        target_width  = std::max(target_width, 1);
        target_height = std::max(target_height, 1);
    }

    virtual void notify_view_activated(bool active) override {
	    (void)active;
        view->damage();
    }

    virtual void notify_view_resized(wf::geometry_t view_geometry) override {
        deco->resize(wf::dimensions(view_geometry));
    }

    virtual void notify_view_tiled() override {}

    virtual void notify_view_fullscreen() override {
        deco->update_decoration_size();

        if (!view->fullscreen) {
            notify_view_resized(view->get_wm_geometry());
        }
    }
};

void init_view(wayfire_view view) {
    auto firedecor = std::make_unique<simple_decorator_t>(view);
    view->set_decoration(std::move(firedecor));
}

void deinit_view(wayfire_view view) {
    view->set_decoration(nullptr);
}
