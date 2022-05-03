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

#include "cairo-simpler.hpp"

#include <wayfire/plugins/common/cairo-util.hpp>

//#include <cairo.h>

#define INACTIVE 0
#define ACTIVE 1
#define FORCE true
#define DONT_FORCE false

#include <fstream>

namespace wf::firedecor {

class simple_decoration_surface : public surface_interface_t,
	public compositor_surface_t {
	bool _mapped = true;
    wayfire_view view;

    signal_connection_t title_set = [=, this] (signal_data_t *data) {
        if (get_signaled_view(data) == view) {
            update_layout(FORCE);
            view->damage(); // trigger re-render
        }
    };

    void update_title(double scale) {
		cairo_surface_t *surface;
		dimensions_t title_size = {
    		(int)(title.dims.width * scale), (int)(title.dims.height * scale)
        };
		dimensions_t dots_size = {
    		(int)(title.dots_dims.width * scale),
    		(int)(title.dots_dims.height * scale)
        };

		auto o = HORIZONTAL;
        surface = theme.form_title(title.text, title_size, ACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.hor[ACTIVE]);
        surface = theme.form_title(title.text, title_size, INACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.hor[INACTIVE]);
        if (!title.dots_set && title.dots_dims.width > 0) {
            surface = theme.form_title("...", dots_size, ACTIVE, o);
            cairo_surface_upload_to_texture(surface, title.hor_dots[ACTIVE]);
            surface = theme.form_title("...", dots_size, INACTIVE, o);
            cairo_surface_upload_to_texture(surface, title.hor_dots[INACTIVE]);
        }
		o = VERTICAL;
		surface = theme.form_title(title.text, title_size, ACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.ver[ACTIVE]);
        surface = theme.form_title(title.text, title_size, INACTIVE, o);
        cairo_surface_upload_to_texture(surface, title.ver[INACTIVE]);
        if (!title.dots_set && title.dots_dims.width > 0) {
            surface = theme.form_title("...", dots_size, ACTIVE, o);
            cairo_surface_upload_to_texture(surface, title.ver_dots[ACTIVE]);
            surface = theme.form_title("...", dots_size, INACTIVE, o);
            cairo_surface_upload_to_texture(surface, title.ver_dots[INACTIVE]);

            title.dots_set = true;
        }
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
        if ((title.colors != theme.get_title_colors()) || force) {
            /** Updating the cached variables */
            title.colors = theme.get_title_colors();
	        title.text = (theme.get_debug_mode()) ? "a" : view->get_title();

	        wf::dimensions_t cur_size = theme.get_text_size(title.text, size.width);

	        if (theme.get_debug_mode()) {
    	        title.text = view->get_app_id() + " " +
    	                     std::to_string(cur_size.height) + "px";
    	        cur_size = theme.get_text_size(title.text, size.width);
	        }

	        title.dims.height = cur_size.height;

	        if (cur_size.width <= theme.get_max_title_size()) {
    	        title.dims.width = cur_size.width;
    	        title.too_big = false;
    	        title.dots_dims = { 0, 0 };
	        } else {
                wf::dimensions_t dots_size = theme.get_text_size("...", size.width);

    	        title.dims.width = theme.get_max_title_size() - dots_size.width;
    	        title.too_big = true;
    	        title.dots_dims = dots_size;
	        }

		    title_needs_update = true;

            /** Necessary in order to immediately place areas correctly */
    		layout.resize(size.width, size.height, title.dims, title.dots_dims);
        }

    }

    /** Title variables */
    struct {
        simple_texture_t hor[2], hor_dots[2];
        simple_texture_t ver[2], ver_dots[2];
        std::string text = "";
        color_set_t colors;
        dimensions_t dims, dots_dims;
        bool dots_set = false, too_big = true;
    } title;

    bool title_needs_update = false;

    /** Icon variables */
    struct {
	    simple_texture_t texture;
	    std::string app_id = "";
    } icon;

    /** Corner variables */
    struct corner_texture_t {
	    simple_texture_t tex[2];
	    cairo_surface_t *surf[2];
	    geometry_t g;
	    int r;
    };

    struct {
        corner_texture_t tr, tl, bl, br;
    } corners;

    /** Edge variables */
    struct edge_colors_t {
	    color_set_t border, outline;
    } edges;

    /** Accent variables */
    struct accent_texture_t {
        simple_texture_t t_trbr[2];
        simple_texture_t t_tlbl[2];
        int radius;
    };

    std::vector<accent_texture_t> accent_textures;

    /** Other general variables */
    decoration_theme_t theme;
    decoration_layout_t layout;
    region_t cached_region;
    dimensions_t size;

	void update_corners(edge_colors_t colors, int corner_radius, double scale) {
		if ((this->corner_radius != corner_radius) ||
			(edges.border != colors.border) ||
			(edges.outline != colors.outline)) {
    		corners.tr.r = corners.tl.r = corners.bl.r = corners.br.r = 0;

    		std::stringstream round_on_str(theme.get_round_on());
    		std::string corner;
    		while (round_on_str >> corner) {
        		if (corner == "all") {
            		corners.tr.r = corners.tl.r = corners.bl.r = corners.br.r
        		                 = theme.get_corner_radius() * scale;
            		break;
        		} else if (corner == "tr") {
            		corners.tr.r = (theme.get_corner_radius() * scale);
        		} else if (corner == "tl") {
            		corners.tl.r = (theme.get_corner_radius() * scale);
        		} else if (corner == "bl") {
            		corners.bl.r = (theme.get_corner_radius() * scale);
        		} else if (corner == "br") {
            		corners.br.r = (theme.get_corner_radius() * scale);
        		}
    		}
    		int height = std::max( { corner_radius, border_size.top,
    		                         border_size.bottom });
    		auto create_s_and_t = [&](corner_texture_t& t, matrix<double> m, int r) {
        		for (auto a : { ACTIVE, INACTIVE }) {
            		t.surf[a] = theme.form_corner(a, r, m, height);
        			cairo_surface_upload_to_texture(t.surf[a], t.tex[a]);
        		}
    		};
    		/** The transformations are how we create 4 different corners */
    		create_s_and_t(corners.tr, { scale, 0, 0, scale }, corners.tr.r);
    		create_s_and_t(corners.tl, { -scale, 0, 0, scale }, corners.tl.r);
    		create_s_and_t(corners.bl, { -scale, 0, 0, -scale }, corners.bl.r);
    		create_s_and_t(corners.br, { scale, 0, 0, -scale }, corners.br.r);

    		corners.tr.g = { size.width - corner_radius, 0,
    		                     corner_radius, height };
    		corners.tl.g = { 0, 0, corner_radius, height };
    		corners.bl.g = { 0, size.height - height, corner_radius, height };
    		corners.br.g = { size.width - corner_radius, size.height - height,
    		                     corner_radius, height };

			edges.border.active    = colors.border.active;
			edges.border.inactive  = colors.border.inactive;
			edges.outline.active   = colors.outline.active;
			edges.outline.inactive = colors.outline.inactive;
			this->corner_radius    = corner_radius;
		}
	}

  public:
    border_size_t border_size;
    int corner_radius;

    simple_decoration_surface(wayfire_view view, theme_options options)
      : theme{options}, 
    	layout{theme, [=, this] (wlr_box box) {this->damage_surface_box(box); }} {
        this->view = view;
        view->connect_signal("title-changed", &title_set);

        // make sure to hide frame if the view is fullscreen
        update_decoration_size();
    }
    
    virtual bool is_mapped() const final {
        return _mapped;
    }

    point_t get_offset() final {
        return { -border_size.left, -border_size.top };
    }

    virtual dimensions_t get_size() const final {
        return size;
    }

    void render_title(const framebuffer_t& fb, geometry_t geometry,
                      geometry_t dots_geometry, edge_t edge, geometry_t scissor) {
	    if (title_needs_update) {
    	    update_title(fb.scale);
	    }

	    simple_texture_t *texture, *dots_texture;
	    uint32_t bits = 0;
	    if (edge == EDGE_TOP || edge == EDGE_BOTTOM) {
	        bits = OpenGL::TEXTURE_TRANSFORM_INVERT_Y;
	        texture = &title.hor[view->activated];
	        dots_texture = &title.hor_dots[view->activated];
	    } else {
    	    texture = &title.ver[view->activated];
	        dots_texture = &title.ver_dots[view->activated];
	    }

		OpenGL::render_begin(fb);
        fb.logic_scissor(scissor);
        OpenGL::render_texture(texture->tex, fb, geometry, glm::vec4(1.0f), bits);
        std::ofstream test{"/home/mateus/Development/wayfire-firedecor/test", std::ofstream::app};
        test << "title: " << geometry << std::endl;
        if (title.too_big) {
            OpenGL::render_texture(dots_texture->tex, fb, dots_geometry,
                                   glm::vec4(1.0f), bits);
        }
		OpenGL::render_end();
    }

    void render_icon(const framebuffer_t& fb, geometry_t g,
                     const geometry_t& scissor, int32_t bits) {
        update_icon();
		OpenGL::render_begin(fb);
		fb.logic_scissor(scissor);
		OpenGL::render_texture(icon.texture.tex, fb, g, glm::vec4(1.0f), bits);
		OpenGL::render_end();
    }

    color_t alpha_trans(color_t c) {
	    return { c.r * c.a, c.g * c.a, c.b * c.a, c.a };
    }

    void form_accent_corners(int r, geometry_t accent, std::string corner_style,
                             matrix<int> m, edge_t edge) {
        const auto format = CAIRO_FORMAT_ARGB32;
        cairo_surface_t *surfaces[4];
        double angle = 0;

        /** Colors of the accent and background, respectively */
        color_t a_color;
        color_t b_color;

        wf::point_t a_origin = { accent.x, accent.y };

        int h = std::max({ corner_radius, border_size.top, border_size.bottom });

        wf::geometry_t a_edges[2];
        if (m.xx == 1) {
            a_edges[0] = { 0, 0, r, accent.height };
            a_edges[1] = { accent.width - r, 0, r, accent.height };
        } else {
            a_edges[0] = { 0, 0, accent.width, r };
            a_edges[1] = { 0, accent.height - r, accent.width, r };
        }

        wf::geometry_t cut;
        if (m.xy == 0) { 
            cut = { accent.x + r, accent.y, accent.width - 2 * r, accent.height };
        } else {
            cut = { accent.x, accent.y + r, accent.width, accent.height - 2 * r };
        }

        /**** Creation of the master path, containing all accent edge textures */
        const cairo_matrix_t matrix = {
            (double)m.xx, (double)m.xy, (double)m.yx, (double)m.yy, 0, 0
        };

        /** Array used to determine if a line starts on the corner or not */
        struct { int tr = 0, br = 0, bl = 0, tl = 0; } retract;

        /** Calculate where to retract, based on diagonality */
        for (int i = 0; auto c : corner_style) {
            if (c == '/') {
                if (i == 0) { 
                    retract.tl = r ;
                } else {
                    retract.br = r;
                }
                i++;
            } else if (c == '\\') {
                if (i == 0) { 
                    retract.bl = r ;
                } else {
                    retract.tr = r;
                }
                i++;
            } else if (c == '!') {
                i++;
            }
        }

        /** "Untransformed" accent area, used for correct transformations later on */
        const wf::dimensions_t mod_a = {
            abs(accent.width * m.xx + accent.height * m.xy),
            abs(accent.width * m.yx + accent.height * m.yy)
        };

        auto full_surface = cairo_image_surface_create(format, accent.width,
                                                       accent.height);
        auto cr = cairo_create(full_surface);

        /** A corner indexing array and the string with the final corners to round */
        std::string c_strs[4] = { "br", "tr", "tl", "bl" };
        std::string to_round;

        /** Deciding which corners to round, based on the string and on rotation */
        if (corner_style == "a") {
            to_round = "brtrtlbl";
            retract.br = r;
        } else {
            /** True mathematical modulo */
            auto modulo = [](int a, int b) -> int {
               return a - b * floor((double)a / b);
            };

            for (int c_to_rotate = 0; auto str : c_strs) {
               if (corner_style.find(str) != std::string::npos) {

                   /**
                    * This function effectively rotates the chosen corner.
                    * When m.xy == 1 (left edge), br becomes tr, tr becomes tl, etc.
                    * On the right edge, the opposite happens.
                    * This is to keep the correct corners rounded for the end user.
                    */
                   to_round += c_strs[modulo(c_to_rotate - m.xy, 4)];
                   if (modulo(c_to_rotate - m.xy, 4) == 0) { retract.br = r; }
               }
               c_to_rotate++;
            }
        }

        /** Point of rotation, in case it is needed */
        int rotation_x_d = ((m.xy == 1) ? mod_a.height : mod_a.width ) / 2;
        wf::point_t rotation_point = { rotation_x_d, rotation_x_d };

        /** Rotation depending on the edge */
        cairo_translate(cr, rotation_point);
        cairo_transform(cr, &matrix);
        cairo_translate(cr, -rotation_point);

        /** Lambda that creates a rounded or flat corner */
        auto create_corner = [&](int w, int h, int i) {
            if (to_round.find(c_strs[i]) != std::string::npos) {
                cairo_arc(cr, w + ((i < 2) ? -r : r), h + ((i % 3 == 0) ? r : -r), r,
                          M_PI_2 * (i - 1), M_PI_2 * i);
            } else {
                if (i % 2 == 0) { cairo_line_to(cr, w, h); }
                cairo_line_to(cr, w, h + ((i == 0) ? r : ((i == 2) ? -r : 0)));
            }
        };

        cairo_move_to(cr, mod_a.width - retract.br, 0);
        if (to_round.find("r") == std::string::npos) {
            cairo_line_to(cr, mod_a.width - retract.tr, mod_a.height);
        } else {
            create_corner(mod_a.width, 0, 0);
            create_corner(mod_a.width, mod_a.height, 1);
        }
        if (to_round.find("l") == std::string::npos) {
            cairo_line_to(cr, retract.tl, mod_a.height);
            cairo_line_to(cr, retract.bl, 0);
        } else {
            create_corner(0, mod_a.height, 2);
            create_corner(0, 0, 3);
        }
        cairo_close_path(cr);
        auto master_path = cairo_copy_path(cr);
        cairo_destroy(cr);
        cairo_surface_destroy(full_surface);
        /****/
        
        for (int i = 0, j = 0; i < 4; i++, angle += M_PI / 2, j = i % 2) {
            if (i < 2) {
                a_color = theme.get_accent_colors().inactive;
                b_color = theme.get_border_colors().inactive;
            } else {
                a_color = theme.get_accent_colors().active;
                b_color = theme.get_border_colors().active;
            }
            int width = a_edges[j].width;
            int height = a_edges[j].height;
                
            surfaces[i] = cairo_image_surface_create(format, width, height);
            auto cr_a = cairo_create(surfaces[i]);

            /** Background rectangle, behind the accent's corner */
            cairo_set_source_rgba(cr_a, b_color);
            cairo_rectangle(cr_a, 0, 0, a_edges[j].width, a_edges[j].height);
            cairo_fill(cr_a);

            /**** Outline, done early so it can also be cropped off */
            /** Translation to the correct rotation */
            cairo_translate(cr_a , rotation_point); 
            cairo_transform(cr_a , &matrix);
            cairo_translate(cr_a , -rotation_point); 

            int o_size = theme.get_outline_size();
            auto outline_color = (view->activated) ?
                                 alpha_trans(theme.get_outline_colors().active) :
                                 alpha_trans(theme.get_outline_colors().inactive);


            /** Draw outline on the bottom in case it is in the bottom edge */
            int h_offset = (edge == EDGE_BOTTOM) ? mod_a.height : o_size;

            cairo_set_source_rgba(cr_a, outline_color);
            cairo_rectangle(cr_a, 0, mod_a.height - h_offset, mod_a.width, o_size);
            cairo_fill(cr_a);
            /****/

            /** Dealing with intersection between the view's corners and accent */
            for (auto *c : { &corners.tr, &corners.tl, &corners.bl, &corners.br } ) {

                /** Accent edge translated by the accent's origin */
                wf::geometry_t a_edge = a_edges[j] + a_origin;

                /** Skip everything if the areas don't intersect */
                auto in = geometry_intersection(a_edge, c->g);
                if (in.width == 0 || in.height == 0) { continue; }

                /**** Rectangle to cut the background from the accent's corner */
                /** View's corner position relative to the accent's corner */
                point_t v_rel_a = { 
                    c->g.x - a_edge.x, c->g.y - a_edge.y
                };

                cairo_set_operator(cr_a, CAIRO_OPERATOR_CLEAR);
                cairo_rectangle(cr_a, v_rel_a, corner_radius, h);
                cairo_fill(cr_a);
                /****/

                /** Removal of intersecting areas from the view corner */
                for (auto active : { ACTIVE, INACTIVE }) {

                    /**** Removing the edges with cut, flat, or diagonal corners */
                    /** Surface to remove from, the view corner in this case */
                    auto cr_v = cairo_create(c->surf[active]);
                    cairo_set_operator(cr_v, CAIRO_OPERATOR_CLEAR);

                    /** Transformed br accent corner, relative to the view corner */
                    wf::point_t t_br_rel_v = {
                        (a_origin.x) - c->g.x,
                        (c->g.y + c->g.height) - (a_origin.y + accent.height)
                    };

                    cairo_translate(cr_v, t_br_rel_v.x, t_br_rel_v.y);

                    cairo_translate(cr_v , rotation_point); 
                    cairo_transform(cr_v , &matrix);
                    cairo_translate(cr_v , -rotation_point); 

                    cairo_append_path(cr_v, master_path);
                    cairo_fill(cr);
                    /****/

                    /**** Clear the view's corner with the accent rectangles */
                    /** Bottom of the rectangle relative to the view's corner */
                    wf::point_t reb_rel_v;
                    reb_rel_v.y = (c->g.y + c->g.height) - (cut.y + cut.height);
                    reb_rel_v.x = (cut.x - c->g.x);

                    cairo_set_operator(cr_v, CAIRO_OPERATOR_CLEAR);
                    cairo_rectangle(cr_v, reb_rel_v, cut.width, cut.height);
                    cairo_fill(cr_v);
                    /****/
                            
                    cairo_surface_upload_to_texture(c->surf[active], c->tex[active]);
                    cairo_destroy(cr_v);
                }
            }

            /**** Final drawing of accent corner, overlaying the drawn rectangle */
            cairo_set_operator(cr_a, CAIRO_OPERATOR_SOURCE);

            wf::point_t t_br = {
                (r - mod_a.width) * (m.xx * (i % 2) + m.xy * (1 - i % 2)), 0
            };

            cairo_set_source_rgba(cr_a, a_color);
            cairo_translate(cr_a, t_br.x, t_br.y);
            cairo_append_path(cr_a, master_path);
            cairo_fill(cr_a);
            cairo_destroy(cr_a);
            /****/
        }
        auto& texture = accent_textures.back();
        cairo_surface_upload_to_texture(surfaces[0], texture.t_trbr[INACTIVE]);
        cairo_surface_upload_to_texture(surfaces[1], texture.t_tlbl[INACTIVE]);
        cairo_surface_upload_to_texture(surfaces[2], texture.t_trbr[ACTIVE]);
        cairo_surface_upload_to_texture(surfaces[3], texture.t_tlbl[ACTIVE]);
        texture.radius = r;

        for (auto surface : surfaces) { cairo_surface_destroy(surface); }
        cairo_path_destroy(master_path);
    }

    void render_background_area(const framebuffer_t& fb, geometry_t g,
                                point_t rect, geometry_t scissor,
                                std::string rounded, unsigned long i,
                                decoration_area_type_t type, matrix<int> m,
                                edge_t edge) {
        /** The view's origin */                            
        point_t o = { rect.x, rect.y };

        if (type == DECORATION_AREA_ACCENT) {
            /**** Render the corners of an accent */
            int r;

            /** Create the corners, it should happen once per accent */
            if (accent_textures.size() <= i) {
                accent_textures.resize(i + 1);
                r = std::min({ ceil((double)g.height / 2), ceil((double)g.width / 2),
                               (double)corner_radius});
                form_accent_corners(r, g, rounded, m, edge);
            }

            r = accent_textures.at(i).radius;

            geometry_t a_edges[2];
            if (m.xx == 1) {
                a_edges[0] = { g.x, g.y, r, g.height };
                a_edges[1] = { g.x + g.width - r, g.y, r, g.height };
            } else {
                a_edges[0] = { g.x, g.y, g.width, r };
                a_edges[1] = { g.x, g.y + g.height - r, g.width, r };
            }

            OpenGL::render_begin(fb);
            fb.logic_scissor(scissor);
            OpenGL::render_texture(accent_textures.at(i).t_trbr[view->activated].tex,
                                   fb, a_edges[0] + o, glm::vec4(1.0));
            OpenGL::render_texture(accent_textures.at(i).t_tlbl[view->activated].tex,
                                   fb, a_edges[1] + o, glm::vec4(1.0));
            /****/

            /**** Render the internal rectangles of the accent */
            wf::geometry_t accent_rect;
            if (m.xy == 0) {
                accent_rect = { g.x + r, g.y, g.width - 2 * r, g.height };
            } else {
                accent_rect = { g.x, g.y + r, g.width, g.height - 2 * r };
            }

            color_t color = (view->activated) ? 
                            alpha_trans(theme.get_accent_colors().active) :
                            alpha_trans(theme.get_accent_colors().inactive);
            OpenGL::render_rectangle(accent_rect + o, color,
                                     fb.get_orthographic_projection());
            /****/
            OpenGL::render_end();
        } else {
            /**** Render a single rectangle when the area is a background */
            color_t color = (view->activated) ? 
                            alpha_trans(theme.get_border_colors().active) :
                            alpha_trans(theme.get_border_colors().inactive);
    		color_t o_color = (view->activated) ? 
    	                      alpha_trans(theme.get_outline_colors().active) :
    		                  alpha_trans(theme.get_outline_colors().inactive);

            wf::geometry_t g_o;
            int o_s = theme.get_outline_size();
            if (edge == wf::firedecor::EDGE_TOP) {
                g_o = { g.x, g.y, g.width, o_s };
            } else if (edge == wf::firedecor::EDGE_LEFT) {
                g_o = { g.x, g.y, o_s, g.height };
            } else if (edge == wf::firedecor::EDGE_BOTTOM) {
                g_o = { g.x, g.y + g.height - o_s, g.width, o_s };
            } else if (edge == wf::firedecor::EDGE_RIGHT) {
                g_o = { g.x + g.width - o_s, g.y, o_s, g.height };
            }
            g_o = g_o + o;

            OpenGL::render_begin(fb);
            fb.logic_scissor(scissor);
            OpenGL::render_rectangle(g + o, color, fb.get_orthographic_projection());
            OpenGL::render_rectangle(g_o, o_color, fb.get_orthographic_projection());
            OpenGL::render_end();
            /****/
        }
    }

	void render_background(const framebuffer_t& fb, geometry_t rect,
	                       const geometry_t& scissor) {
		edge_colors_t colors = {
			theme.get_border_colors(), theme.get_outline_colors()
		};

		colors.border.active = alpha_trans(colors.border.active);
		colors.border.inactive = alpha_trans(colors.border.inactive);

		int r = theme.get_corner_radius() * fb.scale;
		update_corners(colors, r, fb.scale);

		/** Borders */
		unsigned long i = 0;
		point_t rect_o = { rect.x, rect.y };
		for (auto area : layout.get_background_areas()) {
    		render_background_area(fb, area->get_geometry(), rect_o, scissor,
    		                       area->get_corners(), i, area->get_type(),
    		                       area->get_m(), area->get_edge());
    		i++;
		}

		OpenGL::render_begin(fb);
		fb.logic_scissor(scissor);

		/** Outlines */
        bool a = view->activated;
        point_t o = { rect.x, rect.y };
		/** Rendering all corners */
		for (auto *c : { &corners.tr, &corners.tl, &corners.bl, &corners.br }) {
    		OpenGL::render_texture(c->tex[a].tex, fb, c->g + o, glm::vec4(1.0f));
		}
		OpenGL::render_end();
	}

    void render_scissor_box(const framebuffer_t& fb, point_t origin,
                            const wlr_box& scissor) {
	    /** Draw the background (corners and border) */
        wlr_box geometry{origin.x, origin.y, size.width, size.height};
        render_background(fb, geometry, scissor);

        auto renderables = layout.get_renderable_areas();
        for (auto item : renderables) {
            int32_t bits = 0;
            if (item->get_edge() == EDGE_LEFT) {
                bits = OpenGL::TEXTURE_TRANSFORM_INVERT_Y; 
            } else if (item->get_edge() == EDGE_RIGHT) {
                bits = OpenGL::TEXTURE_TRANSFORM_INVERT_X;
            }
	        if (item->get_type() == DECORATION_AREA_TITLE) {
                render_title(fb, item->get_geometry() + origin,
                			 item->get_dots_geometry() + origin, item->get_edge(),
                             scissor);
            } else if (item->get_type() == DECORATION_AREA_BUTTON) {
	            item->as_button().set_active(view->activated);
	            item->as_button().set_maximized(view->tiled_edges);
                item->as_button().render(fb, item->get_geometry() + origin, scissor);
            } else if (item->get_type() == DECORATION_AREA_ICON) {
	            render_icon(fb, item->get_geometry() + origin, scissor, bits);
            }
        }
    }
    
    virtual void simple_render(const framebuffer_t& fb, int x, int y,
					           const region_t& damage) override {
        region_t frame = this->cached_region + (point_t){x, y};
        frame &= damage;

    	update_layout(DONT_FORCE);

        int h = std::max({ corner_radius, border_size.top, border_size.bottom });
        corners.tr.g = { size.width - corner_radius, 0, corner_radius, h };
        corners.bl.g = { 0, size.height - h, corner_radius, h };
        corners.br.g = { size.width - corner_radius,
                             size.height - h, corner_radius, h };

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
    void handle_action(decoration_layout_t::action_response_t action) {
        switch (action.action) {
          case DECORATION_ACTION_MOVE:
            return view->move_request();

          case DECORATION_ACTION_RESIZE:
            return view->resize_request(action.edges);

          case DECORATION_ACTION_CLOSE:
            return view->close();

          case DECORATION_ACTION_TOGGLE_MAXIMIZE:
            if (view->tiled_edges) {
                view->tile_request(0);
            } else {
                view->tile_request(TILED_EDGES_ALL);
            }

            break;

          case DECORATION_ACTION_MINIMIZE:
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
        emit_map_state_change(this);
    }

    void resize(dimensions_t dims) {
        view->damage();
        size = dims;
		layout.resize(size.width, size.height, title.dims, title.dots_dims);
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

class simple_decorator_t : public decorator_frame_t_t {
    wayfire_view view;
    nonstd::observer_ptr<simple_decoration_surface> deco;

  public:
    simple_decorator_t(wayfire_view view, theme_options options) {
        this->view = view;

        auto sub = std::make_unique<simple_decoration_surface>(view, options);
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

    signal_connection_t on_subsurface_removed = [&] (auto data) {
        auto ev = static_cast<subsurface_removed_signal*>(data);
        if (ev->subsurface.get() == deco.get()) {
            deco->unmap();
            deco = nullptr;
        }
    };

    virtual geometry_t expand_wm_geometry( geometry_t contained_wm_geometry) override {
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

    virtual void notify_view_resized(geometry_t view_geometry) override {
        deco->resize(dimensions(view_geometry));
    }

    virtual void notify_view_tiled() override {}

    virtual void notify_view_fullscreen() override {
        deco->update_decoration_size();

        if (!view->fullscreen) {
            notify_view_resized(view->get_wm_geometry());
        }
    }
};

void init_view(wayfire_view view, theme_options options) {
    auto firedecor = std::make_unique<simple_decorator_t>(view, options);
    view->set_decoration(std::move(firedecor));
}

void deinit_view(wayfire_view view) {
    view->set_decoration(nullptr);
}
}

