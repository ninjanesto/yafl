project(Yafl)

set(SOURCES )

set(HEADERS Functor.h Applicative.h Compose.h Monad.h Maybe.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(Yafl::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE . )

target_link_libraries(${PROJECT_NAME} )

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

