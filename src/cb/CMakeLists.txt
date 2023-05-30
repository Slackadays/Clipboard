add_executable(cb
  src/clipboard.cpp
  src/main.cpp
  src/messages.cpp
  src/themes.cpp
  src/indicator.cpp
  src/externalclipboards.cpp
  src/actions/add.cpp
  src/actions/clear.cpp
  src/actions/copy.cpp
  src/actions/edit.cpp
  src/actions/export.cpp
  src/actions/history.cpp
  src/actions/ignore.cpp
  src/actions/import.cpp
  src/actions/info.cpp
  src/actions/load.cpp
  src/actions/note.cpp
  src/actions/paste.cpp
  src/actions/pipeinout.cpp
  src/actions/remove.cpp
  src/actions/search.cpp
  src/actions/show.cpp
  src/actions/status.cpp
  src/actions/swap.cpp
)

enable_lto(cb)

target_link_libraries(cb gui)

if(WIN32)
  target_sources(cb PRIVATE
    src/windows.cpp
    src/windows.manifest
  )
  target_compile_options(cb PRIVATE /utf-8)
elseif(APPLE)
  enable_language(OBJC)
  target_sources(cb PRIVATE
    src/macos.m
    src/macos.cpp
  )
  target_link_libraries(cb "-framework AppKit")
  target_compile_options(cb PRIVATE -fobjc-arc)
elseif(HAIKU)
  target_sources(cb PRIVATE
    src/haiku.cpp
  )
  target_link_libraries(cb be)
elseif(ANDROID)
  target_sources(cb PRIVATE
    src/android.cpp
  )
elseif(X11WL)
  target_sources(cb PRIVATE src/x11wl.cpp)

  set_property(
    TARGET cb
    APPEND
    PROPERTY BUILD_RPATH
    "$ORIGIN"
  )
  set_property(
    TARGET cb
    APPEND
    PROPERTY BUILD_RPATH
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
  )
  set_property(
    TARGET cb
    APPEND
    PROPERTY INSTALL_RPATH
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
  )
  target_link_options(cb PRIVATE -z origin) # set the rpath to $ORIGIN
  target_link_libraries(cb ${CMAKE_DL_LIBS})
endif()

set(CMAKE_THREAD_PREFER_PTHREAD True)
set(THREADS_PREFER_PTHREAD_FLAG True)
find_package(Threads REQUIRED)
target_link_libraries(cb Threads::Threads)

install(TARGETS cb DESTINATION bin)

if(X11WL)
  install(FILES ${CMAKE_SOURCE_DIR}/documentation/manpages/man.1 DESTINATION share/man/man1 RENAME cb.1)
endif()
