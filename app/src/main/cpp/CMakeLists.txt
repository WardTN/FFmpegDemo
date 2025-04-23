cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(jnilibs ${CMAKE_SOURCE_DIR}/../jniLibs)

set(libname learn-ffmpeg)

include_directories(include)

link_directories(${jnilibs}/${ANDROID_ABI})

file(GLOB src-files ${CMAKE_SOURCE_DIR}/*.cpp)

add_library(
        ${libname}
        SHARED
        ${src-files}
)


set(third-party-libs
        avformat
        avcodec
        avfilter
        swresample
        swscale
        avutil
        fdk-aac
        x264
        )

set(native-libs
        android
        EGL
        GLESv3
        OpenSLES
        log
        m
        z
        )

target_link_libraries(
        ${libname}
        ${log-lib}
        ${third-party-libs}
        ${native-libs}
)