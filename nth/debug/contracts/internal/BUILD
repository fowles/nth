package(default_visibility = [
    "//nth/debug:__subpackages__",
    "//nth/test:__subpackages__",
])

cc_library(
    name = "contracts",
    hdrs = ["contracts.h"],
    deps = [
        ":on_exit",
        ":responder",
        "//nth/base:macros",
        "//nth/debug/trace/internal:injector",
    ],
)

cc_library(
    name = "on_exit",
    hdrs = ["on_exit.h"],
    deps = [
        "//nth/debug/log",
        "//nth/debug:source_location",
        "//nth/strings:interpolate",
    ],
)

cc_library(
    name = "responder",
    hdrs = ["responder.h"],
    srcs = ["responder.cc"],
    deps = [
        "//nth/configuration:trace",
        "//nth/debug/log",
        "//nth/debug:expectation_result",
        "//nth/debug/property/internal:concepts",
        "//nth/debug/property/internal:implementation",
        "//nth/debug/trace/internal:implementation",
        "@abseil-cpp//absl/synchronization",
    ],
)
