project('vrm-viewer', 'c', 'cpp',
  version: '0.1',
  default_options: ['cpp_std=c++17', 'cpp_link_args=-fuse-ld=mold']
)

cc = meson.get_compiler('c')
ccx = meson.get_compiler('cpp')

c_args = []
cpp_args = []
link_args = []
name_suffix = []
dependencies = []

# Raylib config
cmake = import('cmake')
raylib_opts = cmake.subproject_options()
raylib_opts.set_install(false)
raylib_opts.add_cmake_defines({
  'BUILD_EXAMPLES': 'OFF',
  # Add other CMake options for Raylib, if needed
})
if cc.get_id() == 'emscripten'
  raylib_opts.add_cmake_defines({
    'PLATFORM': 'Web',
  })
else
  raylib_opts.add_cmake_defines({
    'PLATFORM': 'Desktop',
  })
endif
raylib_subproject = cmake.subproject('raylib', options: raylib_opts)
dependencies += raylib_subproject.dependency('raylib')

# General configuration
if host_machine.system() == 'windows'
  dependencies += [
    cc.find_library('winmm'),
  ]
elif host_machine.system() == 'darwin'
  link_args += [
    '-framework', 'AppKit',
    '-framework', 'IOKit',
  ]
elif host_machine.system() == 'linux'
  dependencies += [
    cc.find_library('m'),
    cc.find_library('dl'),
  ]
elif host_machine.system() == 'emscripten'
  link_args += [
    '-s', 'ENVIRONMENT=web',
    '-s', 'USE_GLFW=3',
  ]
  name_suffix = 'html'
endif

# Project files
inc_dir = include_directories(
  'subprojects/vrm/include',
  'subprojects/stb',
  'src',
)

sources = [
  'src/utils.cpp',
  'src/mesh.cpp',
  'src/animation.cpp',
  'src/main.cpp',
]

executable('vrm', sources,
  c_args: c_args,
  cpp_args: cpp_args,
  dependencies: dependencies,
  include_directories: inc_dir,
  link_args: link_args,
  name_suffix: name_suffix,
)
