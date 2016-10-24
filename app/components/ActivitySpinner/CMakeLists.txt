# make the qml files visible on qtcreator
file(GLOB ACTIVITY_SPINNER_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_ACTIVITY_SPINNER_QMLFiles ALL SOURCES ${ACTIVITY_SPINNER_QML_FILES})

install(FILES ${ACTIVITY_SPINNER_QML_FILES} DESTINATION ${NOSONAPP_DIR}/components/ActivitySpinner)
