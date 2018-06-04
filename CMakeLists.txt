project(NosonApp C CXX)
cmake_minimum_required(VERSION 3.1.0)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(INSTALL_TESTS "Install the tests on make install" on)

set(APP_VERSION       "3.4.0")
set(APP_NAME          "noson")
set(APP_ID            "io.github.janbar.noson")
set(EXEC              "noson-app")
set(ICON_FILE         "noson.png")
set(DESKTOP_FILE_NAME "noson.desktop")
set(APPDATA_FILE_NAME "noson.appdata.xml")

# Components PATH
execute_process(
  COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
  OUTPUT_VARIABLE ARCH_TRIPLET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(LIBDIR "lib/${ARCH_TRIPLET}")

# Set install paths
include(GNUInstallDirs)
set(APPDATA_DIR ${CMAKE_INSTALL_DATADIR}/metainfo)
set(DESKTOP_DIR ${CMAKE_INSTALL_DATADIR}/applications)
set(PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${APP_NAME}")
set(NOSON_GUI "${PLUGINS_DIR}/noson-gui")
set(NOSON_CLI "${PLUGINS_DIR}/noson-cli")

configure_file(
  ${EXEC}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${EXEC}
)

install(
  PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${EXEC}"
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(
  ${DESKTOP_FILE_NAME}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
  DESTINATION ${DESKTOP_DIR} RENAME ${APP_ID}.desktop
)

install(
  FILES "gui/images/${ICON_FILE}"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)

configure_file(
  ${APPDATA_FILE_NAME}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
  DESTINATION ${APPDATA_DIR} RENAME ${APP_ID}.appdata.xml
)

add_subdirectory(gui)
add_subdirectory(backend)
add_subdirectory(cli)

add_custom_target(
  "run"
  QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/backend ${CMAKE_BINARY_DIR}/gui/noson-gui --debug
  DEPENDS NosonAppbackend NosonAppbackend-qmldir
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gui
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY
)

add_custom_target(
  "uninstall"
  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
