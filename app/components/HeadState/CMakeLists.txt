# make the qml files visible on qtcreator
file(GLOB HEAD_STATE_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_HEAD_STATE_QMLFiles ALL SOURCES ${HEAD_STATE_QML_FILES})

install(FILES ${HEAD_STATE_QML_FILES} DESTINATION ${NOSONAPP_DIR}/components/HeadState)
