cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

set(lib_dir ${CMAKE_SOURCE_DIR}/../../../src/main/jniLibs/${ANDROID_ABI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 \
        -fpermissive -D__STDC_FORMAT_MACROS \
        -D__STDC_CONSTANT_MACROS \
        -D__STDC_LIMIT_MACROS -Wformat \
        ")

include_directories(.)
include_directories(
    include/ffmpeg
    )

add_library(libavfilter STATIC
    IMPORTED)
set_target_properties(libavfilter PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libavfilter.a)

add_library(libavformat STATIC
    IMPORTED)
set_target_properties(libavformat PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libavformat.a)

add_library(libavcodec STATIC
    IMPORTED)
set_target_properties(libavcodec PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libavcodec.a)

add_library(libswscale STATIC
    IMPORTED)
set_target_properties(libswscale PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libswscale.a)

add_library(libavutil STATIC
    IMPORTED)
set_target_properties(libavutil PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libavutil.a)

add_library(libpostproc STATIC
    IMPORTED)
set_target_properties(libpostproc PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libpostproc.a)

add_library(libavdevice STATIC
    IMPORTED)
set_target_properties(libavdevice PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libavdevice.a)

add_library(libswresample STATIC
    IMPORTED)
set_target_properties(libswresample PROPERTIES
    IMPORTED_LOCATION
    ${lib_dir}/libswresample.a)

add_library(nativeNiroFfmpeg SHARED
    src/jni_bridge.cpp
    )

target_link_libraries(nativeNiroFfmpeg
    libavfilter
    libavformat
    libavcodec
    libswscale
    libavutil
    libpostproc
    libavdevice
    libswresample
    -ljnigraphics
    log
    m
    z
    )