cc_library(
    name = "odva_ethernetip",
    srcs = glob(["src/*.cpp"]),
    hdrs = glob([
        "include/odva_ethernetip/**/*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "@boost",
    ],
    strip_include_prefix="include",
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