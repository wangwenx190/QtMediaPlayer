set(BACKEND_NAME QMP_Backend_MDK)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick REQUIRED)
find_package(Vulkan)

set(SOURCES
    # Official MDK headers
    include/mdk/c/global.h
    include/mdk/c/MediaInfo.h
    include/mdk/c/module.h
    include/mdk/c/Player.h
    include/mdk/c/RenderAPI.h
    include/mdk/c/VideoFrame.h
    include/mdk/global.h
    include/mdk/MediaInfo.h
    include/mdk/Player.h
    include/mdk/RenderAPI.h
    include/mdk/VideoFrame.h
    # Backend interface
    ../backendinterface.h
    ../backendinterface.cpp
    # MDK backend
    mdkbackend_global.h
    mdkqthelper.h
    mdkqthelper.cpp
    mdkplayer.h
    mdkplayer.cpp
    mdkvideotexturenode.h
    mdkvideotexturenode.cpp
    mdkvideotexturenode_impl.cpp
    mdkbackend.h
    mdkbackend.cpp
)

if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES mdkbackend.rc)
endif()

add_library(${BACKEND_NAME} SHARED ${SOURCES})
add_library(${PROJECT_NAME}::MDKBackend ALIAS ${BACKEND_NAME})

set_target_properties(${BACKEND_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/QtMediaPlayerBackends
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/QtMediaPlayerBackends
)

if(TARGET Vulkan::Vulkan)
    target_include_directories(${BACKEND_NAME} PRIVATE
        ${Vulkan_INCLUDE_DIRS}
    )
endif()

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
    BUILD_MDK_STATIC # Needed by MDK's own headers
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
