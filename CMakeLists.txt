cmake_minimum_required(VERSION 3.16)

#make a universal binary on macOS
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)

project(Clipboard LANGUAGES CXX C VERSION 0.8.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (UNIX AND NOT APPLE AND NOT HAIKU AND NOT ANDROID)
  set(X11WL ON)
endif()

execute_process( # save the current branch to GIT_BRANCH
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process( # save the current commit hash to GIT_COMMIT_HASH
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions(-DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")
add_definitions(-DGIT_BRANCH="${GIT_BRANCH}")
add_definitions(-DCLIPBOARD_VERSION="${PROJECT_VERSION}")

if (MSVC)
  add_compile_definitions(NOMINMAX) # Prevent min() or max() from being made into macros by windows.h
endif()

if (NOT CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(ALSA REQUIRED)
  #find_path(LIBURING_INCLUDE_DIR liburing.h)
  #if (LIBURING_INCLUDE_DIR)
  #  message(STATUS "Building the Clipboard Project with io_uring support")
  #  set(HAVE_LIBURING TRUE)
  #else()
  #  message(FATAL_ERROR "Could not find liburing")
  #endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Building the Clipboard Project in Debug mode")
  if(NOT MSVC AND NOT APPLE)
    #add_compile_options(-fsanitize=address)
    #add_link_options(-fsanitize=address)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  endif()
  if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    if (NOT NO_COVERAGE)
      if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
      endif()
    endif()
  endif()
else()
  if(NOT WIN32 AND NOT APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -s")
  endif()
endif()

set(LIB_LOCATION ${CMAKE_BINARY_DIR})
set(BIN_LOCATION ${CMAKE_BINARY_DIR})
if ($ENV{CI}) # enable easy artifact access on CI by shipping everything to a unified folder
  set(LIB_LOCATION ${CMAKE_BINARY_DIR}/output/lib)
  set(BIN_LOCATION ${CMAKE_BINARY_DIR}/output/bin)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_LOCATION})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_LOCATION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_LOCATION})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_LOCATION})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIB_LOCATION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_LOCATION})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_LOCATION})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIB_LOCATION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_LOCATION})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${LIB_LOCATION})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${LIB_LOCATION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BIN_LOCATION})

function(enable_lto this_target)
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported AND NOT NO_LTO)
  set_property(TARGET ${this_target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
endif()
endfunction()

add_subdirectory(src/gui)

find_package(X11)
if(NOT NO_X11 AND X11_FOUND AND NOT APPLE) # X11 is technically available on macOS, but we don't want to use it
  message(STATUS "Building the Clipboard Project with X11 support")
  add_subdirectory(src/cbx11)
else()
  message(STATUS "Building the Clipboard Project without X11 support")
endif()

find_package(PkgConfig)
pkg_check_modules(WAYLAND_CLIENT wayland-client wayland-protocols)
if(NOT NO_WAYLAND AND WAYLAND_CLIENT_FOUND)
  message(STATUS "Building the Clipboard Project with Wayland support")
  add_subdirectory(src/cbwayland)
else()
  message(STATUS "Building the Clipboard Project without Wayland support")
endif()

add_subdirectory(src/cb)
