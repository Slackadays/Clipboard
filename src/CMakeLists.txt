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

if($ENV{CI})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/bin)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/output/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/output/bin)
endif()

if(APPLE)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation Prefix")
elseif(POSIX)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation Prefix")

  if(NOT NO_X11)
    find_package(X11)
  endif()
  if(NOT NO_WAYLAND)
    find_package(PkgConfig)
    pkg_check_modules(WAYLAND_CLIENT wayland-client)
  endif()
endif()

if (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_BIN "${CMAKE_INSTALL_PREFIX}/bin")
  set(CMAKE_INSTALL_PREFIX_LIB "${CMAKE_INSTALL_PREFIX}/lib")
endif()

add_subdirectory(gui)

if(X11_FOUND)
  message(STATUS "Found X11")
  message(STATUS "Building with X11 support")
  add_subdirectory(clipboardx11)
else()
  message(STATUS "X11 not found, building without X11 support")
endif()

if(WAYLAND_CLIENT_FOUND)
  message(STATUS "Found Wayland")
  message(STATUS "Building with Wayland support")
  add_subdirectory(clipboardwayland)
else()
  message(STATUS "Wayland not found, building without Wayland support")
endif()

add_subdirectory(clipboard)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE AND (NOT DEFINED ENV{CI})) #do not use a native uarch if we are in a CI environment
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADD_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_FLAGS}")
