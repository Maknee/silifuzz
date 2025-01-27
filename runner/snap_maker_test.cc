// Copyright 2022 The SiliFuzz Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "./runner/snap_maker.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "./runner/runner_provider.h"
#include "./snap/testing/snap_test_snapshots.h"
#include "./snap/testing/snap_test_types.h"
#include "./util/checks.h"
#include "./util/testing/status_matchers.h"

namespace silifuzz {
namespace {
using silifuzz::testing::IsOk;
using silifuzz::testing::StatusIs;
using ::testing::HasSubstr;
using ::testing::IsEmpty;

SnapMaker::Options DefaultSnapMakerOptions() {
  SnapMaker::Options opts;
  opts.runner_path = RunnerLocation();
  return opts;
}

// Applies Make(), Record() and Verify() to the snapshot and returns either
// the fixed Snapshot or an error.
absl::StatusOr<Snapshot> Fix(const Snapshot& snapshot) {
  SnapMaker snap_maker(DefaultSnapMakerOptions());
  auto made_snapshot_or = snap_maker.Make(snapshot);
  RETURN_IF_NOT_OK(made_snapshot_or.status());
  auto recorded_snap_or = snap_maker.RecordEndState(made_snapshot_or.value());
  RETURN_IF_NOT_OK(recorded_snap_or.status());
  auto verify_status = snap_maker.Verify(recorded_snap_or.value());
  if (!verify_status.ok()) {
    return verify_status;
  }
  return recorded_snap_or;
}

TEST(SnapMaker, AsExpected) {
  auto endsAsExpectedSnap =
      MakeSnapRunnerTestSnapshot(SnapRunnerTestType::kEndsAsExpected);
  auto result_or = Fix(endsAsExpectedSnap);
  ASSERT_THAT(result_or, IsOk());
}

TEST(SnapMaker, MemoryMismatchSnap) {
  auto memoryMismatchSnap =
      MakeSnapRunnerTestSnapshot(SnapRunnerTestType::kMemoryMismatch);
  auto result_or = Fix(memoryMismatchSnap);
  ASSERT_THAT(result_or, IsOk());
}

TEST(SnapMaker, RandomRegsMismatch) {
  auto regsMismatchRandomSnap =
      MakeSnapRunnerTestSnapshot(SnapRunnerTestType::kRegsMismatchRandom);
  auto result_or = Fix(regsMismatchRandomSnap);
  ASSERT_THAT(result_or, StatusIs(absl::StatusCode::kInternal,
                                  HasSubstr("non-deterministic")));
}

TEST(SnapMaker, SigSegvRead) {
  auto sigSegvReadSnap =
      MakeSnapRunnerTestSnapshot(SnapRunnerTestType::kSigSegvRead);
  auto result_or = Fix(sigSegvReadSnap);
  ASSERT_THAT(result_or, IsOk());
  ASSERT_EQ(result_or->memory_mappings().size(),
            sigSegvReadSnap.memory_mappings().size() + 1)
      << "Expected Make to add 1 extra memory mapping";
  ASSERT_THAT(result_or->negative_memory_mappings(), IsEmpty());
}

TEST(SnapMaker, Idempotent) {
  auto memoryMismatchSnap =
      MakeSnapRunnerTestSnapshot(SnapRunnerTestType::kMemoryMismatch);
  auto result_or = Fix(memoryMismatchSnap);
  ASSERT_THAT(result_or, IsOk());
  auto result2_or = Fix(*result_or);
  ASSERT_THAT(result2_or, IsOk());
  ASSERT_EQ(*result2_or, *result_or);
}

}  // namespace
}  // namespace silifuzz
