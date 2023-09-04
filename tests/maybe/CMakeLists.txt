project(YaflTests)

add_unit_test(
        BASENAME MaybeTest
        LIBS Yafl::Yafl
        SOURCES MaybeTest.cpp
)
