set(BACKEND_NAME QMP_Backend_MPV)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick REQUIRED)

set(SOURCES
    # Interfaces
    ../backendinterface.h
    ../backendinterface.cpp
    ../texturenodeinterface.h
    ../texturenodeinterface.cpp
    # MPV backend
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

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES mpvbackend.rc)
endif()

add_library(${BACKEND_NAME} SHARED ${SOURCES})
add_library(${PROJECT_NAME}::MPVBackend ALIAS ${BACKEND_NAME})

target_include_directories(${BACKEND_NAME} PRIVATE
    include
)

set_target_properties(${BACKEND_NAME} PROPERTIES
    OUTPUT_NAME mpv
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtmediaplayer
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/qtmediaplayer
)

if(MSVC)
    target_compile_options(${BACKEND_NAME} PRIVATE /utf-8)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        target_compile_options(${BACKEND_NAME} PRIVATE /guard:cf)
        target_link_options(${BACKEND_NAME} PRIVATE /GUARD:CF)
    endif()
endif()

target_compile_definitions(${BACKEND_NAME} PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060100
    QTMEDIAPLAYER_BACKEND_BUILD_LIBRARY
    MPV_ENABLE_DEPRECATED=0
)

if(WIN32)
    target_compile_definitions(${BACKEND_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )
endif()

target_link_libraries(${BACKEND_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick
)

target_include_directories(${BACKEND_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)
