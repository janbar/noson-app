cmake_minimum_required(VERSION 3.8.2...3.20)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

set(QT_MIN_VERSION 6.4.0)
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED)
find_package(Qt6Core REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Qml REQUIRED)
find_package(Qt6Quick REQUIRED)
find_package(Qt6QuickControls2 REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Xml REQUIRED)
find_package(Qt6Svg REQUIRED)

file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

if(Qt6Core_VERSION VERSION_LESS 6.99.0)
  add_subdirectory(controls2_640)
  qt6_add_resources(noson-gui-resources noson_controls2_640.qrc)
else()
  message(FATAL_ERROR "Invalid Qt6Core version")
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

target_link_libraries(noson-gui Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Widgets Qt6::Xml Qt6::Svg)

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
