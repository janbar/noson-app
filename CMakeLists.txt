cmake_minimum_required(VERSION 3.8.2)
cmake_policy(VERSION 3.8.2)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
project(noson-app C CXX)

option(INSTALL_TESTS "Install the tests on make install" OFF)

if(IOS OR ANDROID)
  set(QT_STATICPLUGIN ON CACHE BOOL "Build static plugins" FORCE)
endif()

set(APP_VERSION       "5.2.2")
set(APP_VERSION_CODE  94)
set(APP_ID            "io.github.janbar.noson")
set(APP_NAME          "noson")

set(EXEC              "noson-app")
set(ICON_NAME         "${APP_NAME}")
set(APP_DESKTOP_ID    "${APP_ID}")

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

if(NOT QT_STATICPLUGIN)
  set(PLUGINS_BINARY_DIR ${CMAKE_BINARY_DIR}/backend/qml)
  configure_file(
    ${EXEC}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${EXEC}
  )
  install(
    PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${EXEC}"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
else()
  set(PLUGINS_BINARY_DIR "")
endif()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop "[Desktop Entry]
Name=${APP_NAME}
Exec=${EXEC}
Icon=${ICON_NAME}
Terminal=false
Type=Application
Categories=AudioVideo;Audio;")

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop
  DESTINATION ${DESKTOP_DIR}
)

install(FILES "gui/icons/${ICON_NAME}-8x8.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/8x8/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-16x16.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/16x16/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-22x22.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/22x22/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-24x24.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/24x24/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-32x32.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-36x36.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/36x36/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-42x42.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/42x42/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-48x48.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-64x64.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-72x72.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/72x72/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-80x80.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/80x80/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-96x96.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/96x96/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-128x128.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-192x192.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/192x192/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-256x256.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME "${ICON_NAME}.png")
install(FILES "gui/icons/${ICON_NAME}-512x512.png"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps RENAME "${ICON_NAME}.png")

configure_file(
  ${APPDATA_FILE_NAME}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
  DESTINATION ${APPDATA_DIR} RENAME ${APP_ID}.appdata.xml
)

if(MSVC)
  include(MSVCRuntime)
  set(MSVC_RUNTIME "dynamic")
  configure_msvc_runtime()
endif()

set(QT_MIN_VERSION 5.9.5)
find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Core REQUIRED)

add_subdirectory(backend)
add_subdirectory(gui)

if(QT_STATICPLUGIN)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/gui/noson-gui
          DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME ${EXEC})
else()
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/gui/noson-gui
          DESTINATION ${PLUGINS_DIR})
endif()

if(ANDROID)
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
      PLUGINS ${PLUGINS_BINARY_DIR}
      KEYSTORE ${KEYSTORE_FILE} ${KEYSTORE_ALIAS}
      KEYSTORE_PASSWORD ${KEYSTORE_PASSWORD}
  )
endif()

if(QT_STATICPLUGIN)
  add_custom_target(
    "run"
    ${CMAKE_BINARY_DIR}/${EXEC} --debug
    DEPENDS noson-gui
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
else()
  add_custom_target(
    "run"
    QML2_IMPORT_PATH=${PLUGINS_BINARY_DIR} ${CMAKE_BINARY_DIR}/gui/noson-gui
    DEPENDS noson-gui
            NosonApp NosonApp-qmldir
            NosonThumbnailer NosonThumbnailer-qmldir
            NosonMediaScanner NosonMediaScanner-qmldir
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gui
  )
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY
)

add_custom_target(
  "uninstall"
  ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
