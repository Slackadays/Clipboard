add_library(clipboardx11 MODULE
  src/x11.cpp
)

target_link_libraries(clipboardx11
  ${X11_LIBRARIES}
  gui
)

include(../lto.cmake)
enable_lto(clipboardx11)

target_include_directories(clipboardx11 PRIVATE ${X11_INCLUDE_DIR})

install(TARGETS clipboardx11 LIBRARY DESTINATION lib)
