cmake_minimum_required(VERSION 3.14)

set (CMAKE_CXX_STANDARD 17)

project(LostNFound VERSION 0.0.0)

if (EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS  "-s USE_SDL=2 -s USE_ZLIB=1 -s USE_FREETYPE=1")	
	add_compile_options("-Wno-implicit-function-declaration")	
endif()

include(FetchContent)

FetchContent_Declare(
    stb_image
    URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
    DOWNLOAD_NO_EXTRACT True
)

FetchContent_MakeAvailable(stb_image)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})

set(GLM_DIR "${CMAKE_SOURCE_DIR}/3rd-party/glm")
add_subdirectory(${GLM_DIR})
include_directories(${GLM_DIR})

set(DELAUNAY_DIR "${CMAKE_SOURCE_DIR}/3rd-party/delaunay-cpp")
include_directories(${DELAUNAY_DIR})

set(FREETYPE_INCLUDE_DIRS /usr/include/freetype2)
set(FREETYPE_LIBRARY /usr/lib/x86_64-linux-gnu/libfreetype.so)
set(freetype-gl_BUILD_APIDOC OFF CACHE BOOL "" FORCE)
set(freetype-gl_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(freetype-gl_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(freetype-gl_OFF_SCREEN ON CACHE BOOL "" FORCE)
set(freetype-gl_WITH_GLEW OFF CACHE BOOL "" FORCE)
set(freetype-gl_WITH_GLEW_DEFAULT OFF CACHE BOOL "" FORCE)
set(freetype-gl_BUILD_MAKEFONT OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rd-party/freetype-gl)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/3rd-party/imgui")
include_directories(${IMGUI_DIR})

# Add ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
)
add_compile_options("-DIMGUI_IMPL_OPENGL_ES3")
add_library(imgui STATIC ${IMGUI_SOURCES})

include_directories(${CMAKE_SOURCE_DIR}/3rd-party)
include_directories(${PROJECT_BINARY_DIR}/3rd-party/freetype-gl)
include_directories(/usr/include/freetype2  -I/usr/local/include/freetype2  -I/usr/include/libpng16)

set(EXE_SOURCES
	src/main.cpp
	3rd-party/freetype-gl/texture-atlas.c
	3rd-party/freetype-gl/texture-font.c
	3rd-party/freetype-gl/ftgl-utils.c
	3rd-party/freetype-gl/utf8-utils.c
	3rd-party/freetype-gl/distance-field.c
	3rd-party/freetype-gl/edtaa3func.c
	3rd-party/freetype-gl/vector.c
)

add_executable(LostNFound ${EXE_SOURCES})

include_directories(${CMAKE_SOURCE_DIR}/src)

if (EMSCRIPTEN)
    set(SDL2_LIBRARIES "")
else()
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

if (EMSCRIPTEN)
		SET(CMAKE_EXE_LINKER_FLAGS "-s LLD_REPORT_UNDEFINED -sNO_DISABLE_EXCEPTION_CATCHING -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2 --preload-file ${PROJECT_SOURCE_DIR}/assets@/assets --preload-file ${PROJECT_SOURCE_DIR}/shaders@/shaders -sUSE_SDL=2 -sUSE_WEBGL2=1 -sWASM=1")
		set_target_properties(LostNFound PROPERTIES RUNTIME_OUTPUT_DIRECTORY build)
		configure_file(${PROJECT_SOURCE_DIR}/index.html ${PROJECT_BINARY_DIR}/build/index.html COPYONLY)
else()
		set(CMAKE_BUILD_TYPE Debug)
		set(CMAKE_CXX_FLAGS  "-fsanitize=undefined -fsanitize=bounds -fsanitize=address")
		find_package(GLEW REQUIRED)
		include_directories(${GLEW_INCLUDE_DIRS})
                target_link_libraries(LostNFound PRIVATE ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} GL)
endif()

if (NOT EMSCRIPTEN)
    target_link_libraries(LostNFound PRIVATE ${SDL2_LIBRARIES})
endif()
target_link_libraries(LostNFound PRIVATE stb_image)
target_link_libraries(LostNFound PRIVATE freetype)
target_link_libraries(LostNFound PRIVATE imgui)

target_compile_options(LostNFound PRIVATE)