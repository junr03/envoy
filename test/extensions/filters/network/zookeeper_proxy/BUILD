licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_package",
)
load(
    "//test/extensions:extensions_build_system.bzl",
    "envoy_extension_cc_test",
)

envoy_package()

envoy_extension_cc_test(
    name = "filter_test",
    srcs = [
        "filter_test.cc",
    ],
    extension_name = "envoy.filters.network.zookeeper_proxy",
    deps = [
        "//source/extensions/filters/network/zookeeper_proxy:config",
        "//test/mocks/network:network_mocks",
        "//test/test_common:simulated_time_system_lib",
    ],
)

envoy_extension_cc_test(
    name = "config_test",
    srcs = [
        "config_test.cc",
    ],
    extension_name = "envoy.filters.network.zookeeper_proxy",
    deps = [
        "//source/extensions/filters/network/zookeeper_proxy:config",
        "//test/mocks/server:server_mocks",
        "//test/test_common:utility_lib",
        "@envoy_api//envoy/config/filter/network/zookeeper_proxy/v1alpha1:pkg_cc_proto",
    ],
)
