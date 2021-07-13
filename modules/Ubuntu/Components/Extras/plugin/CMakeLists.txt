include(FindPkgConfig)
pkg_check_modules(EXIV2 REQUIRED exiv2)             # photoeditor

set(PLUGIN_SRC
    components.cpp
)

set(EXAMPLE_PLUGIN_SRC
    example/example-model.cpp
)

set(PHOTO_EDITOR_PLUGIN_SRC
    photoeditor/file-utils.cpp
    photoeditor/orientation.cpp
    photoeditor/photo-data.cpp
    photoeditor/photo-image-provider.cpp
    photoeditor/photo-metadata.cpp
    photoeditor/imaging.cpp
    photoeditor/photo-edit-thread.cpp
    )

set(TABS_BAR_PLUGIN_SRC
    tabsbar/drag-helper.cpp
)

include_directories(
    ${CMAKE_BINARY_DIR}
)

add_library(lomiri-ui-extras-plugin SHARED ${PLUGIN_SRC} ${PLUGIN_HDRS}
            ${EXAMPLE_PLUGIN_SRC} ${PHOTO_EDITOR_PLUGIN_SRC} ${TABS_BAR_PLUGIN_SRC})
qt5_use_modules(lomiri-ui-extras-plugin Core Qml Quick Xml Widgets)
target_link_libraries(lomiri-ui-extras-plugin
    ${EXIV2_LIBRARIES}
    )


set(QT_IMPORTS_DIR "${CMAKE_INSTALL_LIBDIR}/qt5/qml")

set(PLUGIN_DIR ${QT_IMPORTS_DIR}/Ubuntu/Components/Extras)
install(TARGETS lomiri-ui-extras-plugin DESTINATION ${PLUGIN_DIR})

# copy the product of the build (plugin binary) to the test directory before
# unit tests are run, so it can be loaded by them.
# Also copy it one directory up so it can be loaded by autopilot tests when
# running them uninstalled.
add_custom_command(
            TARGET lomiri-ui-extras-plugin
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:lomiri-ui-extras-plugin>
                ${CMAKE_BINARY_DIR}/tests/unittests/Ubuntu/Components/Extras/)
add_custom_command(
            TARGET lomiri-ui-extras-plugin
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:lomiri-ui-extras-plugin>
                ../)
