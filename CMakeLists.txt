cmake_minimum_required(VERSION 3.16)
project(Clipboard LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (UNIX AND NOT APPLE)
  set(POSIX ON)
endif()

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

if(NOT CMAKE_INSTALL_PREFIX AND APPLE)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation Prefix")
elseif(NOT CMAKE_INSTALL_PREFIX AND POSIX)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation Prefix")
endif()

if (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_BIN "${CMAKE_INSTALL_PREFIX}/bin")
  set(CMAKE_INSTALL_PREFIX_LIB "${CMAKE_INSTALL_PREFIX}/lib")
endif()

add_subdirectory(src/gui)

find_package(X11)
if(NOT NO_X11 AND X11_FOUND AND NOT APPLE) # X11 is technically available on macOS, but we don't want to use it
  message(STATUS "Building with X11 support")
  add_subdirectory(src/clipboardx11)
else()
  message(STATUS "Building without X11 support")
endif()

find_package(PkgConfig)
pkg_check_modules(WAYLAND_CLIENT wayland-client)
if(NOT NO_WAYLAND AND WAYLAND_CLIENT_FOUND)
  message(STATUS "Building with Wayland support")
  add_subdirectory(src/clipboardwayland)
else()
  message(STATUS "Building without Wayland support")
endif()

add_subdirectory(src/clipboard)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE AND (NOT DEFINED ENV{CI})) #do not use a native uarch if we are in a CI environment
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)
if(lto_supported)
  set_property(TARGET clipboard PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADD_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_FLAGS}")
