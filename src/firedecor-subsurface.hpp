#pragma once

#include <wayfire/view.hpp>

#include "firedecor-theme.hpp"

void init_view(wayfire_view view, wf::firedecor::extra_options_t options);
void deinit_view(wayfire_view view);
