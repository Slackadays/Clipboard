add_library(clipboardwayland MODULE
  src/wayland.cpp
)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported)
  set_property(TARGET clipboardwayland PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
endif()

target_link_libraries(clipboardwayland
  ${WAYLAND_CLIENT_LIBRARIES}
  gui
)

install(TARGETS clipboardwayland LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/")