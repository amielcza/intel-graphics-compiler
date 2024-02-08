;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; RUN: igc_opt -igc-buffer-bounds-checking -igc-add-implicit-args -igc-buffer-bounds-checking-patcher -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

define spir_kernel void @kernel(i32 addrspace(1)* %input) nounwind {
  %1 = getelementptr inbounds i32, i32 addrspace(1)* %input, i64 2
  store i32 42, i32 addrspace(1)* %1
  ret void
}

!igc.functions = !{!0}

!0 = !{void (i32 addrspace(1)*)* @kernel, !1}
!1 = !{!2}
!2 = !{!"function_type", i32 0}

; CHECK:          [[BASE_ADDRESS:%[0-9]+]] = ptrtoint i32 addrspace(1)* %input to i64
; CHECK-NEXT:     [[ADDRESS:%[0-9]+]] = ptrtoint i32 addrspace(1)* %1 to i64
; CHECK-NEXT:     [[OFFSET:%[0-9]+]] = sub i64 [[ADDRESS]], [[BASE_ADDRESS]]
; CHECK-NEXT:     [[BUFFER_SIZE_EQ_ZERO:%[0-9]+]] = icmp eq i64 %bufferSize, 0
; CHECK-NEXT:     [[OFFSET_GE_ZERO:%[0-9]+]] = icmp sge i64 [[OFFSET]], 0
; CHECK-NEXT:     [[OFFSET_LT_BUFFER_SIZE:%[0-9]+]] = icmp slt i64 [[OFFSET]], %bufferSize
; CHECK-NEXT:     [[OFFSET_IN_RANGE:%[0-9]+]] = and i1 [[OFFSET_GE_ZERO]], [[OFFSET_LT_BUFFER_SIZE]]
; CHECK-NEXT:     [[CONDITION:%[0-9]+]] = or i1 [[BUFFER_SIZE_EQ_ZERO]], [[OFFSET_IN_RANGE]]
; CHECK-NEXT:     br i1 [[CONDITION]], label %bufferboundschecking.valid, label %bufferboundschecking.invalid

; CHECK:        bufferboundschecking.valid:
; CHECK-NEXT:     store i32 [[VALUE:[0-9]+]], i32 addrspace(1)* {{%[0-9]+}}
; CHECK-NEXT:     br label %bufferboundschecking.end

; CHECK:        bufferboundschecking.invalid:
; CHECK:          call spir_func void @__bufferoutofbounds_assert
; CHECK:          store volatile i32 [[VALUE]], i32 addrspace(1)* null

; CHECK:        declare void @__bufferoutofbounds
