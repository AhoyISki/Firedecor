#pragma once

#include <cairo.h>
#include <wayfire/geometry.hpp>
#include <wayfire/config/types.hpp>

void cairo_move_to(cairo_t *cr, wf::point_t point) {
    cairo_move_to(cr, point.x, point.y);
}

void cairo_line_to(cairo_t *cr, wf::point_t point) {
    cairo_line_to(cr, point.x, point.y);
}

void cairo_arc(cairo_t *cr, wf::point_t point, double r, double a1, double a2) {
    cairo_arc(cr, point.x, point.y, r, a1, a2);
}

void cairo_rectangle(cairo_t *cr, wf::point_t point, wf::dimensions_t size) {
    cairo_rectangle(cr, point.x, point.y, size.width, size.height);
}

void cairo_rectangle(cairo_t *cr, wf::point_t point, int width, int height) {
    cairo_rectangle(cr, point.x, point.y, width, height);
}

void cairo_rectangle(cairo_t *cr, wf::geometry_t geometry) {
    cairo_rectangle(cr, geometry.x, geometry.y, geometry.width, geometry.height);
}

void cairo_set_source_rgba(cairo_t *cr, wf::color_t color) {
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
}

void cairo_translate(cairo_t *cr, wf::point_t point) {
    cairo_translate(cr, (double)point.x, (double)point.y);
}
