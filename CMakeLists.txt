project(NosonApp C CXX)
cmake_minimum_required(VERSION 2.8.9)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt5Core)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5QuickControls2)

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

set(APP_VERSION       3.0.0)
set(APP_NAME          noson)
set(APP_ID            "noson.janbar")
set(EXEC              "noson-app")
set(ICON              "noson.png")
set(ICON_FILE         "app/images/noson.png")
set(DESKTOP_FILE_NAME "noson.desktop")

# Set install paths
include(GNUInstallDirs)
set(APP_HARDCODE "noson")
set(DATA_DIR ${CMAKE_INSTALL_DATADIR}/${APP_HARDCODE})
set(DESKTOP_DIR ${CMAKE_INSTALL_DATADIR}/applications)
set(NOSONAPP_DIR ${CMAKE_INSTALL_PREFIX}/${DATA_DIR})
set(ICON ${NOSONAPP_DIR}/${ICON})

configure_file(${DESKTOP_FILE_NAME}.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
        DESTINATION ${DESKTOP_DIR} RENAME ${APP_HARDCODE}.desktop)
install(FILES ${ICON_FILE} DESTINATION ${DATA_DIR})

add_subdirectory(app)
add_subdirectory(backend)

add_custom_target("run" QML2_IMPORT_PATH=${CMAKE_BINARY_DIR}/backend ${CMAKE_BINARY_DIR}/app/noson-app --debug
                    DEPENDS NosonAppbackend NosonAppbackend-qmldir
                    WORKING_DIRECTORY ./app)

