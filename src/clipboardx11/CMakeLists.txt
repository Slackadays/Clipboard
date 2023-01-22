add_library(clipboardx11 MODULE
  src/x11.cpp
)

target_link_libraries(clipboardx11
  ${X11_LIBRARIES}
  gui
)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported AND NOT NO_LTO)
  set_property(TARGET clipboardx11 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
endif()

target_include_directories(clipboardx11 PRIVATE ${X11_INCLUDE_DIR})

install(TARGETS clipboardx11 LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX_LIB}")
