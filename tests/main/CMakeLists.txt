project(TestMain)

set(SOURCES main.cpp)

set(HEADERS )

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(Yafl::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE . )
target_link_libraries(${PROJECT_NAME} GTest::gtest GTest::gmock)

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17)
