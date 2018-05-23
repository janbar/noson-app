project(NosonApp C CXX)
cmake_minimum_required(VERSION 2.8.9)

# Do not remove this line, its required for the correct functionality of the Ubuntu-SDK
set(UBUNTU_MANIFEST_PATH "manifest.json.in" CACHE INTERNAL "Tells QtCreator location and name of the manifest file")
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(INSTALL_TESTS "Install the tests on make install" on)
option(CLICK_MODE "Installs to a contained location" on)

set(APP_VERSION       "2.8.3")
set(APP_NAME          "noson")
set(APP_ID            "noson.janbar")
set(EXEC              "noson-app")
set(ICON_FILE         "noson.png")
set(DESKTOP_FILE_NAME "noson.desktop")

# Components PATH 
execute_process(
  COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
  OUTPUT_VARIABLE ARCH_TRIPLET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)   

set(LIBDIR "lib/${ARCH_TRIPLET}")

# Set install paths
if(CLICK_MODE)
  set(NOSONAPP_DIR "share/qml/${APP_NAME}")
  set(CMAKE_INSTALL_PREFIX /)
  set(DATA_DIR ${CMAKE_INSTALL_PREFIX})
  set(DESKTOP_DIR ${DATA_DIR})
  set(ICON "${ICON_FILE}")
  set(PLUGINS_DIR "${LIBDIR}/${APP_NAME}")
  set(NOSON_GUI "${PLUGINS_DIR}/noson-gui")

  # This command figures out the target architecture for use in the manifest file
  execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_ARCH
    OUTPUT_VARIABLE CLICK_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  configure_file(
    manifest.json.in
    ${CMAKE_CURRENT_BINARY_DIR}/manifest.json)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json
    DESTINATION ${CMAKE_INSTALL_PREFIX})
  install(
    FILES "noson.apparmor"
    DESTINATION ${DATA_DIR})
  install(
    FILES "noson-content.json"
    DESTINATION ${DATA_DIR})
  configure_file(
    ${EXEC}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${EXEC})
  install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${EXEC}
    DESTINATION ${DATA_DIR})
  configure_file(
    ${DESKTOP_FILE_NAME}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}.in)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
    DESTINATION ${DESKTOP_DIR} RENAME ${APP_NAME}.desktop)
  install(
    FILES "app/graphics/${ICON_FILE}"
    DESTINATION ${DATA_DIR})
else(CLICK_MODE)
  # Standard install paths
  include(GNUInstallDirs)
  set(DESKTOP_DIR ${CMAKE_INSTALL_DATADIR}/applications)
  set(ICON "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${APP_NAME}/${ICON_FILE}")
  set(PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${APP_NAME}")
  set(NOSON_GUI "${PLUGINS_DIR}/noson-gui")

  configure_file(
    ${EXEC}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${EXEC})
  install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${EXEC}
    DESTINATION ${CMAKE_INSTALL_BINDIR})
  configure_file(
    ${DESKTOP_FILE_NAME}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}.in)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
    DESTINATION ${DESKTOP_DIR} RENAME ${APP_NAME}.desktop)
  install(
    FILES noson-content.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/content-hub/peers/ RENAME ${APP_NAME})
  install(
    FILES "app/graphics/${ICON_FILE}"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${APP_NAME})
endif(CLICK_MODE)

add_subdirectory(app)
add_subdirectory(backend)
add_subdirectory(po)

add_custom_target("run" QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/backend ${CMAKE_BINARY_DIR}/app/noson-gui --debug
  DEPENDS NosonAppbackend NosonAppbackend-qmldir
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# No op custom target for all not compiled files, so they show up in the QtCreator project tree
add_custom_target("nosonapp_ClickFiles" ALL SOURCES "noson.apparmor" "manifest.json.in" "noson-content.json")


configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY
)

add_custom_target(
  "uninstall"
  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
