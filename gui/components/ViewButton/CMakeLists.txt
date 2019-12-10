# make the qml files visible on qtcreator
file(GLOB VIEW_BUTTON_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_VIEW_BUTTON_QMLFiles ALL SOURCES ${VIEW_BUTTON_QML_FILES})
