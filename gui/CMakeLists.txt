cmake_minimum_required(VERSION 3.8.2...3.20)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

find_package(Qt${QT_VERSION_MAJOR} ${QT_VERSION} COMPONENTS
    Core Gui Qml Quick QuickControls2 Widgets Xml Svg REQUIRED)
if(ANDROID)
  find_package(Qt5AndroidExtras REQUIRED)
endif()

file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

if (QT_VERSION VERSION_LESS 5.99.0)
    add_subdirectory(controls2_515)
    qt_add_resources(noson-gui-resources noson_controls2_515.qrc)
elseif (QT_VERSION VERSION_LESS 6.99.0)
    add_subdirectory(controls2_640)
    qt_add_resources(noson-gui-resources noson_controls2_640.qrc)
else()
    message(FATAL_ERROR "Qt version is not supported")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# Make the files visible on qtcreator
add_custom_target(nosongui_QMlFiles ALL SOURCES ${APP_FILES})

if (MSVC)
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3 /Od /RTC1")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /EHsc /nologo")
    set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /O2 /EHsc /nologo")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /EHsc /nologo")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /Od /RTC1 /EHsc /nologo")
endif ()

set(
    noson-gui_SOURCES
    noson.cpp
    diskcache/diskcachefactory.cpp
    diskcache/cachingnetworkaccessmanager.cpp
    diskcache/cachereply.cpp
)

set(
    noson-gui_HEADERS
    diskcache/diskcachefactory.h
    diskcache/cachingnetworkaccessmanager.h
    diskcache/cachereply.h
)

if(UNIX)
    set(noson-gui_SOURCES ${noson-gui_SOURCES} signalhandler.cpp)
    set(noson-gui_HEADERS ${noson-gui_HEADERS} signalhandler.h)
endif()

add_definitions(-DAPP_VERSION="${APP_VERSION}")

if(ANDROID)
    add_definitions(-frtti) # dynamic_cast: https://github.com/android/ndk/issues/519
    add_library(noson-gui SHARED ${noson-gui_SOURCES} ${noson-gui_HEADERS} ${noson-gui-resources})
elseif(MSVC)
    add_executable(noson-gui WIN32 ${noson-gui_SOURCES} ${noson-gui_HEADERS} ${noson-gui-resources})
else()
    add_executable(noson-gui ${noson-gui_SOURCES} ${noson-gui_HEADERS} ${noson-gui-resources})
endif()

target_link_libraries(noson-gui
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Svg)
if(ANDROID)
    if (QT_VERSION VERSION_LESS 5.99.0)
        target_link_libraries(noson-gui android log Qt5::AndroidExtras)
    else()
        add_definitions(-frtti) # dynamic_cast: https://github.com/android/ndk/issues/519
        set_property(TARGET noson-gui PROPERTY SUFFIX "_${ANDROID_ABI}.so")
        target_link_libraries(noson-gui android log Qt6::CorePrivate)
    endif()
endif()

if(QT_STATICPLUGIN)
    add_definitions(-DQT_STATICPLUGIN)
    target_link_libraries(noson-gui NosonApp NosonThumbnailer NosonMediaScanner)
endif()

if(MSVC)
    target_link_libraries(noson-gui ws2_32)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
    find_library(SOCKET socket REQUIRED)
    find_library(NSL nsl REQUIRED)
    target_link_libraries(noson-gui socket nsl)
endif()
