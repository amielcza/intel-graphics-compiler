;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

;
; RUN: %opt %use_old_pass_manager% -enable-debugify -GenXLoadStoreLowering -march=genx64 -mcpu=Gen9 -mtriple=spir64-unknown-unknown -enable-ldst-lowering=true -mattr=+ocl_runtime -S < %s 2>&1 | FileCheck %s
; RUN: %opt %use_old_pass_manager% -enable-debugify -GenXLoadStoreLowering -march=genx64 -mcpu=XeHPC -mtriple=spir64-unknown-unknown -enable-ldst-lowering=true -mattr=+ocl_runtime -S < %s 2>&1 | FileCheck --check-prefix=CHECK-LSC %s
;
; CHECK-NOT: WARNING
; CHECK: CheckModuleDebugify: PASS
; CHECK-LSC-NOT: WARNING
; CHECK-LSC: CheckModuleDebugify: PASS

; COM: Basic test on store lowering pass
; COM: simplest store to addrspace(3)

target datalayout = "e-p:64:64-p3:32:32-i64:64-n8:16:32:64"
target triple = "genx64-unknown-unknown"

; Address space 3 (local) operations are lowered into slm(254)/slm intrinsics

define void @replace_store_i8(i8 addrspace(3)* %pi8, i8 %val) {
; CHECK: call void @llvm.genx.scatter.scaled.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i32 0, i16 0, i32 254, i32 %{{[a-zA-Z0-9.]+}}, <1 x i32> zeroinitializer, <1 x i32> %{{[a-zA-Z0-9.]+}})
; CHECK-LSC: [[I8EXT:%[0-9a-zA-Z.]+]] = zext <1 x i8> %{{[0-9a-zA-Z]+}} to <1 x i32>
; CHECK-LSC: call void @llvm.vc.internal.lsc.store.slm.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i8 2, i8 5, i8 1, i8 0, i8 0, i32 0, <1 x i32> %{{[a-zA-Z0-9.]+}}, i16 1, i32 0, <1 x i32> [[I8EXT]])
  store i8 %val, i8 addrspace(3)* %pi8, !nontemporal !0
  ret void
}

define void @replace_store_i16(i16 addrspace(3)* %pi16, i16 %val) {
; CHECK: call void @llvm.genx.scatter.scaled.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i32 1, i16 0, i32 254, i32 %{{[a-zA-Z0-9.]+}}, <1 x i32> zeroinitializer, <1 x i32> %{{[a-zA-Z0-9.]+}})
; CHECK-LSC: [[I16EXT:%[0-9a-zA-Z.]+]] = zext <1 x i16> %{{[0-9a-zA-Z]+}} to <1 x i32>
; CHECK-LSC: call void @llvm.vc.internal.lsc.store.slm.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i8 2, i8 6, i8 1, i8 0, i8 0, i32 0, <1 x i32> %{{[a-zA-Z0-9.]+}}, i16 1, i32 0, <1 x i32> [[I16EXT]])
  store i16 %val, i16 addrspace(3)* %pi16
  ret void
}

define void @replace_store_i32(i32 addrspace(3)* %pi32, i32 %val) {
; CHECK: call void @llvm.genx.scatter.scaled.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i32 2, i16 0, i32 254, i32 %{{[a-zA-Z0-9.]+}}, <1 x i32> zeroinitializer, <1 x i32> %{{[a-zA-Z0-9.]+}})
; CHECK-LSC: call void @llvm.vc.internal.lsc.store.slm.v1i1.i32.v1i32(<1 x i1> <i1 true>, i8 2, i8 3, i8 1, i8 0, i8 0, i32 0, i32 %{{[a-zA-Z0-9.]+}}, i16 1, i32 0, <1 x i32> %{{[a-zA-Z0-9.]+}})
  store i32 %val, i32 addrspace(3)* %pi32
  ret void
}

define void @replace_store_i64(i64 addrspace(3)* %pi64, i64 %val) {
; CHECK: call void @llvm.genx.scatter.scaled.v2i1.v2i32.v2i32(<2 x i1> <i1 true, i1 true>, i32 2, i16 0, i32 254, i32 %{{[a-zA-Z0-9.]+}}, <2 x i32> <i32 0, i32 4>, <2 x i32> %{{[a-zA-Z0-9.]+}})
; CHECK-LSC: call void @llvm.vc.internal.lsc.store.slm.v1i1.i32.v1i64(<1 x i1> <i1 true>, i8 2, i8 4, i8 1, i8 0, i8 0, i32 0, i32 %{{[a-zA-Z0-9.]+}}, i16 1, i32 0, <1 x i64> %{{[a-zA-Z0-9.]+}})
  store i64 %val, i64 addrspace(3)* %pi64
  ret void
}

define void @replace_store_f16(half addrspace(3)* %pf16, half %val) {
; CHECK: call void @llvm.genx.scatter.scaled.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i32 1, i16 0, i32 254, i32 %{{[a-zA-Z0-9.]+}}, <1 x i32> zeroinitializer, <1 x i32> %{{[a-zA-Z0-9.]+}})
; CHECK-LSC: [[F16CAST:%[0-9a-zA-Z.]+]] = bitcast <1 x half> %{{[0-9a-zA-Z]+}} to <1 x i16>
; CHECK-LSC: [[F16EXT:%[0-9a-zA-Z.]+]] = zext <1 x i16> [[F16CAST]] to <1 x i32>
; CHECK-LSC: call void @llvm.vc.internal.lsc.store.slm.v1i1.v1i32.v1i32(<1 x i1> <i1 true>, i8 2, i8 6, i8 1, i8 0, i8 0, i32 0, <1 x i32> %{{[a-zA-Z0-9.]+}}, i16 1, i32 0, <1 x i32> [[F16EXT]])
  store half %val, half addrspace(3)* %pf16
  ret void
}

define void @replace_store_f32(float addrspace(3)* %pf32, float %val) {
; CHECK: call void @llvm.genx.scatter.scaled.v1i1.v1i32.v1f32(<1 x i1> <i1 true>, i32 2, i16 0, i32 254, i32 %{{[a-zA-Z0-9.]+}}, <1 x i32> zeroinitializer, <1 x float> %{{[a-zA-Z0-9.]+}})
; CHECK-LSC: call void @llvm.vc.internal.lsc.store.slm.v1i1.i32.v1f32(<1 x i1> <i1 true>, i8 2, i8 3, i8 1, i8 0, i8 0, i32 0, i32 %{{[a-zA-Z0-9.]+}}, i16 1, i32 0, <1 x float> %{{[a-zA-Z0-9.]+}})
  store float %val, float addrspace(3)* %pf32
  ret void
}

define void @replace_store_f64(double addrspace(3)* %pf64, double %val) {
; CHECK: call void @llvm.genx.scatter.scaled.v2i1.v2i32.v2i32(<2 x i1> <i1 true, i1 true>, i32 2, i16 0, i32 254, i32 %{{[a-zA-Z0-9.]+}}, <2 x i32> <i32 0, i32 4>, <2 x i32> %{{[a-zA-Z0-9.]+}})
; CHECK-LSC: call void @llvm.vc.internal.lsc.store.slm.v1i1.i32.v1f64(<1 x i1> <i1 true>, i8 2, i8 4, i8 1, i8 0, i8 0, i32 0, i32 %{{[a-zA-Z0-9.]+}}, i16 1, i32 0, <1 x double> %{{[a-zA-Z0-9.]+}})
  store double %val, double addrspace(3)* %pf64
  ret void
}

!0 = !{i32 1}
