project(NosonApp C CXX)
cmake_minimum_required(VERSION 2.8.9)

# Do not remove this line, its required for the correct functionality of the Ubuntu-SDK
set(UBUNTU_MANIFEST_PATH "manifest.json.in" CACHE INTERNAL "Tells QtCreator location and name of the manifest file")
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt5Core)
find_package(Qt5Qml)
find_package(Qt5Quick)
# find_package(ubuntu-sdk-libs)

# Automatically create moc files
set(CMAKE_AUTOMOC ON)

# Components PATH
execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(QT_IMPORTS_DIR "lib/${ARCH_TRIPLET}")

option(INSTALL_TESTS "Install the tests on make install" on)
option(CLICK_MODE "Installs to a contained location" on)

set(APP_VERSION       2.3.4)
set(APP_NAME          noson)
set(APP_ID            "noson.janbar")
set(MAIN_QML          "Main.qml")
set(ICON              "noson.png")
set(ICON_FILE         "app/graphics/noson.png")
set(DESKTOP_FILE_NAME "noson.desktop")

# Set install paths
if(CLICK_MODE)
  set(NOSONAPP_DIR "share/qml/${APP_NAME}")
  set(CMAKE_INSTALL_PREFIX /)
  set(DATA_DIR /)
  set(DESKTOP_DIR ${DATA_DIR})

  set(EXEC "qmlscene $@ ${NOSONAPP_DIR}/${MAIN_QML}")

  # This command figures out the target architecture for use in the manifest file
  execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_ARCH
    OUTPUT_VARIABLE CLICK_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  configure_file(manifest.json.in ${CMAKE_CURRENT_BINARY_DIR}/manifest.json)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json
          DESTINATION ${CMAKE_INSTALL_PREFIX})
  install(FILES "noson.apparmor" DESTINATION ${DATA_DIR})
  install(FILES "noson-content.json" DESTINATION ${DATA_DIR})
  configure_file(${DESKTOP_FILE_NAME}.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}.in)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
          DESTINATION ${DESKTOP_DIR})
else(CLICK_MODE)
    # Standard install paths
    include(GNUInstallDirs)
    set(APP_HARDCODE noson-app)
    set(DATA_DIR ${CMAKE_INSTALL_DATADIR}/${APP_HARDCODE})
    set(DESKTOP_DIR ${CMAKE_INSTALL_DATADIR}/applications)
    set(NOSONAPP_DIR ${CMAKE_INSTALL_PREFIX}/${DATA_DIR})
    set(ICON ${NOSONAPP_DIR}/${ICON})

    set(EXEC ${APP_HARDCODE})

    configure_file(${APP_HARDCODE}.in
        ${CMAKE_CURRENT_BINARY_DIR}/${APP_HARDCODE})
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${APP_HARDCODE}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
    configure_file(${DESKTOP_FILE_NAME}.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}.in)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
        DESTINATION ${DESKTOP_DIR} RENAME noson-app.desktop)
    install(FILES noson-content.json DESTINATION ${CMAKE_INSTALL_DATADIR}/content-hub/peers/ RENAME ${APP_HARDCODE})
endif(CLICK_MODE)

install(FILES ${ICON_FILE} DESTINATION ${DATA_DIR})

add_subdirectory(app)
add_subdirectory(backend)
add_subdirectory(po)

add_custom_target("run" /usr/bin/qmlscene -I ${CMAKE_BINARY_DIR}/backend  ${CMAKE_SOURCE_DIR}/app/Main.qml --debug
                    DEPENDS NosonAppbackend NosonAppbackend-qmldir
                    WORKING_DIRECTORY ./app)

# No op custom target for all not compiled files, so they show up in the QtCreator project tree
add_custom_target("nosonapp_ClickFiles" ALL SOURCES "noson.apparmor" "manifest.json.in" "noson-content.json")

