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
add_custom_target(clipboardwayland_generatedheaders
  DEPENDS "${GENERATED_XDG_SHELL_HEADER}"
)

add_library(clipboardwayland MODULE
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
add_dependencies(clipboardwayland clipboardwayland_generatedheaders)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported AND NOT NO_LTO)
  set_property(TARGET clipboardwayland PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
endif()

target_link_libraries(clipboardwayland
  ${WAYLAND_CLIENT_LIBRARIES}
  gui
)
target_include_directories(clipboardwayland PRIVATE
  ${WAYLAND_CLIENT_INCLUDE_DIRS}
  ${GENERATED_INCLUDE_DIR}
)

install(TARGETS clipboardwayland LIBRARY DESTINATION lib)
