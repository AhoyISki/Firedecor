#pragma once

#include <string>
#include <wayfire/util.hpp>
#include <wayfire/opengl.hpp>
#include <wayfire/surface.hpp>
#include <wayfire/render-manager.hpp>
#include <wayfire/view.hpp>
#include <wayfire/util/duration.hpp>
#include <wayfire/plugins/common/simple-texture.hpp>
#include <wayfire/plugins/common/cairo-util.hpp>

#include <cairo/cairo.h>

namespace wf {
namespace firedecor {
class decoration_theme_t;

enum button_type_t {
    BUTTON_CLOSE,
    BUTTON_TOGGLE_MAXIMIZE,
    BUTTON_MINIMIZE,
};

class button_t {
  public:
    /**
     * Create a new button with the given theme.
     * @param theme  The theme to use.
     * @param damage_callback   A callback to execute when the button needs a
     * repaint. Damage won't be reported while render() is being called.
     */
    button_t(const decoration_theme_t& theme, std::function<void()> damage_callback);

    ~button_t() = default;
    button_t(const button_t &) = delete;
    button_t(button_t &&) = delete;
    button_t& operator =(const button_t&) = delete;
    button_t& operator =(button_t&&) = delete;

    /**
     * Set the type of the button. This will affect the displayed icon and
     * potentially other appearance like colors.
     */
    void set_button_type(button_type_t type);

    /** @return The type of the button */
    button_type_t get_button_type() const;

    /** Set the activation status of the button's view */
    void set_active(bool active);
    /** Set the maximized status of the button's view */
    void set_maximized(uint32_t edges);

    /**
     * Set the button hover state.
     * Affects appearance.
     */
    void set_hover(bool is_hovered);

    /**
     * Set whether the button is pressed or not.
     * Affects appearance.
     */
    void set_pressed(bool is_pressed);

    /**
     * Render the button on the given framebuffer at the given coordinates.
     * Precondition: set_button_type() has been called, otherwise result is no-op
     *
     * @param buffer The target framebuffer
     * @param geometry The geometry of the button, in logical coordinates
     * @param scissor The scissor rectangle to render.
     */
    void render(const wf::render_target_t& buffer, wf::geometry_t geometry,
        wf::geometry_t scissor);

  private:
 	const decoration_theme_t& theme;

    /* Whether the button needs repaint */
    button_type_t type;
    wf::simple_texture_t button_texture;

    /* Whether the button is currently being hovered */
    bool is_hovered = false;
    /* Whether the button is currently being held */
    bool is_pressed = false;
    /* Whether the button's view is active or not */
    bool active = true;
    /* Whether the button's view is maximized or not */
    bool maximized = false;
    
    /* The shade of button background to use. */
    wf::animation::simple_animation_t hover{wf::create_option(100)};

    std::function<void()> damage_callback;
    wf::wl_idle_call idle_damage;
    /** Damage button the next time the main loop goes idle */
    void add_idle_damage();

    /**
     * Redraw the button surface and store it as a texture
     */
    void update_texture();
};
}
}
