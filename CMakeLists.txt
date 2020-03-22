project(noson-app C CXX)
cmake_minimum_required(VERSION 3.8.2)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(INSTALL_TESTS "Install the tests on make install" OFF)
option(BUILD_SAILFISHOS "Build the SailfishOS application" OFF)

if(IOS OR BUILD_SAILFISHOS)
  set(QT_STATICPLUGIN ON CACHE BOOL "Build static plugins" FORCE)
endif()

set(APP_VERSION       "4.3.1")
set(APP_VERSION_CODE  62)
set(APP_ID            "io.github.janbar.noson")
set(APP_NAME          "noson")

if(QT_STATICPLUGIN)
  if(BUILD_SAILFISHOS)
    set(EXEC            "harbour-noson")
    set(ICON_NAME       "${EXEC}")
    set(APP_DESKTOP_ID  "${EXEC}")
  else()
    set(EXEC          	"noson")
    set(ICON_NAME     	"${EXEC}")
    set(APP_DESKTOP_ID	"${EXEC}")
  endif()
else()
  set(EXEC              "noson-app")
  set(ICON_NAME         "${APP_NAME}")
  set(APP_DESKTOP_ID    "${APP_ID}")
endif()

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
  configure_file(
    ${EXEC}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${EXEC}
  )
  install(
    PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${EXEC}"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

if(BUILD_SAILFISHOS)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop "[Desktop Entry]
Name=${APP_NAME}
Exec=${EXEC}
Icon=${ICON_NAME}
Type=Application
Categories=AudioVideo;Audio;
X-Nemo-Application-Type=generic")

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${APP_DESKTOP_ID}.desktop
    DESTINATION ${DESKTOP_DIR}
  )

  install(
    FILES "gui/sfos/images/${ICON_NAME}-86x86.png"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/86x86/apps RENAME "${ICON_NAME}.png"
  )

  install(
    FILES "gui/sfos/images/${ICON_NAME}-108x108.png"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/108x108/apps RENAME "${ICON_NAME}.png"
  )

  install(
    FILES "gui/sfos/images/${ICON_NAME}-128x128.png"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps RENAME "${ICON_NAME}.png"
  )

  install(
    FILES "gui/sfos/images/${ICON_NAME}-172x172.png"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/172x172/apps RENAME "${ICON_NAME}.png"
  )
else()
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

  install(
    FILES "gui/images/${ICON_FILE}"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME "${ICON_NAME}.png"
  )

  configure_file(
    ${APPDATA_FILE_NAME}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
  )

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPDATA_FILE_NAME}
    DESTINATION ${APPDATA_DIR} RENAME ${APP_ID}.appdata.xml
  )
endif()

if(MSVC)
  include(MSVCRuntime)
  set(MSVC_RUNTIME "dynamic")
  configure_msvc_runtime()
endif()

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
    QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/backend/qml ${CMAKE_BINARY_DIR}/gui/noson-gui
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
