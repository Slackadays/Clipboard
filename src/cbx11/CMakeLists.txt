add_library(cbx11 MODULE
  src/x11.cpp
)

target_link_libraries(cbx11
  ${X11_LIBRARIES}
  gui
)

enable_lto(cbx11)

target_include_directories(cbx11 PRIVATE ${X11_INCLUDE_DIR})

install(TARGETS cbx11 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
