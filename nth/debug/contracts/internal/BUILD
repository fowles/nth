package(default_visibility = [
    "//nth/debug:__subpackages__",
    "//nth/test:__subpackages__",
])

cc_library(
    name = "any_formattable_ref",
    hdrs = ["any_formattable_ref.h"],
    deps = [
        "//nth/format",
        "//nth/io/writer",
        "//nth/io/writer:string",
    ],
)



cc_library(
    name = "macros",
    hdrs = ["macros.h"],
    deps = [
        "//nth/base:macros",
    ],
)

cc_library(
    name = "checker",
    hdrs = ["checker.h"],
    deps = [
        "//nth/debug/trace/internal:trace",
        "//nth/format:interpolate",
        "//nth/format",
        "//nth/io/writer",
        "//nth/meta:type",
    ],
)

cc_library(
    name = "contracts",
    hdrs = ["contracts.h"],
    srcs = ["contracts.cc"],
    deps = [
        ":checker",
        ":enabler",
        ":macros",
        ":on_exit",
        "//nth/debug/contracts:violation",
        "//nth/debug/trace",
        "//nth/format",
        "//nth/format:interpolate",
        "//nth/io/writer",
    ],
)

cc_library(
    name = "enabler",
    hdrs = ["enabler.h"],
    deps = [
        "//nth/debug/contracts:violation",
        "//nth/base:section",
        "//nth/debug:source_location",
    ],
)

cc_library(
    name = "on_exit",
    hdrs = ["on_exit.h"],
    deps = [
        "//nth/debug:source_location",
    ],
)
