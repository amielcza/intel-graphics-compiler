;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; REQUIRES: pvc-supported

; RUN: llvm-as %s -o %t.bc
; RUN: ocloc compile -llvm_input -file %t.bc -device pvc -options " -igc_opts 'VISAOptions=-asmToConsole'" 2>&1 | FileCheck %s --check-prefixes=CHECK-ASM

target triple = "spir64-unknown-unknown"

define spir_kernel void @test(i64 %arg1, i32 %arg2, i32 %arg3, i32 %arg4, <2 x i32> %vec, <64 x i16>* %output) {
entry:
  %data = call spir_func <64 x i16> @__builtin_IB_subgroup_block_read_flat_u16_m32k16v2(i64 %arg1, i32 %arg2, i32 %arg3, i32 %arg4, <2 x i32> %vec)
  ; CHECK-ASM: load_block2d.ugm.d16.a64 (1|M0)  r{{[0-9]*}}:31   [r{{[0-9]*}}:1]
  store <64 x i16> %data, <64 x i16>* %output, align 128
  ret void
}

declare spir_func <64 x i16> @__builtin_IB_subgroup_block_read_flat_u16_m32k16v2(i64, i32, i32, i32, <2 x i32>)