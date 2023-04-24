;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; RUN: opt %use_old_pass_manager% -GenXPrintfResolution -vc-printf-bif-path=%VC_PRITF_OCL_BIF% -march=genx64 -mcpu=Gen9 -S < %s | FileCheck %s

target datalayout = "e-p:64:64-i64:64-n8:16:32:64"

@str = internal unnamed_addr addrspace(2) constant [13 x i8] c"Hello world!\00", align 1 #0

declare spir_func i32 @_Z18__spirv_ocl_printfPU3AS2c(i8 addrspace(2)*, ...)
; CHECK-NOT: @_Z18__spirv_ocl_printfPU3AS2c

define dllexport spir_kernel void @spirv_hello_world() {
  %printf = call spir_func i32 (i8 addrspace(2)*, ...) @_Z18__spirv_ocl_printfPU3AS2c(i8 addrspace(2)* getelementptr inbounds ([13 x i8], [13 x i8] addrspace(2)* @str, i64 0, i64 0))
; COM:                                                                           |Total|64-bit|  ptr | str  |fmt len|
; CHECK-DAG: %[[PRINTF_INIT:[^ ]+]] = call <4 x i32> @__vc_printf_init(<5 x i32> <i32 0, i32 0, i32 0, i32 0, i32 13>)
; CHECK-DAG: %[[PRINTF_FMT:[^ ]+]] = call <4 x i32> @__vc_printf_fmt(<4 x i32> %[[PRINTF_INIT]], i8 addrspace(2)* getelementptr inbounds ([13 x i8], [13 x i8] addrspace(2)* @str, i64 0, i64 0))
; CHECK-DAG: %printf = call i32 @__vc_printf_ret(<4 x i32> %[[PRINTF_FMT]])
  %user = add i32 %printf, 1
  ret void
}

; CHECK-DAG: define internal spir_func <4 x i32> @__vc_printf_init(<5 x i32>
; CHECK-DAG: define internal spir_func <4 x i32> @__vc_printf_fmt(<4 x i32>
; CHECK-DAG: define internal spir_func i32 @__vc_printf_ret(<4 x i32>
