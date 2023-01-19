cmake_minimum_required(VERSION 3.16)
project(Clipboard LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (UNIX AND NOT APPLE)
  set(POSIX ON)
endif()

if($ENV{CI}) # enable easy artifact access on CI by shipping everything to a unified folder
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/bin)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/output/bin)
else()
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
endif()

if(APPLE AND NOT INSTALL_PREFIX)
  set(INSTALL_PREFIX "/usr/local")
elseif(POSIX AND NOT INSTALL_PREFIX)
  set(INSTALL_PREFIX "/usr")
endif()

set(CMAKE_INSTALL_PREFIX_BIN "${INSTALL_PREFIX}/bin")
set(CMAKE_INSTALL_PREFIX_LIB "${INSTALL_PREFIX}/lib")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Building Clipboard in Debug mode")
  if(NOT MSVC AND NOT APPLE)
    #add_compile_options(-fsanitize=address)
    #add_link_options(-fsanitize=address)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  endif()
  if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  endif()
else()
  if(NOT WIN32 AND NOT APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -s")
  endif()
endif()

add_subdirectory(src/gui)

find_package(X11)
if(NOT NO_X11 AND X11_FOUND AND NOT APPLE) # X11 is technically available on macOS, but we don't want to use it
  message(STATUS "Building Clipboard with X11 support")
  add_subdirectory(src/clipboardx11)
else()
  message(STATUS "Building Clipboard without X11 support")
endif()

find_package(PkgConfig)
pkg_check_modules(WAYLAND_CLIENT wayland-client)
if(NOT NO_WAYLAND AND WAYLAND_CLIENT_FOUND)
  message(STATUS "Building Clipboard with Wayland support")
  add_subdirectory(src/clipboardwayland)
else()
  message(STATUS "Building Clipboard without Wayland support")
endif()

add_subdirectory(src/clipboard)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADD_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_FLAGS}")
