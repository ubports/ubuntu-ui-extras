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

file(GLOB QML_FILES *.qml *.qmltheme)

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
install(FILES qmldir DESTINATION ${PLUGIN_DIR})

