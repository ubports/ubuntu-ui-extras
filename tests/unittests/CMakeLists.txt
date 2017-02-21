include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/modules/Ubuntu/Components/Extras/plugin/example/
    ${CMAKE_SOURCE_DIR}/modules/Ubuntu/Components/Extras/plugin/photoeditor/
    )

qt5_add_resources(sampledata_rc_srcs sampledata.qrc)

set(plugin-dir ../../modules/Ubuntu/Components/Extras/plugin)
macro(generate_tests)
    foreach(test ${ARGN})
        add_executable(${test} 
            ${test}.cpp
            ${plugin-dir}/photoeditor/photo-image-provider.cpp
            ${sampledata_rc_srcs}
            )
        qt5_use_modules(${test} Core Qml Quick Test)
        target_link_libraries(${test}
            ${TPL_QT5_LIBRARIES}
            ubuntu-ui-extras-plugin
            )
        add_test(${test} 
            ${CMAKE_CURRENT_BINARY_DIR}/${test}
            -xunitxml -o ${test}.xml
            )
        set_tests_properties(${test} PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=minimal"
            )
    endforeach(test)
endmacro(generate_tests)

generate_tests(
    tst_ExampleModelTests
    tst_PhotoEditorPhoto
    tst_PhotoEditorPhotoImageProvider
    )

add_subdirectory(Printers)
