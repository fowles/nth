package(default_visibility = ["//visibility:public"])

cc_library(
    name = "integer",
    hdrs = ["integer.h"],
    srcs = ["integer.cc"],
    deps = [
        "//nth/debug",
        "//nth/format:interpolate",
        "//nth/memory:bytes",
        "//nth/meta/concepts:core",
        "//nth/io/reader",
        "//nth/io/writer",
    ],
)

cc_library(
    name = "test_traits",
    testonly = True,
    hdrs = ["test_traits.h"],
    deps = [
      "//nth/test",
    ],
)

cc_test(
    name = "integer_test",
    srcs = ["integer_test.cc"],
    deps = [
      ":integer",
      ":test_traits",
      "//nth/test:main",
    ],
)
