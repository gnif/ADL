cmake_minimum_required(VERSION 3.0.0)

set(TARGET_NAME "adl")
project(${TARGET_NAME} LANGUAGES C)
set(CMAKE_C_STANDARD 11)

find_package(PkgConfig)

include_directories(include)
link_libraries(${CMAKE_DL_LIBS})

if(UNIX)
  set(PLATFORM "linux")
  set(PUBLIC_DEFS -D ADL_PLATFORM_LINUX)
  set(PRIVATE_DEFS -D_GNU_SOURCE)
  link_libraries(pthread)
  link_libraries(rt)
elseif(WIN32)
  set(PLATFORM "win32")
  set(PUBLIC_DEFS -D ADL_PLATFORM_WIN32)
  set(PRIVATE_DEFS)
endif(UNIX)

set(SOURCES
  src/adl.c
  src/window.c
  src/image.c
  src/status.c  
  src/logging.c
  src/linkedlist.c
  ${PLATFORM}/util.c
  ${PLATFORM}/thread.c
  ${PLATFORM}/timer.c
)

find_package(OpenGL COMPONENTS EGL)
if (OpenGL_EGL_FOUND)
  list(APPEND PUBLIC_DEFS -DADL_HAS_EGL)
  include_directories(${OPENGL_EGL_INCLUDE_DIRS})
  link_libraries(${OPENGL_egl_LIBRARY})
endif(OpenGL_EGL_FOUND)

pkg_check_modules(XCB
  x11-xcb
  xcb
  xcb-dri3
  xcb-present
  xcb-xkb
  xcb-cursor
  xcb-render
)

if(XCB_FOUND)  
  list(APPEND SOURCES
    linux/xcb/xcb.c
    linux/xcb/image.c)
  include_directories(${XCB_INCLUDE_DIRS})
  link_libraries(${XCB_LIBRARIES})
endif(XCB_FOUND)

add_compile_options(
  "-Wall"
  "-Werror"
  "-Wfatal-errors"
  "-ffast-math"
  "-fdata-sections"
  "-ffunction-sections"
  "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)

get_filename_component(PROJECT_TOP "${PROJECT_SOURCE_DIR}" ABSOLUTE)
include_directories(${PROJECT_TOP})

add_library(${TARGET_NAME} OBJECT ${SOURCES})
target_compile_definitions(${TARGET_NAME} PRIVATE ${PRIVATE_DEFS} PUBLIC ${PUBLIC_DEFS})
target_include_directories(${TARGET_NAME}
  INTERFACE
    include
  PRIVATE
    src
)
