firedecor = shared_module(
	'firedecor', [ 'firedecor.cpp', 'firedecor-subsurface.cpp',
				   'firedecor-buttons.cpp', 'firedecor-layout.cpp',
			       'firedecor-theme.cpp' ],
    dependencies: [ wf_config, wlroots, rsvg , pixman, glib, gdk_pixbuf, cairo, pango,
					pangocairo],
    install: true, install_dir: wayfire.get_variable(pkgconfig: 'plugindir'))
