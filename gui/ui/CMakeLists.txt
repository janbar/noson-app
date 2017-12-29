# make the qml files visible on qtcreator
file(GLOB UI_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_UI_QMLFiles ALL SOURCES ${UI_QML_FILES})
