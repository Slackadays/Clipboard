set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_INCLUDE_DIR "${GENERATED_DIR}/include")
set(GENERATED_SRC_DIR "${GENERATED_DIR}/src")

find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

set(XDG_SHELL_PROTOCOL "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
set(GENERATED_XDG_SHELL_HEADER "${GENERATED_INCLUDE_DIR}/wayland-xdg-shell.hpp")
set(GENERATED_XDG_SHELL_CODE "${GENERATED_SRC_DIR}/wayland-xdg-shell.c")

file(MAKE_DIRECTORY
  "${GENERATED_INCLUDE_DIR}"
  "${GENERATED_SRC_DIR}"
)

add_custom_command(
  OUTPUT "${GENERATED_XDG_SHELL_HEADER}"
  COMMAND "${WAYLAND_SCANNER}"
  ARGS
    --strict
    client-header
    "${XDG_SHELL_PROTOCOL}"
    "${GENERATED_XDG_SHELL_HEADER}"
)
add_custom_command(
  OUTPUT "${GENERATED_XDG_SHELL_CODE}"
  COMMAND "${WAYLAND_SCANNER}"
  ARGS
  --strict
  private-code
  "${XDG_SHELL_PROTOCOL}"
  "${GENERATED_XDG_SHELL_CODE}"
)
add_custom_target(cbwayland_generatedheaders
  DEPENDS "${GENERATED_XDG_SHELL_HEADER}"
)

add_library(cbwayland SHARED
  src/fd.cpp
  src/wayland.cpp

  src/objects/buffer.cpp
  src/objects/callback.cpp
  src/objects/data_device.cpp
  src/objects/data_offer.cpp
  src/objects/data_source.cpp
  src/objects/display.cpp
  src/objects/keyboard.cpp
  src/objects/registry.cpp
  src/objects/seat.cpp
  src/objects/shm.cpp
  src/objects/shm_pool.cpp
  src/objects/surface.cpp
  src/objects/xdg_surface.cpp
  src/objects/xdg_toplevel.cpp
  src/objects/xdg_wm_base.cpp

  "${GENERATED_XDG_SHELL_CODE}"
)
add_dependencies(cbwayland cbwayland_generatedheaders)

enable_lto(cbwayland)

target_link_libraries(cbwayland
  ${WAYLAND_CLIENT_LIBRARIES}
  gui
)
target_include_directories(cbwayland PRIVATE
  ${WAYLAND_CLIENT_INCLUDE_DIRS}
  ${GENERATED_INCLUDE_DIR}
)

set_target_properties(cbwayland PROPERTIES
  VERSION 1.0
  SOVERSION 1
)

install(TARGETS cbwayland LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
