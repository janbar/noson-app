file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

add_subdirectory(components)
add_subdirectory(ui)

# Make the files visible on qtcreator
add_custom_target(nosonapp_QMlFiles ALL SOURCES ${APP_FILES})

install(FILES ${QML_JS_FILES} DESTINATION ${NOSONAPP_DIR})
install(DIRECTORY "graphics" DESTINATION ${NOSONAPP_DIR})

