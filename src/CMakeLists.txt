project(Yafl)

add_library(${PROJECT_NAME} INTERFACE)
add_library(Yafl::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE . )

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
