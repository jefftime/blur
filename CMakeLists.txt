cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)
project(blur LANGUAGES C)

set(PLATFORM_DEFINITION "-DPLATFORM_LINUX")
set(RENDER_BACKEND "SW")


## External libraries
set(EXTLIBS)
find_library(DL NAMES "dl")
find_library(XAU NAMES "Xau" PATHS "./libs" NO_DEFAULT_PATH)
find_library(XDMCP NAMES "Xdmcp" PATHS "./libs" NO_DEFAULT_PATH)
find_library(XCB NAMES "xcb" PATHS "./libs" NO_DEFAULT_PATH)
if (NOT DL)
  message(FATAL_ERROR "Could not find libdl")
endif()
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
list(APPEND EXTLIBS
  "-Wl,--start-group"
  ${DL} ${XAU} ${XDMCP} ${XCB}
  "-Wl,--end-group"
  )

## Sources
set(PLATFORM_SOURCES
  "src/window_linux.c"
  "src/keypoll_linux.c"
  )
set(SOURCES
  "src/blur.c"
  "src/xrand.c"
  )

if (RENDER_BACKEND MATCHES "SW")
  list(APPEND SOURCES "src/render_sw.c")
elseif(RENDER_BACKEND MATCHES "VK")
  list(APPEND SOURCES "src/render_vk.c")
endif()

add_executable(blur ${SOURCES} ${PLATFORM_SOURCES})

## Configuration
target_compile_options(blur
  PRIVATE "-fcolor-diagnostics"
  PRIVATE "-std=c90"
  PRIVATE "-pedantic-errors"
  PRIVATE "-Wall"
  PRIVATE "-Wconversion"
  )
target_link_libraries(blur ${EXTLIBS})
target_compile_definitions(blur
  PRIVATE ${PLATFORM_DEFINITION}
  )
if (RENDER_BACKEND MATCHES "SW")
  target_compile_definitions(blur
    PRIVATE "-DRENDER_BACKEND_SW"
    )
endif()
