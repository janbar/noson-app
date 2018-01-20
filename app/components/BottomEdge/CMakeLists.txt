# make the qml files visible on qtcreator
file(GLOB BOTTOMEDGE_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_BOTTOMEDGE_QMLFiles ALL SOURCES ${BOTTOMEDGE_QML_FILES})
