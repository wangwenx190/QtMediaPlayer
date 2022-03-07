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

set(PROJ_NAME QtMediaPlayerLoader)

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

if(WIN32 AND (NOT BUILD_STATIC_LOADER))
    enable_language(RC)
    list(APPEND SOURCES qtmediaplayer.rc)
endif()

if(BUILD_STATIC_LOADER)
    add_library(${PROJ_NAME} STATIC ${SOURCES})
else()
    add_library(${PROJ_NAME} SHARED ${SOURCES})
endif()

qt_add_qml_module(${PROJ_NAME}
    URI org.wangwenx190.QtMediaPlayer
    VERSION 1.0
    IMPORTS
        QtQml/auto
        QtQuick/auto
    SOURCES
        ../common/playertypes.h
        ../common/playerinterface.h
        ../common/playerinterface.cpp
        ../common/dummyplayer.h
        ../common/dummyplayer.cpp
)

if(BUILD_STATIC_LOADER)
    target_compile_definitions(${PROJ_NAME} PUBLIC
        QTMEDIAPLAYER_LOADER_STATIC
    )
endif()

target_compile_definitions(${PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    QTMEDIAPLAYER_LOADER_BUILD_LIBRARY
)

target_link_libraries(${PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick
)

target_include_directories(${PROJ_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../common"
)
