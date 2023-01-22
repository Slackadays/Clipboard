add_library(gui STATIC
  src/fork.cpp
  src/gui.cpp
  src/utils.cpp
)
target_include_directories(gui PUBLIC include/all)
set_property(TARGET gui PROPERTY POSITION_INDEPENDENT_CODE ON)

include(CheckSymbolExists)
check_symbol_exists(fork "unistd.h" HAVE_FORK)
if(HAVE_FORK)
  target_compile_definitions(gui PUBLIC HAVE_FORK)
endif()

if(POSIX)
  target_sources(gui PRIVATE
    src/posix/mime.cpp
  )

  target_include_directories(gui PUBLIC include/posix)
endif()

