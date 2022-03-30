firedecor = shared_module(
	'firedecor', [ 'firedecor.cpp', 'firedecor-subsurface.cpp',
				   'firedecor-buttons.cpp', 'firedecor-layout.cpp',
			       'firedecor-theme.cpp' ],
    dependencies: [ wlroots, wfconfig, rsvg , pixman, glib, gdk_pixbuf, cairo, pango,
					pangocairo],
    install: true, install_dir: join_paths(wayfire.get_variable(pkgconfig: 'libdir'), 'wayfire'))
