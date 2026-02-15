cmake_minimum_required(VERSION 3.20)

project(shrooms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SHROOMS_ASAN "Enable AddressSanitizer" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(SHROOMS_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(ENGINE_PLATFORM "web")
  message(STATUS "Configuring for Web (Emscripten)")
else()
  set(ENGINE_PLATFORM "native")
  message(STATUS "Configuring for native platform")
endif()

include(FetchContent)

FetchContent_Declare(
  stb_image
  URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
  DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(stb_image)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})

# Engine headers include both glm/... and glm/glm/... forms.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/glm)

add_subdirectory(3rd-party/engine/libs/core)
add_subdirectory(3rd-party/engine/libs/ui)
add_subdirectory(3rd-party/engine/libs/render)
add_subdirectory(3rd-party/engine/libs/platform)

add_executable(shrooms
  apps/shrooms_demo/main.cpp
  apps/shrooms_demo/shrooms_app.cpp
)

target_link_libraries(shrooms
  PRIVATE
    engine_core
    engine_render
    engine_webgl
    engine_platform
)

target_include_directories(shrooms PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/libs
  ${CMAKE_CURRENT_SOURCE_DIR}/apps/shrooms_demo
)

target_compile_features(shrooms PRIVATE cxx_std_20)

add_custom_command(TARGET shrooms POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/assets
  $<TARGET_FILE_DIR:shrooms>/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/assets
  ${CMAKE_CURRENT_SOURCE_DIR}/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/shaders
  $<TARGET_FILE_DIR:shrooms>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/shaders
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)

if(ENGINE_PLATFORM STREQUAL "web")
  set_target_properties(shrooms PROPERTIES SUFFIX ".html")
  target_link_options(shrooms PRIVATE
    "-sWASM=1"
    "--use-preload-plugins"
    "-sMIN_WEBGL_VERSION=2"
    "-sMAX_WEBGL_VERSION=2"
    "-sUSE_WEBGL2=1"
    "-sFULL_ES3=1"
    "-sUSE_SDL=2"
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/assets@/assets"
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/engine/shaders@/shaders"
  )
else()
  find_package(SDL2 REQUIRED)
  target_link_libraries(shrooms PRIVATE SDL2::SDL2)
  target_include_directories(shrooms PRIVATE ${SDL2_INCLUDE_DIRS})
endif()
