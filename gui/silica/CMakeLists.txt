# make the qml files visible on qtcreator
file(GLOB_RECURSE SILICA_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_SILICA_QMLFiles ALL SOURCES ${SILICA_QML_FILES})
