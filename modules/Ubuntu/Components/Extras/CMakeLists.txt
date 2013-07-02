file(GLOB QML_FILES *.qml qmldir)
file(GLOB EXAMPLE_QML_FILES Example.qml)

# Qt5's cmake does not export QT_IMPORTS_DIR, lets query qmake on our own for now
get_target_property(QMAKE_EXECUTABLE Qt5::qmake LOCATION)
function(QUERY_QMAKE VAR RESULT)
  exec_program(${QMAKE_EXECUTABLE} ARGS "-query ${VAR}" RETURN_VALUE return_code OUTPUT_VARIABLE output )
  if(NOT return_code)
    file(TO_CMAKE_PATH "${output}" output)
    set(${RESULT} ${output} PARENT_SCOPE)
  endif(NOT return_code)
endfunction(QUERY_QMAKE)
query_qmake(QT_INSTALL_QML QT_IMPORTS_DIR)

set(PLUGIN_DIR ${QT_IMPORTS_DIR}/Ubuntu/Components/Extras)
install(FILES ${QML_FILES} DESTINATION ${PLUGIN_DIR})
install(FILES ${BROWSER_QML_FILES} DESTINATION ${PLUGIN_DIR})

# copy qml files over to build dir to be able to import them in tests
foreach(qmlFile ${QML_FILES} ${EXAMPLE_QML_FILES} Example)
  file(COPY ${qmlFile} DESTINATION ${CMAKE_BINARY_DIR}/tests/unittests/Ubuntu/Components/Extras/)
endforeach(qmlFile)

add_subdirectory(plugin)
add_subdirectory(Example)
