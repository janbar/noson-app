cmake_minimum_required(VERSION 3.1.0)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../backend/
)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickControls2 REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Svg REQUIRED)

file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

add_subdirectory(components)
add_subdirectory(ui)

set(CMAKE_CXX_STANDARD 11)

# Make the files visible on qtcreator
add_custom_target(nosongui_QMlFiles ALL SOURCES ${APP_FILES})

set(
  noson-gui-sources
  noson.cpp
  diskcache/diskcachefactory.cpp
  diskcache/diskcachefactory.h
  diskcache/cachingnetworkaccessmanager.cpp
  diskcache/cachingnetworkaccessmanager.h
  diskcache/cachereply.cpp
  diskcache/cachereply.h
)

if(UNIX)
  set(
    extra-sources
    signalhandler.cpp
    signalhandler.h
  )
else()
  unset(extra-sources)
endif()

add_definitions(-DAPP_VERSION="${APP_VERSION}")

qt5_add_resources(noson-gui-resources noson.qrc)

if(ANDROID)
  add_library(noson-gui SHARED ${noson-gui-sources} ${noson-gui-resources} ${extra-sources})
else()
  add_executable(noson-gui ${noson-gui-sources} ${noson-gui-resources} ${extra-sources})
endif()

target_link_libraries(noson-gui NosonThumbnailer-static Qt5::Gui Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Widgets Qt5::Xml Qt5::Svg)

if(MSVC)
  target_link_libraries (noson-gui ws2_32)
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noson-gui
	DESTINATION ${PLUGINS_DIR})
