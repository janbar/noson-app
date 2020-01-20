cmake_minimum_required(VERSION 3.8.2)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
if (BUILD_SAILFISHOS)
  find_package(PkgConfig)
  pkg_check_modules(LIBSAILFISHAPP REQUIRED sailfishapp)
else()
find_package(Qt5QuickControls2 REQUIRED)
endif()
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Svg REQUIRED)

file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

add_subdirectory(components)
add_subdirectory(ui)
if (BUILD_SAILFISHOS)
  add_subdirectory(sfos)
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

qt5_add_resources(noson-gui-resources noson.qrc)

if(ANDROID)
  add_library(noson-gui SHARED ${noson-gui_SOURCES} ${noson-gui-resources})
elseif(MSVC)
  add_executable(noson-gui WIN32 ${noson-gui_SOURCES} ${noson-gui-resources})
else()
  add_executable(noson-gui ${noson-gui_SOURCES} ${noson-gui-resources})
endif()

if(BUILD_SAILFISHOS)
  add_definitions(-DSAILFISHOS)
  include_directories(${LIBSAILFISHAPP_INCLUDE_DIRS})
  target_link_libraries(noson-gui Qt5::Gui Qt5::Qml Qt5::Quick Qt5::Widgets Qt5::Xml Qt5::Svg ${LIBSAILFISHAPP_LIBRARIES})
else()
  target_link_libraries(noson-gui Qt5::Gui Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Widgets Qt5::Xml Qt5::Svg)
endif()

if(QT_STATICPLUGIN)
  add_definitions(-DQT_STATICPLUGIN)
  target_link_libraries(noson-gui NosonApp NosonThumbnailer NosonMediaScanner)
endif()

if(MSVC)
  target_link_libraries(noson-gui ws2_32)
endif()
