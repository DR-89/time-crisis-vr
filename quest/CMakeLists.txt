cmake_minimum_required(VERSION 3.22)
project(timecrisisvr C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(UP "${ROOT}/upstream")
set(SDL_ROOT "${ROOT}/.tools/sdl/SDL2-2.30.11")
set(XR "${ROOT}/.tools/openxr/prefab/modules")
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_HIDAPI ON CACHE BOOL "" FORCE)
add_subdirectory("${SDL_ROOT}" "${CMAKE_BINARY_DIR}/sdl")
# Upstream uses <SDL2/SDL.h>; SDL's source build installs headers under include/.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/compat/SDL2")
file(GLOB SDL_HEADERS "${SDL_ROOT}/include/*.h")
foreach(header ${SDL_HEADERS})
  get_filename_component(name "${header}" NAME)
  configure_file("${header}" "${CMAKE_BINARY_DIR}/compat/SDL2/${name}" COPYONLY)
endforeach()
include("${UP}/engine/engine.cmake")
list(REMOVE_ITEM NAMCO22_ENGINE_GL_SRC "${NAMCO22_ENGINE_DIR}/render_target.c" "${NAMCO22_ENGINE_DIR}/post_gl.c")
file(GLOB LIFTED "${UP}/timecris/gen/tc_lifted_*.c")
set_source_files_properties(${LIFTED} "${UP}/timecris/gen/tc_snd_driver.c" PROPERTIES COMPILE_OPTIONS "-O2;-w")
set_source_files_properties("${UP}/timecris/src/tc_game.c" PROPERTIES COMPILE_DEFINITIONS "main=tc_game_main;SDL_MAIN_HANDLED")
add_library(main SHARED
  quest_main.c quest_host.c quest_gl.c quest_scene.c quest_gun.c quest_options.c quest_cover.c quest_ui.c
  "${UP}/timecris/src/tc_game.c" ${LIFTED}
  "${ROOT}/build/tc_crate.c"
  "${UP}/timecris/gen/tc_c25.c" "${UP}/timecris/gen/tc_snd_driver.c"
  ${NAMCO22_ENGINE_LIFT_SRC} ${NAMCO22_ENGINE_C25_SRC} ${NAMCO22_ENGINE_SND_SRC}
  ${NAMCO22_ENGINE_GL_SRC} ${NAMCO22_ENGINE_SS22_SRC} ${NAMCO22_ENGINE_ROMZIP_SRC}
  "${UP}/engine/ss22_board.c" "${UP}/engine/ss22_dsp.c" "${UP}/engine/ss22_snd.c"
  "${UP}/engine/ss22_video.c" "${UP}/engine/ss22_env.c" "${UP}/engine/ss22_run.c")
target_compile_definitions(main PRIVATE TCVR XR_USE_PLATFORM_ANDROID XR_USE_GRAPHICS_API_OPENGL_ES)
target_compile_options(main PRIVATE -O2 -fno-strict-aliasing -fwrapv -Wall -Wno-unused-function -Wno-unused-variable -Wno-misleading-indentation)
target_include_directories(main PRIVATE . include "${CMAKE_BINARY_DIR}/compat" "${UP}/timecris/gen" "${UP}/include" "${XR}/headers/include")
add_library(openxr_loader SHARED IMPORTED)
set_target_properties(openxr_loader PROPERTIES IMPORTED_LOCATION "${XR}/openxr_loader/libs/android.arm64-v8a/libopenxr_loader.so")
target_link_libraries(main SDL2 openxr_loader GLESv3 EGL android log z m)
target_link_options(main PRIVATE "-Wl,-z,max-page-size=16384" "-Wl,--no-undefined")
