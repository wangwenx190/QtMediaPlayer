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

set(PROJ_NAME Demo)
set(PROJ_VER 1.0.0.0)
set(PROJ_VER_SHORT 1.0)
set(PROJ_URI wangwenx190.QtMediaPlayer.Demo)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED
    COMPONENTS
        Core Gui Qml Quick QuickTemplates2 QuickControls2 Svg
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED
    COMPONENTS
        Core Gui Qml Quick QuickTemplates2 QuickControls2 Svg
)

configure_file(version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

set(SOURCES
    demo.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    console.h console.cpp
    logger.h logger.cpp
    application.h application.cpp
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES
        demo.rc demo.manifest
        qtwinextras/qwinevent_p.h
        qtwinextras/qwinevent.cpp
        qtwinextras/qwintaskbarbutton.h
        qtwinextras/qwintaskbarbutton.cpp
        qtwinextras/qwintaskbarprogress.h
        qtwinextras/qwintaskbarprogress.cpp
        qtwinextras/qwinthumbnailtoolbar.h
        qtwinextras/qwinthumbnailtoolbar.cpp
        qtwinextras/qwinthumbnailtoolbutton.h
        qtwinextras/qwinthumbnailtoolbutton.cpp
        qtwinextras/qquickiconloader_p.h
        qtwinextras/qquickiconloader.cpp
        qtwinextras/qquicktaskbarbutton_p.h
        qtwinextras/qquicktaskbarbutton.cpp
        qtwinextras/qquickthumbnailtoolbar_p.h
        qtwinextras/qquickthumbnailtoolbar.cpp
        qtwinextras/qquickthumbnailtoolbutton_p.h
        qtwinextras/qquickthumbnailtoolbutton.cpp
    )
endif()

qt_add_executable(${PROJ_NAME} ${SOURCES})

set(QML_SOURCES
    qml/MainWindowBase.qml
    qml/MainWindow.qml
    qml/CloseButton.qml
    qml/MaximizeButton.qml
    qml/MinimizeButton.qml
    qml/TitleBar.qml
    qml/PlayButton.qml
    qml/OpenMediaDialog.qml
    qml/Player.qml
    qml/ProgressSlider.qml
    qml/ControlPanel.qml
    qml/StopButton.qml
    qml/VolumeButton.qml
    qml/+windows/MainWindow.qml
)

qt_add_qml_module(${PROJ_NAME}
    URI ${PROJ_URI}
    VERSION ${PROJ_VER_SHORT}
    IMPORTS
        QtQml/auto
        QtQuick/auto
        QtQuick.Controls/auto
        QtQuick.Layouts/auto
        QtQuick.Dialogs/auto
        org.wangwenx190.QtMediaPlayer/auto
    QML_FILES ${QML_SOURCES}
    SOURCES
        constants.h constants.cpp
        history.h history.cpp
        i18n.h i18n.cpp
        settings.h settings.cpp
        theme.h theme.cpp
        framelesswindow.h framelesswindow.cpp
        os.h os.cpp
)

# QML Type Compiler (qmltc): Compile QML types to C++ code.
# Only available since Qt 6.3
#qt_target_compile_qml_to_cpp(${PROJ_NAME}
#    QML_FILES ${QML_SOURCES}
#)

set_target_properties(${PROJ_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJ_URI}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJ_VER}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJ_VER_SHORT}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(WIN32)
    target_include_directories(${PROJ_NAME} PRIVATE
        qtwinextras
    )
endif()

target_link_libraries(${PROJ_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::QmlPrivate # For qmltc
    Qt${QT_VERSION_MAJOR}::QuickPrivate # For qmltc
    Qt${QT_VERSION_MAJOR}::QuickTemplates2Private # For qmltc
    Qt${QT_VERSION_MAJOR}::QuickControls2Private # For qmltc
    Qt${QT_VERSION_MAJOR}::Svg
    QtMediaPlayerLoader
    QtMediaPlayerCommon
)

if(BUILD_STATIC_PLUGINS)
    target_link_libraries(${PROJ_NAME} PRIVATE
        QtMediaPlayer_MDKBackend
        QtMediaPlayer_MPVBackend
        #QtMediaPlayer_FFmpegBackend
    )
endif()

target_compile_definitions(${PROJ_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    #QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060400
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

if(WIN32)
    set(_WIN32_WINNT_WIN10 0x0A00)
    set(NTDDI_WIN10_CO 0x0A00000B)
    target_compile_definitions(${PROJ_NAME} PRIVATE
        _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
        _ENABLE_EXTENDED_ALIGNED_STORAGE NOMINMAX UNICODE
        _UNICODE WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN _DWMAPI_
        WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
        _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
    )
endif()

if(MSVC)
    target_compile_options(${PROJ_NAME} PRIVATE
        /W3 /WX # We can't use "/W4" here due to some generated code are not warning clean.
    )
else()
    target_compile_options(${PROJ_NAME} PRIVATE
        -Wall -Wextra -Werror
    )
endif()
