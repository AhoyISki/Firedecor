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

#include <cairo.h>

#define ACTIVE true
#define INACTIVE false

class simple_decoration_surface : public wf::surface_interface_t, public wf::compositor_surface_t {
	bool _mapped = true;
    wayfire_view view;

	// Re-renders after a title change.
    wf::signal_connection_t title_set = [=] (wf::signal_data_t *data) {
        if (get_signaled_view(data) == view) {
            view->damage(); // trigger re-render
        }
    };

	// Uploads a new title surface to the OpenGL texture, doesn't do any rendering.
    void update_title(int width, int height, double scale) {
        int target_width  = width * scale;
        int target_height = height * scale;

        if ((title_texture.tex.width != target_width) ||
            (title_texture.tex.height != target_height) ||
            (title_texture.current_text != view->get_title())) {
            auto surface = theme.form_text(view->get_title(),
                target_width, target_height);
            cairo_surface_upload_to_texture(surface, title_texture.tex);
            cairo_surface_destroy(surface);
            title_texture.current_text = view->get_title();
        }
    }

    struct {
        wf::simple_texture_t tex;
        std::string current_text = "";
    } title_texture;

    struct corner_textures_t {
	    wf::simple_texture_t active, inactive;
    } corners;

    wf::firedecor::decoration_theme_t::edge_colors_t current_edge;

	void update_corners(
		wf::firedecor::decoration_theme_t::edge_colors_t colors, int corner_radius) {
		if ((current_corner_radius != corner_radius) ||
			!(current_edge.active_border == colors.active_border) ||
			!(current_edge.active_border == colors.inactive_border) ||
			!(current_edge.active_outline == colors.active_outline) ||
		    !(current_edge.inactive_outline == colors.inactive_outline)) {
			auto surface = theme.form_corner(ACTIVE);
			cairo_surface_upload_to_texture(surface, corners.active);
			surface = theme.form_corner(INACTIVE);
			cairo_surface_upload_to_texture(surface, corners.inactive);
			current_edge.active_border = colors.active_border;
			current_edge.inactive_border = colors.inactive_border;
			current_edge.active_outline = colors.active_outline;
			current_edge.inactive_outline = colors.inactive_outline;
			current_corner_radius = corner_radius;
		}
	};

    wf::firedecor::decoration_theme_t theme;
    wf::firedecor::decoration_layout_t layout;
    wf::region_t cached_region;

    wf::dimensions_t size;

  public:
	// Variables with more descriptive names.
    int current_border_size;
    int current_titlebar_height;
    int current_corner_radius;

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

    // TODO: Make it possible for the titlebar to be positioned at any edge.
    wf::point_t get_offset() final {
        return {-current_border_size, -current_titlebar_height};
    }

    virtual wf::dimensions_t get_size() const final {
        return size;
    }

    void render_title(const wf::framebuffer_t& fb, wf::geometry_t geometry) {
        update_title(geometry.width, geometry.height, fb.scale);
        OpenGL::render_texture(title_texture.tex.tex, fb, geometry,
            glm::vec4(1.0f), OpenGL::TEXTURE_TRANSFORM_INVERT_Y);
    }

    void render_background(const wf::framebuffer_t& fb, 
    	wf::geometry_t rect, const wf::geometry_t& scissor, bool active, wf::point_t origin) {
	    auto colors = theme.get_edge_colors();

	    int r = theme.get_corner_radius();
	    update_corners(colors, r);
	    r = current_corner_radius;
	    auto& corner = active ? corners.active : corners.inactive;

	    wf::geometry_t top_left = { rect.x, rect.y, r, r };
	    wf::geometry_t top_right = { rect.width - r + origin.x, rect.y, r, r };
	    wf::geometry_t bottom_left = { rect.x, rect.height - r + origin.y, r, r };
	    wf::geometry_t bottom_right = { rect.width - r, rect.height - r, r, r };
		int outline_size = theme.get_outline_size();

		bottom_right = bottom_right + origin;

	 	wf::color_t back = { 0.114 * 0.9, 0.122 * 0.9, 0.129 * 0.9, 0.9 };

		/** Non corner background */
		OpenGL::render_begin(fb);
	    fb.logic_scissor(scissor);

		/** Middle rectangle and central outlines */
		OpenGL::render_rectangle(
			{ rect.x + r, rect.y, rect.width - 2 * r, rect.height },
			/*active ? colors.active_border : colors.inactive_border*/ back,
			fb.get_orthographic_projection());
		OpenGL::render_rectangle(
			{ rect.x + r, rect.y, rect.width - 2 * r, outline_size},
			active ? colors.active_outline : colors.inactive_outline,
			fb.get_orthographic_projection());
		OpenGL::render_rectangle(
			{ rect.x + r, rect.height - outline_size + origin.y,
			  rect.width - 2 * r, outline_size },
			active ? colors.active_outline : colors.inactive_outline,
			fb.get_orthographic_projection());

		/** Left border and outline */
		OpenGL::render_rectangle(
			{ rect.x, rect.y + r, r, rect.height - 2 * r },
			/*active ? colors.active_border : colors.inactive_border*/ back,
			fb.get_orthographic_projection());
		OpenGL::render_rectangle(
			{ rect.x, rect.y + r, outline_size, rect.height - 2 * r },
			active ? colors.active_outline : colors.inactive_outline,
			fb.get_orthographic_projection());

		/** Right border and outline */
		OpenGL::render_rectangle(
			{ rect.width - r + origin.x, rect.y + r, r, rect.height - 2 * r },
			/*active ? colors.active_border : colors.inactive_border*/ back,
			fb.get_orthographic_projection());
		OpenGL::render_rectangle(
			{ rect.width - outline_size + origin.x, 
			  rect.y + r, outline_size, rect.height - 2 * r },
			active ? colors.active_outline : colors.inactive_outline,
			fb.get_orthographic_projection());
		/** Corner background */

		/* Top left corner */
		OpenGL::render_texture(corner.tex, fb, top_left, glm::vec4(1.0f),
			OpenGL::TEXTURE_TRANSFORM_INVERT_X);

		/* Top right corner */
		OpenGL::render_texture(corner.tex, fb, top_right, glm::vec4(1.0f));

		/* Bottom left corner */
		OpenGL::render_texture(corner.tex, fb, bottom_left, glm::vec4(1.0f),
			OpenGL::TEXTURE_TRANSFORM_INVERT_X | OpenGL::TEXTURE_TRANSFORM_INVERT_Y);

		/* Bottom right corner */
		OpenGL::render_texture(corner.tex, fb, bottom_right, glm::vec4(1.0f),
			OpenGL::TEXTURE_TRANSFORM_INVERT_Y);
		OpenGL::render_end();
    }

    void render_scissor_box(const wf::framebuffer_t& fb, wf::point_t origin,
        const wlr_box& scissor) {
        /* Clear background */
        wlr_box geometry{origin.x, origin.y, size.width, size.height};
        render_background(fb, geometry, scissor, view->activated, origin);

        /* Draw title & buttons */
        auto renderables = layout.get_renderable_areas();
        for (auto item : renderables) {
            if (item->get_type() == wf::firedecor::DECORATION_AREA_TITLE) {
                OpenGL::render_begin(fb);
                fb.logic_scissor(scissor);
                render_title(fb, item->get_geometry() + origin);
                OpenGL::render_end();
            } else if 
            	(item->get_type() == wf::firedecor::DECORATION_AREA_BUTTON) { // button
                item->as_button().render(fb, item->get_geometry() + origin, scissor);
            }
        }
    }

    virtual void simple_render(const wf::framebuffer_t& fb, int x, int y,
					           const wf::region_t& damage) override {
        wf::region_t frame = this->cached_region + wf::point_t{x, y};
        frame &= damage;

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
        layout.resize(size.width, size.height);
        if (!view->fullscreen) {
            this->cached_region = layout.calculate_region();
        }

        view->damage();
    }

    void update_decoration_size() {
        if (view->fullscreen) {
            current_border_size = 0;
            current_titlebar_height  = 0;
            this->cached_region.clear();
        } else {
            current_border_size = theme.get_border_size();
            current_titlebar_height  =
                theme.get_titlebar_height() + theme.get_border_size();
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

    // TODO: Make it possible for the titlebar to be positioned at any edge.
    virtual wf::geometry_t expand_wm_geometry( wf::geometry_t contained_wm_geometry) override {
        contained_wm_geometry.x     -= deco->current_border_size;
        contained_wm_geometry.y     -= deco->current_titlebar_height;
        contained_wm_geometry.width += 2 * deco->current_border_size;
        contained_wm_geometry.height += deco->current_border_size + deco->current_titlebar_height;

        return contained_wm_geometry;
    }

    // TODO: Make it possible for the titlebar to be positioned at any edge.
    virtual void calculate_resize_size( int& target_width, int& target_height) override {
        target_width  -= 2 * deco->current_border_size;
        target_height -= deco->current_border_size + deco->current_titlebar_height;

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