cmake_minimum_required(VERSION 3.10)

project(wasm-3d)

# If the EMSDK environment variable is set, configure the toolchain file. this has to be done before the project() line below
if(DEFINED ENV{EMSDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE STRING "Emscripten toolchain file")
    message(STATUS "WebAssembly build enabled")
    message(STATUS "CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}'")
    #include_directories($ENV{EMSDK}/upstream/emscripten/system/include)
else()
    message(STATUS "WebAssembly build NOT enabled. EMSDK environment variable is not set. Falling back to native build. If you want to compile for WebAssembly, first `source emsdk_env.sh` in the emsdk directory and then do `mkdir buildwasm && cd buildwasm && emcmake cmake .. && emmake make`")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(BGFX_DIR ${CMAKE_SOURCE_DIR}/bgfx)

if(NOT DEFINED ENV{EMSDK})
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
endif()

include_directories(${BGFX_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/bx/include)
include_directories(${CMAKE_SOURCE_DIR}/bimg/include)
include_directories(${CMAKE_SOURCE_DIR}/thirdparty)

add_definitions(-DBX_CONFIG_DEBUG)

add_executable(wasm-3d main.cpp     "common/common.h" "common/cube_atlas.cpp" "common/cube_atlas.h" "common/font/font_manager.cpp" "common/font/font_manager.h" "common/font/fs_font_basic.bin.h" "common/font/fs_font_distance_field.bin.h" "common/font/fs_font_distance_field_drop_shadow.bin.h" "common/font/fs_font_distance_field_drop_shadow_image.bin.h" "common/font/fs_font_distance_field_outline.bin.h" "common/font/fs_font_distance_field_outline_drop_shadow_image.bin.h" "common/font/fs_font_distance_field_outline_image.bin.h" "common/font/fs_font_distance_field_subpixel.bin.h" "common/font/text_buffer_manager.cpp" "common/font/text_buffer_manager.h" "common/font/text_metrics.cpp" "common/font/text_metrics.h" "common/font/utf8.cpp" "common/font/utf8.h" "common/font/vs_font_basic.bin.h" "common/font/vs_font_distance_field.bin.h" "common/font/vs_font_distance_field_drop_shadow.bin.h" "common/font/vs_font_distance_field_drop_shadow_image.bin.h" "common/font/vs_font_distance_field_outline.bin.h" "common/font/vs_font_distance_field_outline_drop_shadow_image.bin.h" "common/font/vs_font_distance_field_outline_image.bin.h" "common/font/vs_font_distance_field_subpixel.bin.h" "common/packrect.h" "common/entry/cmd.cpp" "common/entry/cmd.h" "common/entry/dbg.h" "common/entry/entry.cpp" "common/entry/entry.h" "common/entry/entry_p.h" "common/entry/input.cpp" "common/entry/input.h"   "common/font/fs_mesh_basic.bin.h" "common/font/vs_mesh_basic.bin.h")

file(INSTALL DESTINATION ${CMAKE_BINARY_DIR}
    TYPE FILE
    FILES
    ${BGFX_DIR}/examples/runtime/font/special_elite.ttf)

if(DEFINED ENV{EMSDK})
    set(CMAKE_CXX_FLAGS "
        -s USE_WEBGL2=1 \
        -s FULL_ES3=1 \
        --preload-file special_elite.ttf \
        --bind \
        --use-preload-plugins \
        -Wall \
        -Wextra \
        -Werror=return-type \
        -w \
        -g4 \
        -Os \
        --no-heap-copy \
        -s GL_ENABLE_GET_PROC_ADDRESS \
        -s FETCH=1 \
        -static \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s NO_EXIT_RUNTIME=1 \
        -sASSERTIONS")

    target_link_libraries(wasm-3d
        ${BGFX_DIR}/.build/wasm/bin/bgfxRelease.bc
        ${BGFX_DIR}/.build/wasm/bin/bxRelease.bc
        ${BGFX_DIR}/.build/wasm/bin/bimgRelease.bc)

    file(INSTALL DESTINATION ${CMAKE_BINARY_DIR}
        TYPE FILE
        FILES ${CMAKE_SOURCE_DIR}/index.html)
else() # Native Linux/X11
    target_link_libraries(wasm-3d
        glfw
        OpenGL::GL
        ${BGFX_DIR}/.build/linux64_gcc/bin/libbgfx-shared-libDebug.so)
endif()
