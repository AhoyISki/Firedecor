project(
	'firedecor',
	'cpp',
	version: '0.1',
	license: 'MIT',
	meson_version: '>=0.51.0',
	default_options: [
		'cpp_std=c++20',
		'warning_level=2',
		'werror=false',
	],
)

wayfire    = dependency('wayfire')
wlroots    = dependency('wlroots')
wfconfig   = dependency('wf-config')
pixman     = dependency('pixman-1')
cairo      = dependency('cairo')
pango      = dependency('pango')
pangocairo = dependency('pangocairo')

add_project_arguments(['-DWLR_USE_UNSTABLE'], language: ['cpp', 'c'])
add_project_arguments(['-DWAYFIRE_PLUGIN'], language: ['cpp', 'c'])
add_project_link_arguments(['-rdynamic'], language:'cpp')

subdir('src')
subdir('metadata')

summary = [
	'',
	'----------------',
	'firedecor @0@'.format(meson.project_version()),
	'----------------',
	''
]
message('\n'.join(summary))
