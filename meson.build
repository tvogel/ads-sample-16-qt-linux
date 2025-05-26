project('sample16', 'cpp',
  version : '0.1',
  default_options : [
    'buildtype=debug',
    'warning_level=3',
    'werror=true',
    'b_pie=true',
  ]
)

# some hardening options
# add_project_arguments('-D_FORTIFY_SOURCE=2', language: 'cpp')

cxx = meson.get_compiler('cpp')
libs = [
    cxx.find_library('ws2_32', required: false),
    dependency('threads'),
]

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets', 'Core5Compat'])
inc = include_directories('../AdsLib')
moc_files = qt6.compile_moc(headers : 'Sample16Dlg.h',
                            include_directories: inc,
                            dependencies: qt6_dep)
ui_files = qt6.compile_ui(sources : 'Sample16Dlg.ui', preserve_paths: true)

sources = ['Sample16.cpp', 'Sample16Dlg.cpp', 'AdsParseSymbols.cpp', 'AdsDatatypeEntry.cpp']

sample16 = executable('sample16',
  [sources, ui_files, moc_files],
  include_directories: inc,
  dependencies: [
    cxx.find_library('AdsLib', dirs: meson.project_source_root() + '/../build/'),
    libs,
    qt6_dep,
  ]
)

if get_option('tcadsdll_lib') != ''
  libs += cxx.find_library('TcAdsLib', dirs: meson.project_source_root() + '/../build/')
  libs += cxx.find_library('TcAdsDll', dirs: get_option('tcadsdll_lib'))
  inc += get_option('tcadsdll_include')
else
  libs = disabler()
endif

tcsample16 = executable('tcsample16',
  [sources, ui_files, moc_files],
  cpp_args: '-DUSE_TWINCAT_ROUTER',
  include_directories: inc,
  dependencies: [
    libs,
    qt6_dep,
  ]
)
