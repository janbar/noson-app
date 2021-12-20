cmake_minimum_required(VERSION 3.8.2)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

set(QT_MIN_VERSION 5.9.5)
find_package(Qt5Core ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickControls2 REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Svg REQUIRED)

file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

if(Qt5Core_VERSION VERSION_LESS 5.15.0)
  add_subdirectory(controls2_509)
  qt5_add_resources(noson-gui-resources noson_controls2_509.qrc)
else()
  add_subdirectory(controls2_515)
  qt5_add_resources(noson-gui-resources noson_controls2_515.qrc)
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

target_link_libraries(noson-gui Qt5::Gui Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Widgets Qt5::Xml Qt5::Svg)

if(QT_STATICPLUGIN)
  add_definitions(-DQT_STATICPLUGIN)
  target_link_libraries(noson-gui NosonApp NosonThumbnailer NosonMediaScanner)
endif()

if(MSVC)
  target_link_libraries(noson-gui ws2_32)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
  find_library(SOCKET socket REQUIRED)
  target_link_libraries(noson-gui socket)
endif()
