load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# General packages
http_archive(
    name = "com_github_mjbots_bazel_deps",
    sha256 = "2e9bfa3f83c959316f6f2d3ab02cc7ab01cb917c21b10e94d62b4c475419d6af",
    strip_prefix = 'bazel_deps-59b22fffdc250a5205bf47f9a0e9161c71f632a4',
    urls = ['https://github.com/mjbots/bazel_deps/archive/59b22fffdc250a5205bf47f9a0e9161c71f632a4.zip'],
)

load("@com_github_mjbots_bazel_deps//tools/workspace:default.bzl", "add_default_repositories")

add_default_repositories()