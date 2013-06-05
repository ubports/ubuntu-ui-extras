add_executable(tst_QmlTests tst_QmlTests.cpp)
qt5_use_modules(tst_QmlTests Core Qml Quick Test QuickTest)
target_link_libraries(tst_QmlTests ${TPL_QT5_LIBRARIES})
add_test(tst_QmlTests ${CMAKE_CURRENT_BINARY_DIR}/tst_QmlTests -import ${CMAKE_BINARY_DIR}/src)
set_tests_properties(tst_QmlTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=minimal")

# copy qml test files to build dir
file(GLOB qmlTestFiles RELATIVE ${CMAKE_SOURCE_DIR}/tests/unittests/ *qml)
foreach(qmlTestFile ${qmlTestFiles})
  file(COPY ${qmlTestFile} DESTINATION ${CMAKE_BINARY_DIR}/tests/unittests/)
endforeach(qmlTestFile)

set(BROWSER_SOURCE_DIR ../../src/Ubuntu/Components/Extras/Browser)

set(HM_SRC
    ${BROWSER_SOURCE_DIR}/history-model.cpp
    tst_HistoryModelTests.cpp
)
add_executable(tst_HistoryModelTests ${HM_SRC})
include_directories(${BROWSER_SOURCE_DIR})
qt5_use_modules(tst_HistoryModelTests Core Sql Test)
add_test(tst_HistoryModelTests ${CMAKE_CURRENT_BINARY_DIR}/tst_HistoryModelTests)

set(HMM_SRC
    ${BROWSER_SOURCE_DIR}/history-model.cpp
    ${BROWSER_SOURCE_DIR}/history-matches-model.cpp
    tst_HistoryMatchesModelTests.cpp
)
add_executable(tst_HistoryMatchesModelTests ${HMM_SRC})
include_directories(${BROWSER_SOURCE_DIR})
qt5_use_modules(tst_HistoryMatchesModelTests Core Sql Test)
add_test(tst_HistoryMatchesModelTests ${CMAKE_CURRENT_BINARY_DIR}/tst_HistoryMatchesModelTests)
