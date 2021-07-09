cmake_minimum_required (VERSION 3.8)
include(ExternalProject)

project(M5Pi-Launcher)

# SET(CROSS_COMPILE 1)
# set(CMAKE_SYSTEM_NAME Linux)
# set(CMAKE_C_COMPILER "/path/to/your/arm-linux-gcc")
# set(CMAKE_CXX_COMPILER "/path/to/your/arm-linux-g++")
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_C_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -w -fsigned-char")
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -w -fsigned-char")

set(lvgl_dir "${CMAKE_SOURCE_DIR}/lvgl")
ExternalProject_Add(
    lvgl
    URL   https://github.com/lvgl/lvgl/archive/refs/tags/v8.0.1.zip
    SOURCE_DIR ${lvgl_dir}
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND
    make
    C_INCLUDE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    )
ExternalProject_Get_property(lvgl INSTALL_DIR)
set(lvgl_lib_dir ${INSTALL_DIR}/src/lvgl-build)


set(lv_drivers_dir "${CMAKE_SOURCE_DIR}/lv_drivers")
ExternalProject_Add(
    lv_drivers
    GIT_REPOSITORY https://github.com/lvgl/lv_drivers.git
    GIT_SHALLOW 1
    GIT_TAG 8510fcb4d4834fbca8ddcb259ec459d25f3ab270
    SOURCE_DIR ${lv_drivers_dir}
    PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/0001-fix-drv-disp-driver-pointer-data.patch
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND
    make
    C_INCLUDE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    )
add_dependencies(lv_drivers lvgl)
ExternalProject_Get_property(lv_drivers INSTALL_DIR)
set(lv_drivers_lib_dir ${INSTALL_DIR}/src/lv_drivers-build)

set(lv_demos_dir "${CMAKE_SOURCE_DIR}/lv_demos")
ExternalProject_Add(
    lv_demos
    GIT_REPOSITORY https://github.com/lvgl/lv_demos.git
    GIT_SHALLOW 1
    GIT_TAG 137ee357c25111f8fbcdd0ad6125df026ea2099e
    SOURCE_DIR ${lv_demos_dir}
    PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/0001-Modify-for-M5Pi-Launcher.patch
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND
    make
    C_INCLUDE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    )
add_dependencies(lv_demos lvgl)
ExternalProject_Get_property(lv_demos INSTALL_DIR)
set(lv_demos_lib_dir ${INSTALL_DIR}/src/lv_demos-build)

set(lv_lib_qrcode_dir "${CMAKE_SOURCE_DIR}/lv_lib_qrcode")
ExternalProject_Add(
    lib_qrcode
    GIT_REPOSITORY https://github.com/lvgl/lv_lib_qrcode.git
    GIT_SHALLOW 1
    GIT_TAG e194f27d975d199cea9eed18a46921e3631eae03
    SOURCE_DIR ${lv_lib_qrcode_dir}
    PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/0001-QRCode-add-CMakeLists.txt-file.patch
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND
    make
    C_INCLUDE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    )
add_dependencies(lib_qrcode lvgl)
ExternalProject_Get_property(lib_qrcode INSTALL_DIR)
set(lib_qrcode_lib_dir ${INSTALL_DIR}/src/lib_qrcode-build)

set(lv_lib_png_dir "${CMAKE_SOURCE_DIR}/lv_lib_png")
ExternalProject_Add(
    lib_png
    GIT_REPOSITORY https://github.com/lvgl/lv_lib_png.git
    GIT_SHALLOW 1
    GIT_TAG c2e971193c4ea97e2b63a6f16fc6a324673d1565
    SOURCE_DIR ${lv_lib_png_dir}
    PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/0001-PNG-add-CMakeLists.txt-file.patch
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND
    make
    C_INCLUDE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    )
add_dependencies(lib_png lvgl)
ExternalProject_Get_property(lib_png INSTALL_DIR)
set(lib_png_lib_dir ${INSTALL_DIR}/src/lib_png-build)

set(lv_fs_dir "${CMAKE_SOURCE_DIR}/lv_fs_if")
ExternalProject_Add(
    lib_fs
    GIT_REPOSITORY https://github.com/lvgl/lv_fs_if.git
    GIT_SHALLOW 1
    GIT_TAG 2cad463d99244822730171243f2646686b4cd14a
    SOURCE_DIR ${lv_fs_dir}
    PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/0001-fs_if-add-CMakeLists.txt-file.patch
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND
    make
    C_INCLUDE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    )
add_dependencies(lib_fs lvgl)
ExternalProject_Get_property(lib_fs INSTALL_DIR)
set(lib_fs_lib_dir ${INSTALL_DIR}/src/lib_fs-build)

set(nes_dir "${CMAKE_SOURCE_DIR}/nes")
ExternalProject_Add(
    nes
    GIT_REPOSITORY https://github.com/nejidev/arm-NES-linux.git
    GIT_SHALLOW 1
    GIT_TAG e14ec579b3fb58f4c061ca38c103cc11d58d1673
    SOURCE_DIR ${nes_dir}
    PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/patch/0001-NES-Add-CMakeLists.txt-file.patch ${CMAKE_SOURCE_DIR}/patch/0001-Add-pthread_testcancel-function-for-back-to-menu.patch
    CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_COMMAND
    make
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    )
ExternalProject_Get_property(nes INSTALL_DIR)
set(nes_lib_dir ${INSTALL_DIR}/src/nes-build)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lv_demos/src/)
include_directories(${CMAKE_SOURCE_DIR}/nes/)

add_subdirectory(launcher)
add_dependencies(launcher lvgl lv_demos lib_qrcode nes)

find_package(Threads)

if (UNIX)
    list(APPEND libs
        ${lv_drivers_lib_dir}/liblv_drivers.a
        ${lv_demos_lib_dir}/liblv_examples.a
        ${lvgl_lib_dir}/liblvgl.a
        ${lib_qrcode_lib_dir}/liblv_lib_qrcode.a
        ${lib_png_lib_dir}/liblv_lib_png.a
        ${lib_fs_lib_dir}/liblv_fs_if.a
        ${nes_lib_dir}/libnes.a
        -lz
        -lm
        -lpthread
        -lasound
    )
endif()

add_executable(M5Pi-Launcher main.c)
add_dependencies(M5Pi-Launcher lvgl lv_demos lv_drivers lib_qrcode lib_fs lib_qrcode lib_png nes)
target_link_libraries(M5Pi-Launcher launcher ${libs})