cc_library(
    name = "odva_ethernetip",
    srcs = glob(["*.cpp"]),
    hdrs = glob([
        "**/*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "@boost",
    ],
    copts=["-Wno-format -Wno-reorder-ctor"],
    linkopts = ["-lpthread"],
)

cc_binary(
    name = "test",
    srcs =glob(["main.cc"]),
    deps = [
        ":odva_ethernetip",
    ],
)
