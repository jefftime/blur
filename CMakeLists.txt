cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(blur LANGUAGES C)

set(PLATFORM_DEFINITION "-DPLATFORM_LINUX")

## External libraries
set(EXTLIBS)
find_library(XAU NAMES "Xau" PATHS "./libs" NO_DEFAULT_PATH)
find_library(XDMCP NAMES "Xdmcp" PATHS "./libs" NO_DEFAULT_PATH)
find_library(XCB NAMES "xcb" PATHS "./libs" NO_DEFAULT_PATH)
if (NOT XAU)
  message(FATAL_ERROR "Could not find libXau")
endif()
if (NOT XDMCP)
  message(FATAL_ERROR "Could not find libXdmcp")
endif()
if (NOT XCB)
  message(FATAL_ERROR "Could not find libxcb")
endif()
# order is important here to make sure we don't have missing symbols
list(APPEND EXTLIBS ${XCB} ${XDMCP} ${XAU})

## Sources
set(PLATFORM_SOURCES
  "src/window_linux.c"
  )
set(SOURCES
  "src/blur.c"
  )

add_executable(blur ${SOURCES} ${PLATFORM_SOURCES})

## Configuration
target_compile_options(blur
  PRIVATE "-std=c90"
  PRIVATE "-pedantic-errors"
  PRIVATE "-Wall"
  PRIVATE "-Wconversion"
  PRIVATE ${PLATFORM_DEFINITION}
  )
target_link_libraries(blur ${EXTLIBS})
