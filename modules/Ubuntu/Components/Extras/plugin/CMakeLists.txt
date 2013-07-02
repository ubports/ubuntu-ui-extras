set(PLUGIN_SRC
    components.cpp
)

set(EXAMPLE_PLUGIN_SRC
    example/example-model.cpp
)

include_directories(
    ${CMAKE_BINARY_DIR}
)

add_library(ubuntu-ui-extras-plugin SHARED ${PLUGIN_SRC} ${PLUGIN_HDRS} ${EXAMPLE_PLUGIN_SRC})
qt5_use_modules(ubuntu-ui-extras-plugin Core Qml Quick)

# Qt5's cmake does not export QT_IMPORTS_DIR, lets query qmake on our own for now
get_target_property(QMAKE_EXECUTABLE Qt5::qmake LOCATION)
function(QUERY_QMAKE VAR RESULT)
  exec_program(${QMAKE_EXECUTABLE} ARGS "-query ${VAR}" RETURN_VALUE retu   rn_code OUTPUT_VARIABLE output )
  if(NOT return_code)
    file(TO_CMAKE_PATH "${output}" output)
    set(${RESULT} ${output} PARENT_SCOPE)
  endif(NOT return_code)
endfunction(QUERY_QMAKE)
query_qmake(QT_INSTALL_QML QT_IMPORTS_DIR)

set(PLUGIN_DIR ${QT_IMPORTS_DIR}/Ubuntu/Components/Extras)
install(TARGETS ubuntu-ui-extras-plugin DESTINATION ${PLUGIN_DIR})

# copy the product of the build (plugin binary) to the test directory before
# unit tests are run, so it can be loaded by them.
# Also copy it one directory up so it can be loaded by autopilot tests when
# running them uninstalled.
add_custom_command(
            TARGET ubuntu-ui-extras-plugin
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:ubuntu-ui-extras-plugin>
                ${CMAKE_BINARY_DIR}/tests/unittests/Ubuntu/Components/Extras/)
add_custom_command(
            TARGET ubuntu-ui-extras-plugin
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:ubuntu-ui-extras-plugin>
                ../)

