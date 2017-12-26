file(GLOB QML_JS_FILES *.qml *.js)
file(GLOB APP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml *.js *.json)

add_subdirectory(components)
add_subdirectory(ui)

# Make the files visible on qtcreator
add_custom_target(nosonapp_QMlFiles ALL SOURCES ${APP_FILES})

#install(FILES ${QML_JS_FILES} DESTINATION ${NOSONAPP_DIR})
#install(DIRECTORY "images" DESTINATION ${NOSONAPP_DIR})

qt5_add_resources(noson-app-resources noson.qrc)
add_executable(noson-app noson.cpp ${noson-app-resources})
qt5_use_modules(noson-app Gui Qml Quick QuickControls2)

if(MSVC)
  target_link_libraries (noson-app ws2_32)
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noson-app
    DESTINATION ${CMAKE_INSTALL_BINDIR})
