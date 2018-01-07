# Automatically create moc files
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickControls2 REQUIRED)

file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

add_subdirectory(components)
add_subdirectory(ui)

# Make the files visible on qtcreator
add_custom_target(nosongui_QMlFiles ALL SOURCES ${APP_FILES})

set(
  diskcache_SRCS
  diskcache/diskcachefactory.cpp
  diskcache/diskcachefactory.h
  diskcache/cachingnetworkaccessmanager.cpp
  diskcache/cachingnetworkaccessmanager.h
  diskcache/cachereply.cpp
  diskcache/cachereply.h
)

qt5_add_resources(noson-gui-resources noson.qrc)
add_executable(noson-gui noson.cpp ${noson-gui-resources} ${diskcache_SRCS})
qt5_use_modules(noson-gui Gui Qml Quick QuickControls2)

if(MSVC)
  target_link_libraries (noson-gui ws2_32)
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noson-gui
	DESTINATION ${PLUGINS_DIR})
