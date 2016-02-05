# make the qml files visible on qtcreator
file(GLOB AMBIANCE_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_AMBIANCE_QMLFiles ALL SOURCES ${AMBIANCE_QML_FILES})

