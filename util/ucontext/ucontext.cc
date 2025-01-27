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

#include "./util/ucontext/ucontext.h"

#ifdef MEMORY_SANITIZER
#include <sanitizer/msan_interface.h>
#endif

#include "absl/base/attributes.h"

namespace silifuzz {

void ZeroOutRegsPadding(UContext* ucontext) {
  ZeroOutGRegsPadding(&ucontext->gregs);
  ZeroOutFPRegsPadding(&ucontext->fpregs);
}

void FixUpRegsPadding(UContext* ucontext) {
  FixUpGRegsPadding(&ucontext->gregs);
  FixUpFPRegsPadding(&ucontext->fpregs);
}

bool HasZeroRegsPadding(const UContext& ucontext) {
  return HasZeroGRegsPadding(ucontext.gregs) &&
         HasZeroFPRegsPadding(ucontext.fpregs);
}

bool HasZeroGRegsPadding(const GRegSet& gregs) {
  GRegSet copy = gregs;
  ZeroOutGRegsPadding(&copy);
  return copy == gregs;
}

bool HasZeroFPRegsPadding(const FPRegSet& fpregs) {
  FPRegSet copy = fpregs;
  ZeroOutFPRegsPadding(&copy);
  return copy == fpregs;
}

ABSL_ATTRIBUTE_NOINLINE int64_t CurrentInstructionPointer() {
  return reinterpret_cast<int64_t>(__builtin_return_address(0));
}

}  // namespace silifuzz
