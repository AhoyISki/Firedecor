#include <wayfire/core.hpp>
#include <wayfire/opengl.hpp>
#include <wayfire/config.h>

#include <map>

#include "firedecor-theme.hpp"
#include "firedecor-buttons.hpp"

namespace wf {
namespace firedecor {
/** Create a new theme with the default parameters */
decoration_theme_t::decoration_theme_t() {}

/** @return The available height for displaying the title */
int decoration_theme_t::get_titlebar_height() const {
    return titlebar_height;
}

/** @return The available border for resizing */
int decoration_theme_t::get_border_size() const {
    return border_size;
}

/** @return The updated color list for the edges of the decoration */
decoration_theme_t::edge_colors_t decoration_theme_t::get_edge_colors() const {
	return { 
		active_color, inactive_color,
		active_outline_color, inactive_outline_color
	};
}

cairo_surface_t *decoration_theme_t::form_text(std::string text,
    int width, int height) const {
    const auto format = CAIRO_FORMAT_ARGB32;
    auto surface = cairo_image_surface_create(format, width, height);

    if (height == 0) {
        return surface;
    }

    auto cr = cairo_create(surface);

    const float font_scale = 0.8;
    const float font_size  = height * font_scale;

    PangoFontDescription *font_desc;
    PangoLayout *layout;

    // render text
    font_desc = pango_font_description_from_string(((std::string)font).c_str());
    pango_font_description_set_absolute_size(font_desc, font_size * PANGO_SCALE);

    layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(layout, font_desc);
    pango_layout_set_text(layout, text.c_str(), text.size());
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    pango_cairo_show_layout(cr, layout);
    pango_font_description_free(font_desc);
    g_object_unref(layout);
    cairo_destroy(cr);

    return surface;
}

cairo_surface_t *decoration_theme_t::form_corner(bool active) const {
	int outline_radius = corner_radius - outline_size / 2;

    cairo_surface_t *corner_surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, corner_radius, corner_radius);
    auto cra = cairo_create(active_corner_surface);
    cairo_set_operator(cra, CAIRO_OPERATOR_OVER);
    /* Border */
	wf::color_t color = active ? active_color : inactive_color;
    cairo_set_source_rgba(cra, color.r, color.g, color.b, color.a);
    cairo_arc(cra, 0, 0, corner_radius, 0, M_PI / 2);
    cairo_line_to(cra, 0, 0);
    cairo_fill(cra);
    /* Outline */
	color = active ? active_outline_color : inactive_outline_color;
    cairo_set_source_rgba(cra, color.r, color.g, color.b, color.a);
    cairo_set_line_width(cra, outline_size);
    cairo_arc(cra, 0, 0, outline_radius, 0, M_PI / 2);
    cairo_stroke(cra);

    return corner_surface;
}

/**
 * Fill the given rectangle with the background color(s).
 *
 * @param fb The target framebuffer, must have been bound already
 * @param rectangle The rectangle to redraw.
 * @param scissor The GL scissor rectangle to use.
 * @param active Whether to use active or inactive colors
 */
void decoration_theme_t::render_rectangles(const wf::framebuffer_t& fb,
    wf::geometry_t rectangle, const wf::geometry_t& scissor, bool active) const {

    wf::color_t color = active ? active_color : inactive_color;
    wf::color_t color_outline = active ? active_outline_color : inactive_outline_color;

    wf::simple_texture_t frame_texture;

	const auto format = CAIRO_FORMAT_ARGB32;
    auto *surface = cairo_image_surface_create(format, rectangle.width, rectangle.height);
    auto cr = cairo_create(surface);
    //cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    
	if (corner_radius == 0) {
		// Background rectangle.
		cairo_rectangle(cr, 0, 0, rectangle.width, rectangle.height);
		cairo_fill_preserve(cr);

		// Outline.
		cairo_set_line_width(cr, outline_size);
		cairo_set_source_rgba(cr, color_outline.r, color_outline.g, color_outline.b, color_outline.a);
		cairo_stroke(cr);
	} else {
		// Background rectircle.
		// Bottom left corner.
		cairo_move_to(cr, corner_radius, 0);
		// Bottom right corner.
		cairo_line_to(cr, rectangle.width - corner_radius, 0);
		cairo_arc(cr, rectangle.width - corner_radius,  corner_radius, corner_radius, 
				  3 * M_PI / 2, 2 * M_PI);
		// Top right corner.
		cairo_arc(cr, rectangle.width - corner_radius,  rectangle.height - corner_radius, 
				  corner_radius, 
				  0, M_PI / 2);
		// Top left corner.
		cairo_arc(cr, corner_radius, rectangle.height - corner_radius, corner_radius, 
				  M_PI / 2, M_PI);
		// Loop back to top left corner.
		cairo_arc(cr, corner_radius, corner_radius, corner_radius, M_PI, 3 * M_PI / 2);
		cairo_fill(cr);

		// Outline
		double outline_radius = corner_radius - outline_size / 2;
		cairo_set_line_width(cr, outline_size);
		cairo_set_source_rgba(cr, color_outline.r, color_outline.g, color_outline.b, color_outline.a);
		// Bottom left corner.
		cairo_move_to(cr, outline_radius, outline_size / 2);
		// Bottom right corner.
		cairo_line_to(cr, rectangle.width - outline_radius, outline_size / 2);
		cairo_arc(cr, rectangle.width - corner_radius, corner_radius, outline_radius,  3 * M_PI / 2,
		          2 * M_PI);
		// Top right corner.
		cairo_arc(cr, rectangle.width - corner_radius, rectangle.height - corner_radius, outline_radius, 
				  0, M_PI / 2);
		// Top left corner.
		cairo_arc(cr, corner_radius, rectangle.height - corner_radius, outline_radius, M_PI / 2, M_PI);
		// Loop back to top left corner.
		cairo_arc(cr, corner_radius, corner_radius, outline_radius, M_PI, 3 * M_PI / 2);
		cairo_stroke(cr);
	}
	cairo_destroy(cr);
   
    OpenGL::render_begin(fb);
    fb.logic_scissor(scissor);
	cairo_surface_upload_to_texture(surface, frame_texture);
    OpenGL::render_texture(frame_texture.tex, fb, rectangle, glm::vec4(1.0f));
    OpenGL::render_end();
}


cairo_surface_t *decoration_theme_t::get_button_surface(button_type_t button,
    const button_state_t& state) const {
    cairo_surface_t *button_surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, state.width, state.height);

    auto cr = cairo_create(button_surface);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

    /* Clear the button background */
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_rectangle(cr, 0, 0, state.width, state.height);
    cairo_fill(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    /** A gray that looks good on light and dark themes */
    color_t base = {0.60, 0.60, 0.63, 0.36};

    /**
     * We just need the alpha component.
     * r == g == b == 0.0 will be directly set
     */
    double line  = 0.27;
    double hover = 0.27;

    /** Coloured base on hover/press. Don't compare float to 0 */
    if (fabs(state.hover_progress) > 1e-3) {
        switch (button) {
          case BUTTON_CLOSE:
            base = {242.0 / 255.0, 80.0 / 255.0, 86.0 / 255.0, 0.63};
            break;

          case BUTTON_TOGGLE_MAXIMIZE:
            base = {57.0 / 255.0, 234.0 / 255.0, 73.0 / 255.0, 0.63};
            break;

          case BUTTON_MINIMIZE:
            base = {250.0 / 255.0, 198.0 / 255.0, 54.0 / 255.0, 0.63};
            break;

          default:
            assert(false);
        }

        line *= 2.0;
    }

    /** Draw the base */
    cairo_set_source_rgba(cr,
        base.r + 0.0 * state.hover_progress,
        base.g + 0.0 * state.hover_progress,
        base.b + 0.0 * state.hover_progress,
        base.a + hover * state.hover_progress);
    cairo_arc(cr, state.width / 2, state.height / 2,
        state.width / 2, 0, 2 * M_PI);
    cairo_fill(cr);

    /** Draw the border */
    cairo_set_line_width(cr, state.border);
    cairo_set_source_rgba(cr, 0.00, 0.00, 0.00, line);
    // This renders great on my screen (110 dpi 1376x768 lcd screen)
    // How this would appear on a Hi-DPI screen is questionable
    double r = state.width / 2 - 0.5 * state.border;
    cairo_arc(cr, state.width / 2, state.height / 2, r, 0, 2 * M_PI);
    cairo_stroke(cr);

    /** Draw the icon  */
    cairo_set_source_rgba(cr, 0.00, 0.00, 0.00, line / 2);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    switch (button) {
      case BUTTON_CLOSE:
        cairo_set_line_width(cr, 1.5 * state.border);
        cairo_move_to(cr, 1.0 * state.width / 4.0,
            1.0 * state.height / 4.0);
        cairo_line_to(cr, 3.0 * state.width / 4.0,
            3.0 * state.height / 4.0); // '\' part of x
        cairo_move_to(cr, 3.0 * state.width / 4.0,
            1.0 * state.height / 4.0);
        cairo_line_to(cr, 1.0 * state.width / 4.0,
            3.0 * state.height / 4.0); // '/' part of x
        cairo_stroke(cr);
        break;

      case BUTTON_TOGGLE_MAXIMIZE:
        cairo_set_line_width(cr, 1.5 * state.border);
        cairo_rectangle(
            cr, // Context
            state.width / 4.0, state.height / 4.0, // (x, y)
            state.width / 2.0, state.height / 2.0 // w x h
        );
        cairo_stroke(cr);
        break;

      case BUTTON_MINIMIZE:
        cairo_set_line_width(cr, 1.75 * state.border);
        cairo_move_to(cr, 1.0 * state.width / 4.0,
            state.height / 2.0);
        cairo_line_to(cr, 3.0 * state.width / 4.0,
            state.height / 2.0);
        cairo_stroke(cr);
        break;

      default:
        assert(false);
    }

    cairo_fill(cr);
    cairo_destroy(cr);

    return button_surface;
}
}
}
