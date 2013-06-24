set(PLUGIN_SRC
    components.cpp
)

set(BROWSER_PLUGIN_SRC
    Browser/history-model.cpp
    Browser/history-matches-model.cpp
)

include_directories(
    ${CMAKE_BINARY_DIR}
)

file(GLOB QML_FILES *.qml *.qmltheme qmldir)
file(GLOB BROWSER_QML_FILES Browser.qml)

add_library(ubuntu-ui-extras-plugin SHARED ${PLUGIN_SRC} ${PLUGIN_HDRS} ${BROWSER_PLUGIN_SRC})
qt5_use_modules(ubuntu-ui-extras-plugin Core Qml Quick Network Sql)

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
install(TARGETS ubuntu-ui-extras-plugin DESTINATION ${PLUGIN_DIR})
install(FILES ${QML_FILES} DESTINATION ${PLUGIN_DIR})
install(FILES ${BROWSER_QML_FILES} DESTINATION ${PLUGIN_DIR})

# copy qml files over to build dir to be able to import them in tests
foreach(qmlFile ${QML_FILES} ${BROWSER_QML_FILES} Browser)
  file(COPY ${qmlFile} DESTINATION ${CMAKE_BINARY_DIR}/tests/unittests/Ubuntu/Components/Extras/)
endforeach(qmlFile)

# copy the product of the build (plugin binary) to the test directory before
# unit tests are run, so it can be loaded by them.
add_custom_command(
            TARGET ubuntu-ui-extras-plugin
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:ubuntu-ui-extras-plugin>
                ${CMAKE_BINARY_DIR}/tests/unittests/Ubuntu/Components/Extras/)

add_subdirectory(Browser)
