add_subdirectory(Delegates)
add_subdirectory(HeadState)
add_subdirectory(Flickables)
add_subdirectory(ListItemActions)
add_subdirectory(Themes)
add_subdirectory(ViewButton)
add_subdirectory(BottomEdge)
add_subdirectory(Dialog)
add_subdirectory(ActivitySpinner)

# make the qml files visible on qtcreator
file(GLOB COMPONENTS_QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml)

add_custom_target(nosonapp_COMPONENTS_QMLFiles ALL SOURCES ${COMPONENTS_QML_FILES})

install(FILES ${COMPONENTS_QML_FILES} DESTINATION ${NOSONAPP_DIR}/components)

