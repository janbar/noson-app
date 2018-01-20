# make the qml files visible on qtcreator
file(GLOB ACTIONITEMS_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_ACTIONITEMS_QMLFiles ALL SOURCES ${ACTIONITEMS_QML_FILES})
