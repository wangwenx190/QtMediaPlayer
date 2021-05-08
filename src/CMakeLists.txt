set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick REQUIRED)

set(SOURCES
    qtmediaplayer_global.h
    qtmediaplayer.h
    qtmediaplayer.cpp
)

# MPV backend
list(APPEND SOURCES
    backends/mpv/include/mpv/client.h
    backends/mpv/include/mpv/render.h
    backends/mpv/include/mpv/render_gl.h
    backends/mpv/include/mpv/stream_cb.h
    backends/mpv/qtmpvhelper.h
    backends/mpv/qtmpvhelper.cpp
    backends/mpv/qtmpvplayer.h
    backends/mpv/qtmpvplayer.cpp
    backends/mpv/qtmpvvideotexturenode.h
    backends/mpv/qtmpvvideotexturenode.cpp
)

if(WIN32 AND BUILD_SHARED_LIBS)
    enable_language(RC)
    list(APPEND SOURCES qtmediaplayer.rc)
endif()

add_library(${PROJECT_NAME} ${SOURCES})
add_library(wangwenx190::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        QTMEDIAPLAYER_STATIC
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        target_compile_options(${PROJECT_NAME} PRIVATE /guard:cf)
        target_link_options(${PROJECT_NAME} PRIVATE /GUARD:CF)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060100
    QTMEDIAPLAYER_BUILD_LIBRARY
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
