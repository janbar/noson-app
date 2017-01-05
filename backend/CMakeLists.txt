add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/lib/noson/noson)

###############################################################################
# configure
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/lib/noson/noson/include/.
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/noson/src/.
)

set(
    NosonAppbackend_SRCS
    modules/NosonApp/backend.cpp
    modules/NosonApp/sonos.cpp
    modules/NosonApp/player.cpp
    modules/NosonApp/listmodel.cpp
    modules/NosonApp/albumsmodel.cpp
    modules/NosonApp/artistsmodel.cpp
    modules/NosonApp/genresmodel.cpp
    modules/NosonApp/tracksmodel.cpp
    modules/NosonApp/queuemodel.cpp
    modules/NosonApp/radiosmodel.cpp
    modules/NosonApp/playlistsmodel.cpp
    modules/NosonApp/zonesmodel.cpp
    modules/NosonApp/renderingmodel.cpp
    modules/NosonApp/roomsmodel.cpp
    modules/NosonApp/favoritesmodel.cpp
    modules/NosonApp/servicesmodel.cpp
    modules/NosonApp/mediamodel.cpp
)

add_library(NosonAppbackend MODULE
    ${NosonAppbackend_SRCS}
)

add_dependencies (NosonAppbackend noson)
target_link_libraries (NosonAppbackend noson)

set_target_properties(NosonAppbackend PROPERTIES
         LIBRARY_OUTPUT_DIRECTORY NosonApp)

qt5_use_modules(NosonAppbackend Gui Qml Quick)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonAppbackend-qmldir ALL
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/modules/NosonApp/qmldir ${CMAKE_CURRENT_BINARY_DIR}/NosonApp
    DEPENDS ${QMLFILES}
)

# Install plugin file
install(TARGETS NosonAppbackend DESTINATION ${QT_IMPORTS_DIR}/NosonApp/)
install(FILES   modules/NosonApp/qmldir DESTINATION ${QT_IMPORTS_DIR}/NosonApp/)

