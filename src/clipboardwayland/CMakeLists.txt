add_library(clipboardwayland MODULE
  src/wayland.cpp
)
target_link_libraries(clipboardwayland
  ${WAYLAND_CLIENT_LIBRARIES}
  gui
)

install(TARGETS clipboardwayland LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/")