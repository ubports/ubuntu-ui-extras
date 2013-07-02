add_executable(tst_QmlTests tst_QmlTests.cpp)
qt5_use_modules(tst_QmlTests Core Qml Quick Test QuickTest)
target_link_libraries(tst_QmlTests ${TPL_QT5_LIBRARIES})
add_test(tst_QmlTests ${CMAKE_CURRENT_BINARY_DIR}/tst_QmlTests -import ${CMAKE_BINARY_DIR}/modules)
set_tests_properties(tst_QmlTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=minimal")

# copy qml test files to build dir
file(GLOB qmlTestFiles RELATIVE ${CMAKE_SOURCE_DIR}/tests/unittests/ *qml)
foreach(qmlTestFile ${qmlTestFiles})
  file(COPY ${qmlTestFile} DESTINATION ${CMAKE_BINARY_DIR}/tests/unittests/)
endforeach(qmlTestFile)

set(EXAMPLE_SOURCE_DIR ../../modules/Ubuntu/Components/Extras/plugin/example)

set(HM_SRC
    ${EXAMPLE_SOURCE_DIR}/example-model.cpp
    tst_ExampleModelTests.cpp
)
add_executable(tst_ExampleModelTests ${HM_SRC})
include_directories(${EXAMPLE_SOURCE_DIR})
qt5_use_modules(tst_ExampleModelTests Core Test)
add_test(tst_ExampleModelTests ${CMAKE_CURRENT_BINARY_DIR}/tst_ExampleModelTests)

