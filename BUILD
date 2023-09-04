load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary", "cc_test")

cc_library(
    name = "yafl-common",
    hdrs = ["src/Applicative.h",
            "src/Compose.h",
            "src/Functor.h",
            "src/Monad.h",
            "src/TypeTraits.h"],
    visibility = ["//visibility:public",],
    strip_include_prefix = "src",

)

cc_library(
    name = "yafl-maybe",
    hdrs = ["src/Maybe.h"],
    strip_include_prefix = "src",
    deps = ["//:yafl-common"],
    visibility = ["//visibility:public",],
)

cc_library(
    name = "yafl-either",
    hdrs = ["src/Either.h"],
    strip_include_prefix = "src",
    deps = ["//:yafl-common"],
    visibility = ["//visibility:public",],
)

cc_test(
    name = "yafl-maybe-test",
    srcs = ["tests/maybe/MaybeTest.cpp",],
    deps = ["@gtest//:gtest",
            "@gtest//:gtest_main",
            "//:yafl-maybe",],
)

cc_test(
    name = "yafl-either-test",
    srcs = ["tests/either/EitherTest.cpp",],
    deps = ["@gtest//:gtest",
            "@gtest//:gtest_main",
            "//:yafl-either",],
)