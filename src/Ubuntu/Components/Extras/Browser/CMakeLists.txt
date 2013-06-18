add_subdirectory(po)

file(GLOB QML_FILES *.qml *.js)
install(FILES ${QML_FILES} DESTINATION ${PLUGIN_DIR}/Browser)
install(DIRECTORY assets DESTINATION ${PLUGIN_DIR}/Browser)
