#[[
  MIT License

  Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
]]

set(PROJ_NAME QtMediaPlayer_MPVBackend)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick REQUIRED)
if(UNIX AND (NOT APPLE) AND (${QT_VERSION_MAJOR} LESS 6))
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS X11Extras REQUIRED)
endif()

set(SOURCES
    mpvbackend.qrc
    mpvbackend_global.h
    mpvqthelper.h
    mpvqthelper.cpp
    mpvplayer.h
    mpvplayer.cpp
    mpvvideotexturenode.h
    mpvvideotexturenode.cpp
    mpvbackend.h
    mpvbackend.cpp
)

if(WIN32 AND (NOT BUILD_STATIC_PLUGINS))
    enable_language(RC)
    list(APPEND SOURCES mpvbackend.rc)
endif()

if(BUILD_STATIC_PLUGINS)
    add_library(${PROJ_NAME} STATIC ${SOURCES})
else()
    add_library(${PROJ_NAME} SHARED ${SOURCES})
endif()

target_include_directories(${PROJ_NAME} PRIVATE
    include
)

set_target_properties(${PROJ_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtmediaplayer
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/qtmediaplayer
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/qtmediaplayer
)

if(BUILD_STATIC_PLUGINS)
    target_compile_definitions(${PROJ_NAME} PUBLIC
        QTMEDIAPLAYER_PLUGIN_STATIC
    )
endif()

target_compile_definitions(${PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    MPV_ENABLE_DEPRECATED=0
    BUILD_MPV_STATIC # Needed by MPV's own headers
    QTMEDIAPLAYER_PLUGIN_BUILD_LIBRARY
)

target_link_libraries(${PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick
    QtMediaPlayerCommon
)
if(UNIX AND (NOT APPLE) AND (${QT_VERSION_MAJOR} LESS 6))
    target_link_libraries(${PROJ_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::X11Extras
    )
endif()

target_include_directories(${PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

if(MSVC)
    target_compile_options(${PROJ_NAME} PRIVATE
        /W4 /WX
    )
else()
    target_compile_options(${PROJ_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()
