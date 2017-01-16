file(GLOB QML_FILES *.qml *.js *.png)
install(FILES ${QML_FILES} DESTINATION ${PLUGIN_DIR}/TabsBar)
