cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)
project(blur LANGUAGES C)

option(PLATFORM "Set the platform build target")
option(RENDER_BACKEND "Set the renderer backend")

# Defaults
if (NOT PLATFORM)
  message(WARNING "PLATFORM not set, defaulting to Linux")
  set(PLATFORM "LINUX")
endif()
if (NOT RENDER_BACKEND)
  message(WARNING "RENDER_BACKEND not set, defaulting to VK")
  set(RENDER_BACKEND "VK")
endif()

string(TOLOWER ${PLATFORM} PLATFORM)
string(TOLOWER ${RENDER_BACKEND} RENDER_BACKEND)

## External libraries
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
list(APPEND EXTLIBS ${XAU} ${XDMCP} ${XCB})

## Platform specific settings
if (PLATFORM MATCHES "linux")
  set(PLATFORM_DEFINITION "-DPLATFORM_LINUX")
  set(PLATFORM_SOURCES
    "src/window_linux.c"
    "src/keypoll_linux.c"
    )
  find_library(DL NAMES "dl")
  if (NOT DL)
    message(FATAL_ERROR "Could not find libdl")
  endif()
  list(APPEND EXTLIBS ${DL})
endif()

## Sources
set(SOURCES
  "src/blur.c"
  "src/xrand.c"
  )

if (RENDER_BACKEND MATCHES "vk")
  list(APPEND SOURCES "src/render_vk.c" "src/render_vk_pipeline.c")
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
target_link_libraries(blur "-Wl,--start-group" ${EXTLIBS} "-Wl,--end-group")
target_compile_definitions(blur
  PRIVATE ${PLATFORM_DEFINITION}
  )
