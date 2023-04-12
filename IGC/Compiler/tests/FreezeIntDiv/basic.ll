;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2022-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================
;
; RUN: igc_opt -igc-freeze-int-div-pass -S %s | FileCheck %s --check-prefixes=CHECK,%LLVM_DEPENDENT_CHECK_PREFIX%
; ------------------------------------------------
; FreezeIntDiv
; ------------------------------------------------

define void @main(i32 %idx) #0 {
entry:
  %0 = udiv i32 %idx, 0
  call void @foo(i32 %0)
  ; CHECK-PRE-LLVM-14: [[RESULT_UDIV:%[a-zA-Z0-9_.%-]+]] = udiv i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[VALUE_UDIV:%[a-zA-Z0-9_.%-]+]] = udiv i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[RESULT_UDIV:%[a-zA-Z0-9_.%-]+]] = freeze i32 [[VALUE_UDIV]]
  ; CHECK-NEXT: call void @foo(i32 [[RESULT_UDIV]])

  %1 = urem i32 %idx, 0
  call void @foo(i32 %1)
  ; CHECK-PRE-LLVM-14: [[RESULT_UREM:%[a-zA-Z0-9_.%-]+]] = urem i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[VALUE_UREM:%[a-zA-Z0-9_.%-]+]] = urem i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[RESULT_UREM:%[a-zA-Z0-9_.%-]+]] = freeze i32 [[VALUE_UREM]]
  ; CHECK-NEXT: call void @foo(i32 [[RESULT_UREM]])

  %2 = sdiv i32 %idx, 0
  call void @foo(i32 %2)
  ; CHECK-PRE-LLVM-14: [[RESULT_SDIV:%[a-zA-Z0-9_.%-]+]] = sdiv i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[VALUE_SDIV:%[a-zA-Z0-9_.%-]+]] = sdiv i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[RESULT_SDIV:%[a-zA-Z0-9_.%-]+]] = freeze i32 [[VALUE_SDIV]]
  ; CHECK-NEXT: call void @foo(i32 [[RESULT_SDIV]])

  %3 = srem i32 %idx, 0
  call void @foo(i32 %3)
  ; CHECK-PRE-LLVM-14: [[RESULT_SREM:%[a-zA-Z0-9_.%-]+]] = srem i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[VALUE_SREM:%[a-zA-Z0-9_.%-]+]] = srem i32 %idx, 0
  ; CHECK-LLVM-14-PLUS: [[RESULT_SREM:%[a-zA-Z0-9_.%-]+]] = freeze i32 [[VALUE_SREM]]
  ; CHECK-NEXT: call void @foo(i32 [[RESULT_SREM]])

  ret void
}

; Function Attrs: noduplicate nounwind
declare void @foo(i32) #1

attributes #0 = { "null-pointer-is-valid"="true" }
attributes #1 = { noduplicate nounwind }
