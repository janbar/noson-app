# make the qml files visible on qtcreator
file(GLOB FLICKABLES_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_FLICKABLES_QMLFiles ALL SOURCES ${FLICKABLES_QML_FILES})
