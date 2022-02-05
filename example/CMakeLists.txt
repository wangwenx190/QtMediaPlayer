set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick QuickControls2 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick QuickControls2 REQUIRED)

set(SOURCES
    qml.qrc
    main.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES example.rc example.manifest)
endif()

add_executable(Demo ${SOURCES})

set_target_properties(Demo PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(Demo PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
    wangwenx190::QtMediaPlayer
)

target_compile_definitions(Demo PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060300
)

if(WIN32)
    set(_WIN32_WINNT_WIN10 0x0A00)
    set(NTDDI_WIN10_CO 0x0A00000B)
    target_compile_definitions(Demo PRIVATE
        _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
        _ENABLE_EXTENDED_ALIGNED_STORAGE NOMINMAX UNICODE
        _UNICODE WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN
        WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
        _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
    )
endif()
