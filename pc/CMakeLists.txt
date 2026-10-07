cmake_minimum_required(VERSION 3.22)
project(timecrisisvr_pc C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(UP "${ROOT}/upstream")
set(SDL_ROOT "${ROOT}/.tools/sdl/SDL2-2.30.11")
set(XR "${ROOT}/.tools/openxr-pc")
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
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
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
add_subdirectory("${ROOT}/.tools/zlib/zlib-1.3.1" "${CMAKE_BINARY_DIR}/zlib")
add_executable(main
  pc_main.c pc_gl_context.c vendor/glad/src/gl.c
  "${ROOT}/quest/quest_host.c" "${ROOT}/quest/quest_gl.c" "${ROOT}/quest/quest_scene.c"
  "${ROOT}/quest/quest_gun.c" "${ROOT}/quest/quest_options.c" "${ROOT}/quest/quest_cover.c" "${ROOT}/quest/quest_ui.c"
  "${UP}/timecris/src/tc_game.c" ${LIFTED}
  "${ROOT}/build/tc_crate.c"
  "${UP}/timecris/gen/tc_c25.c" "${UP}/timecris/gen/tc_snd_driver.c"
  ${NAMCO22_ENGINE_LIFT_SRC} ${NAMCO22_ENGINE_C25_SRC} ${NAMCO22_ENGINE_SND_SRC}
  ${NAMCO22_ENGINE_GL_SRC} ${NAMCO22_ENGINE_SS22_SRC} ${NAMCO22_ENGINE_ROMZIP_SRC}
  "${UP}/engine/ss22_board.c" "${UP}/engine/ss22_dsp.c" "${UP}/engine/ss22_snd.c"
  "${UP}/engine/ss22_video.c" "${UP}/engine/ss22_env.c" "${UP}/engine/ss22_run.c")
target_compile_definitions(main PRIVATE TCVR TCVR_PC SDL_MAIN_HANDLED XR_USE_PLATFORM_WIN32 XR_USE_GRAPHICS_API_OPENGL)
target_compile_options(main PRIVATE -O2 -fno-strict-aliasing -fwrapv -Wall -Wno-unused-function -Wno-unused-variable -Wno-misleading-indentation -include "${UP}/include/win_compat.h")
target_include_directories(main PRIVATE "${ROOT}/quest" "${ROOT}/quest/include" vendor/glad/include "${ROOT}/.tools/zlib/zlib-1.3.1" "${CMAKE_BINARY_DIR}/zlib" "${CMAKE_BINARY_DIR}/compat" "${UP}/timecris/gen" "${UP}/include" "${XR}/include")
add_library(openxr_loader UNKNOWN IMPORTED)
set_target_properties(openxr_loader PROPERTIES IMPORTED_LOCATION "${XR}/native/x64/release/lib/openxr_loader.lib")
target_link_libraries(main SDL2-static openxr_loader zlibstatic opengl32 winpthread)
set_target_properties(main PROPERTIES OUTPUT_NAME TimeCrisisVR)
target_link_options(main PRIVATE -static "-Wl,--stack,67108864")
