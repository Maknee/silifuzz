# Copyright 2022 The SiliFuzz Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Description:
#   Testing helpers and snaphots for the Snap library.
#

load(
    "@silifuzz//util:nolibc.bzl",
    "cc_library_plus_nolibc",
)

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

# =========================================================================== #

cc_library_plus_nolibc(
    name = "snap_test_types",
    testonly = 1,
    hdrs = ["snap_test_types.h"],
)

cc_library(
    name = "snap_test_snapshots",
    testonly = 1,
    srcs = ["snap_test_snapshots.cc"],
    hdrs = ["snap_test_snapshots.h"],
    deps = [
        ":snap_test_types",
        "@silifuzz//common:memory_perms",
        "@silifuzz//common:snapshot",
        "@silifuzz//common:snapshot_test_util",
        "@silifuzz//common:snapshot_util",
        "@silifuzz//util:checks",
        "@silifuzz//util:misc_util",
        "@silifuzz//util/ucontext:ucontext_types",
    ],
)

cc_binary(
    name = "snap_test_snaps_gen",
    testonly = 1,
    srcs = ["snap_test_snaps_gen.cc"],
    deps = [
        ":snap_test_snapshots",
        ":snap_test_types",
        "@silifuzz//common:snapshot",
        "@silifuzz//snap/gen:relocatable_snap_generator",
        "@silifuzz//snap/gen:snap_generator",
        "@silifuzz//util:checks",
        "@silifuzz//util:file_util",
        "@silifuzz//util:misc_util",
        "@silifuzz//util:mmapped_memory_ptr",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

genrule(
    name = "snap_test_snaps_cc",
    testonly = 1,
    outs = ["snap_test_snaps.cc"],
    cmd = "$(location :snap_test_snaps_gen) > $@",
    tools = [":snap_test_snaps_gen"],
)

cc_library_plus_nolibc(
    name = "snap_test_snaps",
    testonly = 1,
    srcs = ["snap_test_snaps.cc"],
    hdrs = ["snap_test_snaps.h"],
    deps = [
        ":snap_test_types",
        "@silifuzz//snap",
        "@silifuzz//util:checks",
        "@silifuzz//util:misc_util",
    ],
)

cc_library(
    name = "snap_generator_test_lib",
    testonly = 1,
    srcs = ["snap_generator_test_lib.cc"],
    hdrs = ["snap_generator_test_lib.h"],
    deps = [
        "@silifuzz//common:mapped_memory_map",
        "@silifuzz//common:memory_perms",
        "@silifuzz//common:snapshot",
        "@silifuzz//snap",
        "@silifuzz//snap/gen:snap_generator",
        "@silifuzz//util:checks",
        "@silifuzz//util:mem_util",
        "@silifuzz//util/ucontext:serialize",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)
