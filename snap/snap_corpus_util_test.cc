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

#include "./snap/snap_corpus_util.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/statusor.h"
#include "./common/snapshot.h"
#include "./snap/gen/relocatable_snap_generator.h"
#include "./snap/gen/snap_generator.h"
#include "./snap/testing/snap_test_snapshots.h"
#include "./snap/testing/snap_test_types.h"
#include "./util/file_util.h"
#include "./util/mmapped_memory_ptr.h"
#include "./util/path_util.h"
#include "./util/testing/status_matchers.h"

namespace silifuzz {
namespace {
using silifuzz::testing::IsOk;
using ::testing::UnitTest;

TEST(SnapCorpusUtilTest, LoadCorpusFromFile) {
  // Generate relocatable snaps from runner test snaps.
  std::vector<Snapshot> snapified_corpus;
  {
    SnapGenerator::Options opts = SnapGenerator::Options::V2InputRunOpts();
    Snapshot snapshot =
        MakeSnapRunnerTestSnapshot(SnapRunnerTestType::kFirstSnapRunnerTest);
    absl::StatusOr<Snapshot> snapified_or =
        SnapGenerator::Snapify(snapshot, opts);
    ASSERT_THAT(snapified_or, IsOk());
    snapified_corpus.emplace_back(std::move(snapified_or.value()));
  }

  MmappedMemoryPtr<char> buffer = GenerateRelocatableSnaps(snapified_corpus);
  auto tmpfile = CreateTempFile(
      UnitTest::GetInstance()->current_test_info()->test_case_name());
  ASSERT_TRUE(
      SetContents(*tmpfile, {reinterpret_cast<const char*>(buffer.get()),
                             MmappedMemorySize(buffer)}));
  auto loaded_corpus = LoadCorpusFromFile(tmpfile->c_str());
  EXPECT_EQ(loaded_corpus->size, 1);
  EXPECT_EQ(loaded_corpus->elements[0]->id, snapified_corpus[0].id());
}

}  // namespace
}  // namespace silifuzz
