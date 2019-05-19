project(noson-app C CXX)
cmake_minimum_required(VERSION 3.1.0)

option(INSTALL_TESTS "Install the tests on make install" on)

set(APP_VERSION       "3.11.8")
set(APP_VERSION_CODE  33)
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

# Set install paths
include(GNUInstallDirs)
set(APPDATA_DIR ${CMAKE_INSTALL_DATADIR}/metainfo)
set(DESKTOP_DIR ${CMAKE_INSTALL_DATADIR}/applications)
set(PLUGINS_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${APP_NAME})
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
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME ${APP_NAME}.png
)

configure_file(
  ${APPDATA_FILE_NAME}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
  DESTINATION ${APPDATA_DIR} RENAME ${APP_ID}.appdata.xml
)

add_subdirectory(backend)
add_subdirectory(gui)
if(ANDROID)
  configure_file(
    ${CMAKE_SOURCE_DIR}/android/AndroidManifest.xml.in
    ${CMAKE_SOURCE_DIR}/android/package-sources/AndroidManifest.xml
  )
  include(android/AddQtAndroidApk.cmake)
  add_dependencies(noson-gui NosonApp NosonApp-qmldir
      NosonThumbnailer NosonThumbnailer-qmldir
      NosonMediaScanner NosonMediaScanner-qmldir
  )
  add_qt_android_apk(noson-apk noson-gui
      NAME ${APP_NAME}
      VERSION_CODE ${APP_VERSION_CODE}
      PACKAGE_NAME ${APP_ID}
      PACKAGE_SOURCES ${CMAKE_SOURCE_DIR}/android/package-sources
      BUILDTOOLS_REVISION ${QT_ANDROID_SDK_BUILDTOOLS_REVISION}
      DEPENDS ssl crypto FLAC FLACXX
      PLUGINS ${CMAKE_BINARY_DIR}/backend/qml
      KEYSTORE ${KEYSTORE_FILE} ${KEYSTORE_ALIAS}
      KEYSTORE_PASSWORD ${KEYSTORE_PASSWORD}
  )
endif()

add_custom_target(
  "run"
  QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/backend/qml ${CMAKE_BINARY_DIR}/gui/noson-gui
  DEPENDS NosonApp NosonApp-qmldir
        NosonThumbnailer NosonThumbnailer-qmldir
        NosonMediaScanner NosonMediaScanner-qmldir
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
