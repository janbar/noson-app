# make the qml files visible on qtcreator
file(GLOB_RECURSE SFOS_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_SFOS_QMLFiles ALL SOURCES ${SFOS_QML_FILES})
