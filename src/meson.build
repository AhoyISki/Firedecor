firedecor = shared_module('firedecor',
    ['firedecor.cpp', 'firedecor-buttons.cpp', 'firedecor-theme.cpp', 'firedecor-layout.cpp', 'firedecor-subsurface.cpp'],
    #include_directories: [wayfire_api_inc, wayfire_conf_inc, plugins_common_inc],
    dependencies: [wlroots, pixman, wfconfig, cairo, pango, pangocairo],
    install: true,
    install_dir: join_paths(get_option('libdir'), 'wayfire'))
