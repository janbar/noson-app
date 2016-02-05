include(FindGettext)

# Find the translation tools
find_program(INTLTOOL_MERGE intltool-merge)
if(NOT INTLTOOL_MERGE)
 message(FATAL_ERROR "Could not find intltool-merge, please install the intltool package")
endif()

find_program(INTLTOOL_EXTRACT intltool-extract)
if(NOT INTLTOOL_EXTRACT)
 message(FATAL_ERROR "Could not find intltool-extract, please install the intltool package")
endif()

if(NOT CMAKE_INSTALL_LOCALEDIR)
 set(CMAKE_INSTALL_LOCALEDIR "share/locale")
endif()

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

set(DOMAIN ${APP_ID})
set(POT_FILE ${DOMAIN}.pot)
file(GLOB PO_FILES *.po)

file(GLOB_RECURSE I18N_SRC_FILES
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_SOURCE_DIR}/*.qml 
 ${CMAKE_SOURCE_DIR}/*.js 
 ${CMAKE_SOURCE_DIR}/*.cpp 
 ${CMAKE_SOURCE_DIR}/*.h
)
list(APPEND I18N_SRC_FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}.in.h)
list(SORT I18N_SRC_FILES)

add_custom_target(${POT_FILE} ALL
 COMMENT "Generating translation template"
 # Extract the translatable messages from the desktop file 
 COMMAND ${INTLTOOL_EXTRACT} --update --type=gettext/ini
 --srcdir=${CMAKE_BINARY_DIR} ${DESKTOP_FILE_NAME}.in
 # Update the translation file
 COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} -o ${POT_FILE}
 --from-code=UTF-8
 --c++ --qt --add-comments=TRANSLATORS
 --keyword=tr --keyword=tr:1,2 --keyword=N_
 --package-name='${PROJECT}'
 -D ${CMAKE_CURRENT_SOURCE_DIR} ${I18N_SRC_FILES}
 # Copy the up2date translation file to the source directory
 COMMAND ${CMAKE_COMMAND} -E copy ${POT_FILE} ${CMAKE_CURRENT_SOURCE_DIR}
)

foreach(PO_FILE ${PO_FILES})
 get_filename_component(LANG ${PO_FILE} NAME_WE)
 gettext_process_po_files(${LANG} ALL PO_FILES ${PO_FILE})
 set(INSTALL_DIR ${CMAKE_INSTALL_LOCALEDIR}/${LANG}/LC_MESSAGES)
 install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LANG}.gmo
 DESTINATION ${INSTALL_DIR}
 RENAME ${DOMAIN}.mo)
endforeach(PO_FILE)

add_custom_target(${DESKTOP_FILE_NAME} ALL
    COMMENT "Merging translations into ${DESKTOP_FILE_NAME}"
    COMMAND LC_ALL=C ${INTLTOOL_MERGE} -d -u ${CMAKE_SOURCE_DIR}/po ${CMAKE_BINARY_DIR}/${DESKTOP_FILE_NAME}.in ${CMAKE_BINARY_DIR}/${DESKTOP_FILE_NAME} > /dev/null
)

