file(GLOB QML_FILES *.qml qmldir)

set(QT_IMPORTS_DIR "${CMAKE_INSTALL_LIBDIR}/qt5/qml")

set(PLUGIN_DIR ${QT_IMPORTS_DIR}/Ubuntu/Components/Extras)
install(FILES ${QML_FILES} DESTINATION ${PLUGIN_DIR})

# copy qml files over to build dir to be able to import them in tests
foreach(qmlFile ${QML_FILES} Example.qml PhotoEditor.qml TabsBar.qml Example PhotoEditor TabsBar)
  file(COPY ${qmlFile} DESTINATION ${CMAKE_BINARY_DIR}/tests/unittests/Ubuntu/Components/Extras/)
endforeach(qmlFile)

add_subdirectory(plugin)
add_subdirectory(Example)
add_subdirectory(PhotoEditor)
add_subdirectory(Printers)
add_subdirectory(TabsBar)
