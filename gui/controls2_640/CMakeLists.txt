# make the qml files visible on qtcreator
file(GLOB_RECURSE CONTROLS2_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_CONTROLS2_QMLFiles ALL SOURCES ${CONTROLS2_QML_FILES})
