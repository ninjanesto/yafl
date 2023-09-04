load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary", "cc_test")

cc_library(
    name = "yafl-common",
    hdrs = ["src/yafl/Applicative.h",
            "src/yafl/HOF.h",
            "src/yafl/Functor.h",
            "src/yafl/Monad.h",
            "src/yafl/TypeTraits.h"],
    visibility = ["//visibility:public",],
    strip_include_prefix = "src",
)

cc_library(
    name = "yafl-maybe",
    hdrs = ["src/yafl/Maybe.h"],
    strip_include_prefix = "src",
    deps = ["//:yafl-common"],
    visibility = ["//visibility:public",],
)

cc_library(
    name = "yafl-either",
    hdrs = ["src/yafl/Either.h"],
    strip_include_prefix = "src",
    deps = ["//:yafl-common"],
    visibility = ["//visibility:public",],
)

cc_library(
    name = "yafl",
    strip_include_prefix = "src",
    deps = ["//:yafl-common", "//:yafl-maybe", "//:yafl-either"],
    visibility = ["//visibility:public",],
)

cc_test(
    name = "yafl-hof-test",
    srcs = ["tests/hof/HOFTest.cpp",],
    deps = ["@gtest//:gtest",
            "@gtest//:gtest_main",
            "//:yafl-common",],
)

cc_test(
    name = "yafl-maybe-test",
    srcs = ["tests/maybe/MaybeTest.cpp",],
    deps = ["@gtest//:gtest",
            "@gtest//:gtest_main",
            "//:yafl-common",
            "//:yafl-maybe",],
)

cc_test(
    name = "yafl-either-test",
    srcs = ["tests/either/EitherTest.cpp",],
    deps = ["@gtest//:gtest",
            "@gtest//:gtest_main",
            "//:yafl-common",
            "//:yafl-either",],
)