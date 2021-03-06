#pragma once

#include <wayfire/view.hpp>

#include "firedecor-theme.hpp"

namespace wf::firedecor {

void init_view(wayfire_view view, wf::firedecor::theme_options options);
void deinit_view(wayfire_view view);

}
