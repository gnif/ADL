cmake_minimum_required(VERSION 3.0.0)

set(TARGET_NAME "adl")
project(${TARGET_NAME} LANGUAGES C)
set(CMAKE_C_STANDARD 11)

find_package(PkgConfig)

include_directories(include)
link_libraries(${CMAKE_DL_LIBS})

set(SOURCES
  src/adl.c
  src/window.c
  src/status.c
  src/logging.c
  src/linkedlist.c
)

pkg_check_modules(XCB xcb)
if(XCB_FOUND)	
  list(APPEND SOURCES linux/xcb/xcb.c)
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
#target_compile_options(${TARGET_NAME} PUBLIC -Wl,-r)
target_include_directories(${TARGET_NAME}
	INTERFACE
		include
	PRIVATE
		src
)

