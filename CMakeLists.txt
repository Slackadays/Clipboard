cmake_minimum_required(VERSION 3.16)

project(Clipboard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(TEST)
  message(STATUS "Building in test mode")
  set(CMAKE_BUILD_TYPE Debug)
  if(NOT MSVC AND NOT APPLE)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  endif()
else()
  if(NOT CMAKE_BUILD_TYPE AND NOT WIN32)
    set(CMAKE_BUILD_TYPE MinSizeRel)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APPLE)
      set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -s") # strip the binary for reduced size
    endif()
  endif()
endif()

set(CMAKE_THREAD_PREFER_PTHREAD True)
set(THREADS_PREFER_PTHREAD_FLAG True)
find_package(Threads REQUIRED)

if(UNIX AND NOT APPLE AND (NOT DEFINED ENV{CI} OR TEST))
  find_package(X11)
  include_directories(${X11_INCLUDE_DIR})
  find_package(PkgConfig)
  pkg_check_modules(WAYLAND_CLIENT wayland-client)
endif()

if(X11_FOUND)
  message(STATUS "Found X11")
  if(NOT WAYLAND_CLIENT_FOUND)
    message(STATUS "Building with X11 support")
    add_compile_definitions(X11_AVAILABLE)
  endif()
else()
  message(STATUS "X11 not found, building without X11 support")
endif()

if(WAYLAND_CLIENT_FOUND)
  message(STATUS "Found Wayland")
  message(STATUS "Building with Wayland support")
  add_compile_definitions(WAYLAND_AVAILABLE)
else()
  message(STATUS "Wayland not found, building without Wayland support")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE AND (NOT DEFINED ENV{CI})) #do not use a native uarch if we are in a CI environment
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADD_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_FLAGS}")

add_executable(clipboard
  src/clipboard.cpp
  src/messages.cpp
  src/logging.cpp
)

if(WIN32)
  target_sources(clipboard PRIVATE src/windows.cpp)
endif()

if(X11_FOUND AND NOT WAYLAND_CLIENT_FOUND)
  target_sources(clipboard PRIVATE src/x11.cpp)
endif()

if(WAYLAND_CLIENT_FOUND)
  target_sources(clipboard PRIVATE src/wayland.cpp)
endif()

if(APPLE)
  enable_language(OBJC)
  target_sources(clipboard PRIVATE src/macos.m)
  target_sources(clipboard PRIVATE src/macos.cpp)
endif()

if(lto_supported)
  set_property(TARGET clipboard PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE) 
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
else()
    message(STATUS "IPO/LTO not supported: ${lto_error}")
endif()

target_link_libraries(clipboard Threads::Threads)

if(X11_FOUND AND NOT WAYLAND_CLIENT_FOUND)
  target_link_libraries(clipboard ${X11_LIBRARIES})
endif()

if(WAYLAND_CLIENT_FOUND)
  target_link_libraries(clipboard ${WAYLAND_CLIENT_LIBRARIES})
endif()

if(APPLE)
  target_link_libraries(clipboard "-framework AppKit")
endif()

if(WIN32)
  install(TARGETS clipboard DESTINATION bin)
elseif(APPLE)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation Prefix")
  install(TARGETS clipboard
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/")
  install(CODE "execute_process(COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bin/clipboard ${CMAKE_INSTALL_PREFIX}/bin/cb)")
else()
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation Prefix")
  install(TARGETS clipboard
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/")
  install(CODE "execute_process(COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bin/clipboard ${CMAKE_INSTALL_PREFIX}/bin/cb)")
endif()
