project(
	'firedecor',
	'c',
	'cpp',
	version: '0.1',
	license: 'MIT',
	meson_version: '>=0.51.0',
	default_options: [
		'cpp_std=c++17',
        'c_std=c11',
		'warning_level=2',
		'werror=false',
		#'wcomment=false'
	],
)

wayfire    = dependency('wayfire')
wlroots    = dependency('wlroots')
wfconfig   = dependency('wf-config')
rsvg       = dependency('librsvg-2.0')
pixman     = dependency('pixman-1')
cairo      = dependency('cairo')
pango      = dependency('pango')
pangocairo = dependency('pangocairo')
glib       = dependency('glib-2.0')
gdk_pixbuf = dependency('gdk-pixbuf-2.0')

add_project_arguments(['-DWLR_USE_UNSTABLE'], language: ['cpp', 'c'])
add_project_arguments(['-DWAYFIRE_PLUGIN'], language: ['cpp', 'c'])
add_project_link_arguments(['-rdynamic'], language:'cpp')

subdir('src')
subdir('metadata')

install_data('src/firedecor.xml', install_dir: wayfire.get_variable(pkgconfig: 'metadatadir'))

summary = [
	'',
	'----------------',
	'firedecor @0@'.format(meson.project_version()),
	'----------------',
	''
]
message('\n'.join(summary))
