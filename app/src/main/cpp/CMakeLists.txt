cmake_minimum_required(VERSION 3.10.2)
project("learnffmpeg")
#头文件设置
include_directories(
        libffmpeg
        recorder
        util
        player
        common

)

#业务代码设置
file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*.cpp
        ${CMAKE_SOURCE_DIR}/*/*.cpp
        ${CMAKE_SOURCE_DIR}/*/*/*.cpp
        ${CMAKE_SOURCE_DIR}/*/*/*/*.cpp
        ${CMAKE_SOURCE_DIR}/*/*/*/*/*.cpp
     #[[   ${CMAKE_SOURCE_DIR}/recorder/audio/*.cpp
        ${CMAKE_SOURCE_DIR}/recorder/common/*.cpp
        ${CMAKE_SOURCE_DIR}/util/*.cpp
        ${CMAKE_SOURCE_DIR}/player/audio/*.cpp
        ${CMAKE_SOURCE_DIR}/player/video/*.cpp
        ${CMAKE_SOURCE_DIR}/player/threeparty/*.cpp
        ${CMAKE_SOURCE_DIR}/player/threeparty/opensl_media/*.cpp
        ${CMAKE_SOURCE_DIR}/player/threeparty/opengl_media/*.cpp
        ${CMAKE_SOURCE_DIR}/common/message_queue/*.cpp
        ${CMAKE_SOURCE_DIR}/common/egl_core/*.cpp]]
        )
add_library(
        learnffmpeg
        SHARED
        ${src-files})

#Android ndk log配置
find_library(
        log-lib
        log)


#ffmpeg 设置
set(jnilibs "${CMAKE_SOURCE_DIR}/../jniLibs")
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libavcodec.so)

add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libavdevice.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libavfilter.so)


add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${jnilibs}/${ANDROID_ABI}/libswscale.so)
set(ffmpeg-libs
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        )


#库链接
target_link_libraries(
        learnffmpeg
        ${log-lib}
        ${ffmpeg-libs}
        OpenSLES
        EGL
        GLESv3
        android
)