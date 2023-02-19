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

target_link_libraries(clipboard gui)

if(WIN32)
  target_sources(clipboard PRIVATE
    src/windows.cpp
    src/windows.manifest
  )
  target_compile_options(clipboard PRIVATE /utf-8)
elseif(APPLE)
  enable_language(OBJC)
  target_sources(clipboard PRIVATE
    src/macos.m
    src/macos.cpp
  )
  target_link_libraries(clipboard "-framework AppKit")
  target_compile_options(clipboard PRIVATE -fobjc-arc)
elseif(HAIKU)
  target_sources(clipboard PRIVATE
    src/haiku.cpp
  )
  target_link_libraries(clipboard be)
elseif(ANDROID)
  target_sources(clipboard PRIVATE
    src/android.cpp
  )
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
    "${CMAKE_INSTALL_PREFIX}/lib"
  )
  set_property(
    TARGET clipboard
    APPEND
    PROPERTY INSTALL_RPATH
    "${CMAKE_INSTALL_PREFIX}/lib"
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

install(TARGETS clipboard DESTINATION bin)

if(NOT WIN32)
  install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
      clipboard \
      \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/cb\" \
    )"
  )
else()
  install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
      clipboard.exe \
      \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/cb.exe\" \
    )"
  )
endif()

if(POSIX)
  install(FILES ${CMAKE_SOURCE_DIR}/documentation/manpages/man.1 DESTINATION share/man/man1 RENAME clipboard.1)
endif()
