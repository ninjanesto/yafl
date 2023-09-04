project(ACC)

set(SOURCES main.cpp)

set(HEADERS )

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} Yafl::Yafl)

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17)
