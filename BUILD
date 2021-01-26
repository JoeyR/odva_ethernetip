load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "odva_ethernetip",
    srcs = glob(["src/*.cpp"]),
    hdrs = glob([
        "include/odva_ethernetip/**/*.h",
    ]),
    copts = ["-Wno-format -Wno-reorder-ctor"],
    defines = ["BOOST_ASIO_DISABLE_CONCEPTS"],
    linkopts = ["-lpthread"],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    deps = [
        "@boost",
    ],
)

#cc_binary(
#    name = "test",
#    srcs =glob(["main.cc"]),
#    deps = [
#        ":odva_ethernetip",
#    ],
#)
