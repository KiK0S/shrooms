add_executable(shrooms_demo
  main.cpp
  shrooms_app.cpp
)

target_link_libraries(shrooms_demo
  PRIVATE
    engine_core
    engine_render
    engine_webgl
    engine_platform
)

target_include_directories(shrooms_demo PRIVATE ${CMAKE_SOURCE_DIR}/libs ${CMAKE_CURRENT_LIST_DIR})

target_compile_features(shrooms_demo PRIVATE cxx_std_20)

add_custom_command(TARGET shrooms_demo POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/assets
  $<TARGET_FILE_DIR:shrooms_demo>/assets)

if(ENGINE_PLATFORM STREQUAL "web")
  set_target_properties(shrooms_demo PROPERTIES SUFFIX ".html")
  target_link_options(shrooms_demo PRIVATE "-sWASM=1" "--use-preload-plugins"
    "-sMIN_WEBGL_VERSION=2" "-sMAX_WEBGL_VERSION=2" "-sUSE_WEBGL2=1" "-sFULL_ES3=1"
    "-sUSE_SDL=2"
    "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
    "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/shaders@/shaders")
else()
  find_package(SDL2 REQUIRED)
  target_link_libraries(shrooms_demo PRIVATE SDL2::SDL2)
  target_include_directories(shrooms_demo PRIVATE ${SDL2_INCLUDE_DIRS})
endif()
