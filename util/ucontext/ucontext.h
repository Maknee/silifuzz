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

#ifndef THIRD_PARTY_SILIFUZZ_UTIL_UCONTEXT_UCONTEXT_H_
#define THIRD_PARTY_SILIFUZZ_UTIL_UCONTEXT_UCONTEXT_H_

#include <cstdint>

#include "./util/ucontext/ucontext_types.h"

// Saves current CPU register state into *ucontext.
//
// Very similar to getcontext() from libc, but unlike it:
// * We use a datastruct that (unlike ucontext_t) has only the saved
//   register state.
// * We save eflags.
// * We save all registers including rax.
// * We save all segment registers.
//   Note: We can't write CS and SS segment registers in RestoreUContext();
//   in practice, cs and ss have non-0 values and the others (ds,es,fs,gs)
//   have 0 values.
// * We fully restore the register state and eflags before exiting
//   SaveUContext() inlcuding rax.
//   Thus calling SaveUContext() results in a very minimal register changes
//   for the caller: `ucontext` gets written to rdi as part of calling
//   SaveUContext() and rip gets advanced.
//
// CAVEAT: SaveUContext() must aways be followed by ZeroOut(FP)RegsPadding()
// or FixUp(FP)RegsPadding() if one cares about correctness of everything in
// ucontext->fpregs.
//
// CAVEAT: SaveUContext() does not set all the bytes in UContext.
// If you need that guarantee for certain portions of UContext,
// either 0-initialize UContext or call ZeroOutRegsPadding() after calling
// SaveUContext().
extern "C" void SaveUContext(silifuzz::UContext* ucontext)
    __attribute__((__returns_twice__));

// Similar to above but does not make any syscalls. On x86_64, FS_BASE, GS_BASE
// are not saved.
extern "C" void SaveUContextNoSyscalls(silifuzz::UContext* ucontext)
    __attribute__((__returns_twice__));

// Restores CPU register state from *ucontext.
// This never returns, instead execution continues with the *exact same*
// register state as if we've just returned from the SaveUContext() call
// that created `*ucontext`.
//
// Very similar to setcontext() from libc, but unlike it:
// * We use a different datastruct.
// * We restore eflags.
// * We restore all registers including rax.
// * We do not restore CS and SS segment registers.
//
// REQUIRES: the %rsp value set by `ucontext` points to a writable memory
// region that has at least 16 bytes. Those 16 bytes get overwritten as part of
// executing RestoreUContext() with values of eflags and %rip.
//
// Note that this writing is not a problem for matching end-state expectations
// during snapshot execution since we capture the snapshot's expected memory
// end-state with the effects of those 16 bytes written present (note that
// snapshot execution itself may overwrite those bytes - they are in the free
// portion of its stack).
extern "C" void RestoreUContext(const silifuzz::UContext* ucontext)
    __attribute__((__noreturn__));

// Similar to above but does not make any syscalls. On x86_64, FS_BASE, GS_BASE
// and are not restored.
//
// * CAVEAT * This restores FS and GS selectors. For user mode, the only
// allowed values are the null selectors, which cause the segement bases to
// be reset. If the callee depends on either FS or GS, e.g. TLS pointer in
// FS base, callee needs to set the segment bases separately.
extern "C" void RestoreUContextNoSyscalls(const silifuzz::UContext* ucontext)
    __attribute__((__noreturn__));

// ========================================================================= //

namespace silifuzz {

// Zeroes-our padding/unused portions of the register portions of UContext
// (ucontext->gregs and ucontext->fpregs) w.r.t. SaveUContext() -- the latter
// does not write those areas.
// Each is async-signal-safe.
void ZeroOutRegsPadding(UContext* ucontext);
void ZeroOutGRegsPadding(GRegSet* gregs);
void ZeroOutFPRegsPadding(FPRegSet* fpregs);

// Part of ZeroOutFPRegsPadding() that needs to happen after SaveUContext()
// or ConvertFPRegsFromLibC() from ./convert.h (of course ZeroOutFPRegsPadding()
// or ZeroOutRegsPadding() themselves can be done after SaveUContext() instead).
// It fixes up a part of what SaveUContext() and ucontext_t-creation code
// in the kernel actually write.
void FixUpRegsPadding(UContext* ucontext);
void FixUpGRegsPadding(GRegSet* gregs);
void FixUpFPRegsPadding(FPRegSet* fpregs);

// Returns true iff corresponding zeroing has been done on the arg.
// Has simple, not most-efficient impl: meant for (D)CHECK-s.
bool HasZeroRegsPadding(const UContext& ucontext);
bool HasZeroGRegsPadding(const GRegSet& gregs);
bool HasZeroFPRegsPadding(const FPRegSet& fpregs);

// RestoreUContext may not restore every register to its original state for
// architecture-specific reasons. Some of these registers may cause difficulties
// if they have changed between the time the context was saved and the time it
// was restored. Check that these registers have not changed and that it is safe
// to restore the context.
// Returns true iff specific registers in `actual` have the same values as the
// corresponding registers in `expected`.
bool CriticalUnrestoredRegistersAreSame(const GRegSet& actual,
                                        const GRegSet& expected);

// This accessor function allows architecture-neutral code to reason about the
// state of execution.
// Note: aarch64 would call this the "program counter" but we're defaulting to
// x86_64 terminology when we need to make an arbitrary choice for an
// architecture-neutral name.
uint64_t GetInstructionPointer(const GRegSet& gregs);

// Returns the instruction pointer that points right after the
// call into CurrentInstructionPointer(). Test-only uses so far.
int64_t CurrentInstructionPointer();

}  // namespace silifuzz

#endif  // THIRD_PARTY_SILIFUZZ_UTIL_UCONTEXT_UCONTEXT_H_
