add_executable(clipboard
  src/clipboard.cpp
  src/messages.cpp
)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported AND NOT NO_LTO)
  set_property(TARGET clipboard PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
endif()

include(CheckSymbolExists)
check_symbol_exists(pthread_cancel "pthread.h" HAVE_PTHREAD_CANCEL)
if(HAVE_PTHREAD_CANCEL)
  target_compile_definitions(gui PUBLIC HAVE_PTHREAD_CANCEL)
endif()

target_link_libraries(clipboard gui)

if(WIN32)
  target_sources(clipboard PRIVATE src/windows.cpp)
elseif(APPLE)
  enable_language(OBJC)
  target_sources(clipboard PRIVATE
    src/macos.m
    src/macos.cpp
  )
  target_link_libraries(clipboard "-framework AppKit")
elseif(POSIX)
  target_sources(clipboard PRIVATE src/posix.cpp)

  set_property(
    TARGET clipboard
    APPEND
    PROPERTY BUILD_RPATH
    "$ORIGIN"
  )
  set_property(
    TARGET clipboard
    APPEND
    PROPERTY BUILD_RPATH
    "${CMAKE_INSTALL_PREFIX_LIB}"
  )
  set_property(
    TARGET clipboard
    APPEND
    PROPERTY INSTALL_RPATH
    "${CMAKE_INSTALL_PREFIX_LIB}"
  )
  target_link_options(clipboard PRIVATE -z origin) # set the rpath to $ORIGIN
endif()

set(CMAKE_THREAD_PREFER_PTHREAD True)
set(THREADS_PREFER_PTHREAD_FLAG True)
find_package(Threads REQUIRED)
target_link_libraries(clipboard Threads::Threads)

if(CMAKE_DL_LIBS)
  target_link_libraries(clipboard ${CMAKE_DL_LIBS})
endif()

if(WIN32)
  install(TARGETS clipboard DESTINATION bin)
else()
  install(TARGETS clipboard
    DESTINATION "${CMAKE_INSTALL_PREFIX_BIN}")
  install(CODE "execute_process(COMMAND ln -sf ${CMAKE_INSTALL_PREFIX_BIN}/clipboard ${CMAKE_INSTALL_PREFIX_BIN}/cb)")
endif()