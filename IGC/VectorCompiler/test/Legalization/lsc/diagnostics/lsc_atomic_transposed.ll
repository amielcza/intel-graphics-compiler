;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; RUN: %not_for_vc_diag% opt %use_old_pass_manager% -GenXLegalization -march=genx64 -mcpu=XeHPG -mtriple=spir64-unknown-unknown -S %s 2>&1 | FileCheck %s

define void @test(<1 x i32> %0, <1 x i32> %1, <1 x i32> %2) {
entry:
; CHECK: GenXLegalization failed for: < %data = call <1 x i32> @llvm.genx.lsc.xatomic.bti.v1i32.v1i1.v1i32(<1 x i1> <i1 true>, i8 18, i8 0, i8 0, i16 1, i32 0, i8 3, i8 1, i8 2, i8 0, <1 x i32> %0, <1 x i32> %1, <1 x i32> %2, i32 0, <1 x i32> undef){{.*}}>: LSC atomic instructions do not support transposed data order
  %data = call <1 x i32> @llvm.genx.lsc.xatomic.bti.v1i32.v1i1.v1i32(<1 x i1> <i1 true>, i8 18, i8 0, i8 0, i16 1, i32 0, i8 3, i8 1, i8 2, i8 0, <1 x i32> %0, <1 x i32> %1, <1 x i32> %2, i32 0, <1 x i32> undef)
  ret void
}

declare <1 x i32> @llvm.genx.lsc.xatomic.bti.v1i32.v1i1.v1i32(<1 x i1>, i8, i8, i8, i16, i32, i8, i8, i8, i8, <1 x i32>, <1 x i32>, <1 x i32>, i32, <1 x i32>)
