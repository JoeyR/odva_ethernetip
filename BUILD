cc_library(
    name = "odva_ethernetip",
    srcs = glob(["odva_ethernetip/*.cpp"]),
    hdrs = glob([
        "odva_ethernetip/**/*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "@boost",
    ],
    strip_include_prefix = "odva_ethernetip",
    copts=["-Wno-format -Wno-reorder-ctor"],
    linkopts = ["-lpthread"],
)

#cc_binary(
#    name = "test",
#    srcs =glob(["main.cc"]),
#    deps = [
#        ":odva_ethernetip",
#    ],
#)
