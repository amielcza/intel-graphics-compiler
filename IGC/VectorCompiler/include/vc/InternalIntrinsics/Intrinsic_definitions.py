# ========================== begin_copyright_notice ============================
#
# Copyright (C) 2022-2023 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
# =========================== end_copyright_notice =============================

#===----------------------------------------------------------------------===//
#
# This file defines all of the Internal-specific intrinsics, which correspond to
# vISA instructions.
#
# Comment lines with a triple slash ### introduction are extracted and
# appended to docs/Targets/Internal/InternalLangRef.rst to give the Internal backend
# language reference in docs/autogenerated/Targets/Internal/InternalLangRef.rst.
#
#===------------------------------------------------------------------------===#

#------------ Currently Supported Types ----------------------
#PointerTypes = ["ptr_private", "ptr_global", "ptr_constant", "ptr_local", "ptr_generic"]
#FloatingPointTypes = ["half", "float", "double"]
#IntegerTypes = ["bool", "char", "short", "int", "long"]
#AdditionalTypes = ["vararg"]
#IntrinsicsProperties = ["None", "NoMem", "ReadArgMem", "ReadMem", "ReadWriteArgMem", "NoReturn", "NoDuplicate", "Convergent"]
#IntrinsicsProperties may be specified as a comma separated list(e.g., "Convergent,NoMem")
#
# EX. "blah": {"result" : {return_type}, "arguments" : [arg1_type, arg2_type.....], "attributes" : Property }
#
# The "any" type can be followed by a default type if a type is not explicitly specified : Ex. "any:int"
#
# 0 - LLVMMatchType<0>
# 1 - LLVMMatchType<1>
# {int} - LLVMMatchType<{int}>

Imported_Intrinsics = {
## ``llvm.vc.internal.jump.table`` : CMC internal, no VISA
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: integer BasicBlock index in the full set of destinations
## * arg1-N: the full set of switch labels
##
## * Return value: selected label
##
## The intrinsic is a helper for switch jump tables generation. Arg0
## will be used by visa switchjmp as index. Return value and arg1-N are
## used to make ir semantically legal.
##
    "jump_table" : { "result" : "anyptr",
                     "arguments" :  ["anyint", "vararg"],
                     "attributes" :  "NoMem" },

## ``llvm.vc.internal.read.variable.region`` : read a vISA variable region
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: ptr pointer to a global variable that corresponds to a vISA variable
##         (overloaded)
## * arg1: i32 vstride in elements, constant
## * arg2: i32 width in elements, constant
## * arg3: i32 stride in elements, constant
## * arg4: i32 offset in elements, constant
##
## * Return value: iN, fN, vXiN, vXfN the read value (overloaded)
##
## This corresponds to MOV instruction or a general source operand in visa.
## Utilizes technique of using global variable in LLVM IR for predefined
## vISA variables.
##
    "read_variable_region" : { "result": "any",
                               "arguments" : ["anyptr", "int", "int", "int",
                                              "int"],
                               "attributes" : "ReadMem", },

## ``llvm.vc.internal.write.variable.region`` : write a vISA variable region
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: ptr pointer to a global variable that corresponds to a vISA variable
##         (overloaded)
## * arg1: iN, fN, vXiN, vXfN value to write (overloaded)
## * arg2: i32 stride in elements, constant
## * arg3: i32 offset in elements, constant
## * arg4: i1 or vXi1 mask (overloaded)
##
## This corresponds to MOV instruction or a general destination operand in visa.
## Utilizes technique of using global variable in LLVM IR for predefined
## vISA variables.
##
    "write_variable_region" : { "result": "void",
                                "arguments" : ["anyptr", "any", "int",
                                               "int", "anyint"],
                                "attributes" : "WriteMem", },

## ``llvm.vc.internal.cast.to.ptr.explicit`` : convert ptr_generic to
## private/local/global ptr.
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: generic pointer
##
## * Return value: private/local/global pointer
##
## This intrisic attempts to explicitly convert a generic ptr to a
##  private/local/global ptr. If the cast fails the intrisic returns null pointer.
    "cast_to_ptr_explicit" : { "result": "anyptr",
                               "arguments": ["ptr_generic"],
                               "attributes": "NoMem", },

### --------------
### ALU intrinsics
### --------------

## ``llvm.vc.internal.cast.to.bf16`` : convert float into bfloat16
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: input data, f32 scalar or vector (overloaded)
##
## * Return value: i16 scalar or vector (overloaded)
##
## This intrinsic represents float -> bfloat16 conversion operation
    "cast_to_bf16" : { "result": "anyint",
                       "arguments": ["anyfloat"],
                       "attributes": "NoMem", },
## ``llvm.vc.internal.cast.from.bf16`` : convert bfloat16 into float
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: bfloat16 input data, i16 scalar or vector (overloaded)
##
## * Return value: f32 scalar or vector (overloaded)
##
## This intrinsic represents float -> bfloat16 conversion operation
    "cast_from_bf16" : { "result": "anyfloat",
                         "arguments": ["anyint"],
                         "attributes": "NoMem", },

## ``llvm.vc.internal.round.to.tf32`` : round float into tfloat32
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: input data, f32 scalar or vector (overloaded)
##
## * Return value: i32 scalar or vector (overloaded)
##
## This intrinsic represents float -> tfloat32 conversion operation
    "round_to_tf32" : { "result": "anyfloat",
                        "arguments": ["anyint"],
                        "attributes": "NoMem", },

## ``llvm.vc.internal.stochastic.round.to.f16`` : half stochastic rounding operation
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: input data, f32 scalar or vector (overloaded)
## * arg1: random number, i16 scalar or vector of the same width as arg0
##
## * Return value: f16 scalar or vector of the same width as arg0
##
## This intrinsic represents float -> half stochastic rounding operation
    "stochastic_round_to_f16" : { "result": "anyfloat",
                                  "arguments": ["anyfloat", "anyint"],
                                  "attributes": "NoMem", },

### ---------------------------
### Low-level memory intrinsics
### ---------------------------

## ``llvm.vc.internal.lsc.atomic.*``: LSC atomic intrinsics
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: vNxi1 Predicate (overloaded)
## * arg1: i8 Atomic opcode [MBC]
## * arg2: i8 Address size [MBC]
## * arg3: i8 Element size [MBC]
## * arg4: vNi8 Cache controls, where N is a number of supported cache levels [MBC]
## * arg5: i64 Address base (for stateless)
##         i32 BTI (for stateful)
## * arg6: vNxi32 or vNxi64 Address indices (overloaded)
## * arg7: i16 Address scale [MBC]
## * arg8: i32 Address immediate offset [MBC]
## * arg9: 1st source vector for the atomic operation,
##          must be undef for unary operations
## * arg10: 2nd source vector for the atomic operation,
##          must be undef for unary and binary operations
## * arg11: vector to take values for masked simd lanes from
##
## * Return value: the value read from memory, merged with arg12 by predicate
##
    "lsc_atomic_bti": { "result": "anyvector",
                        "arguments": [
                            "anyint", # vNxi1, predicate
                            "char",   # atomic opcode
                            "char",   # address size
                            "char",   # element size
                            "anyint", # cache controls
                            "int",    # i32 BTI
                            "anyint", # vNi32 address offsets
                            "short",  # address scale
                            "int",    # address immediate offset
                            0,        # src1
                            0,        # src2
                            0,        # passthru value
                        ],
                        "attributes": "SideEffects", },
    "lsc_atomic_bss": { "result": "anyvector",
                        "arguments": [
                            "anyint", # vNxi1, predicate
                            "char",   # atomic opcode
                            "char",   # address size
                            "char",   # element size
                            "anyint", # cache controls
                            "int",    # i32 BSS
                            "anyint", # vNi32 address offsets
                            "short",  # address scale
                            "int",    # address immediate offset
                            0,        # src1
                            0,        # src2
                            0,        # passthru value
                        ],
                        "attributes": "SideEffects", },
    "lsc_atomic_slm": { "result": "anyvector",
                        "arguments": [
                            "anyint", # vNxi1, predicate
                            "char",   # atomic opcode
                            "char",   # address size
                            "char",   # element size
                            "anyint", # cache controls
                            "int",    # i32 address base
                            "anyint", # vNi32 address offsets
                            "short",  # address scale
                            "int",    # address immediate offset
                            0,        # src1
                            0,        # src2
                            0,        # passthru value
                        ],
                      "attributes": "SideEffects", },
    "lsc_atomic_ugm": { "result": "anyvector",
                        "arguments": [
                            "anyint", # vNxi1, predicate
                            "char",   # atomic opcode
                            "char",   # address size
                            "char",   # element size
                            "anyint", # cache controls
                            "long",   # i64 address base
                            "anyint", # vNi32 or vNi64 address offsets
                            "short",  # address scale
                            "int",    # address immediate offset
                            0,        # src1
                            0,        # src2
                            0,        # passthru value
                        ],
                        "attributes": "SideEffects", },

## ``llvm.vc.internal.lsc.load.*`` : LSC load intrinsics
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: vNxi1 Predicate (overloaded)
## * arg1: i8 Address size [MBC]
## * arg2: i8 Element size [MBC]
## * arg3: i8 Vector size (number of elements per SIMD lane) [MBC]
##         i8 Channel mask (for quad intrinsics) [MBC]
## * arg4: vNi8 Cache controls, where N is a number of supported cache levels [MBC]
## * arg5: i64 Address base (for stateless)
##         i32 BTI (for stateful)
## * arg6: vNxi32 or vNxi64 Address indices (overloaded)
## * arg7: i16 Address scale [MBC]
## * arg8: i32 Address immediate offset [MBC]
## * arg9: vector to take values for masked simd lanes from
##
## * Return value: the value read from memory, merged with arg10 by predicate
##
    "lsc_load_bti": { "result": "anyvector",
                      "arguments": [
                          "anyint", # vNxi1, predicate
                          "char",   # address size
                          "char",   # element size
                          "char",   # vector size
                          "anyint", # cache controls
                          "int",    # i32 BTI
                          "anyint", # vNi32 address offsets
                          "short",  # address scale
                          "int",    # address immediate offset
                          0,        # passthru value
                      ],
                      "attributes": "ReadMem", },
    "lsc_load_bss": { "result": "anyvector",
                      "arguments": [
                          "anyint", # vNxi1, predicate
                          "char",   # address size
                          "char",   # element size
                          "char",   # vector size
                          "anyint", # cache controls
                          "int",    # i32 BSS
                          "anyint", # vNi32 address offsets
                          "short",  # address scale
                          "int",    # address immediate offset
                          0,        # passthru value
                      ],
                      "attributes": "ReadMem", },
    "lsc_load_slm": { "result": "anyvector",
                      "arguments": [
                          "anyint", # vNxi1, predicate
                          "char",   # address size
                          "char",   # element size
                          "char",   # vector size
                          "anyint", # cache controls
                          "int",    # i32 address base
                          "anyint", # vNi32 address offsets
                          "short",  # address scale
                          "int",    # address immediate offset
                          0,        # passthru value
                      ],
                      "attributes": "ReadMem", },
    "lsc_load_ugm": { "result": "anyvector",
                      "arguments": [
                          "anyint", # vNxi1, predicate
                          "char",   # address size
                          "char",   # element size
                          "char",   # vector size
                          "anyint", # cache controls
                          "long",   # i64 address base
                          "anyint", # vNi32 or vNi64 address offsets
                          "short",  # address scale
                          "int",    # address immediate offset
                          0,        # passthru value
                      ],
                      "attributes": "ReadMem", },

    "lsc_load_quad_bti": { "result": "anyvector",
                           "arguments": [
                               "anyint", # vNxi1, predicate
                               "char",   # address size
                               "char",   # element size
                               "char",   # channel mask
                               "anyint", # cache controls
                               "int",    # i32 BTI
                               "anyint", # vNi32 address offsets
                               "short",  # address scale
                               "int",    # address immediate offset
                               0,        # passthru value
                           ],
                           "attributes": "ReadMem", },
    "lsc_load_quad_bss": { "result": "anyvector",
                           "arguments": [
                               "anyint", # vNxi1, predicate
                               "char",   # address size
                               "char",   # element size
                               "char",   # channel mask
                               "anyint", # cache controls
                               "int",    # i32 BSS
                               "anyint", # vNi32 address offsets
                               "short",  # address scale
                               "int",    # address immediate offset
                               0,        # passthru value
                           ],
                           "attributes": "ReadMem", },
    "lsc_load_quad_slm": { "result": "anyvector",
                           "arguments": [
                               "anyint", # vNxi1, predicate
                               "char",   # address size
                               "char",   # element size
                               "char",   # channel mask
                               "anyint", # cache controls
                               "int",    # i32 address base
                               "anyint", # vNi32 address offsets
                               "short",  # address scale
                               "int",    # address immediate offset
                               0,        # passthru value
                           ],
                           "attributes": "ReadMem", },
    "lsc_load_quad_ugm": { "result": "anyvector",
                           "arguments": [
                               "anyint", # vNxi1, predicate
                               "char",   # address size
                               "char",   # element size
                               "char",   # channel mask
                               "anyint", # cache controls
                               "long",   # i64 address base
                               "anyint", # vNi32 or vNi64 address offsets
                               "short",  # address scale
                               "int",    # address immediate offset
                               0,        # passthru value
                           ],
                           "attributes": "ReadMem", },

## ``llvm.vc.internal.lsc.prefetch.*`` : LSC prefetch intrinsics
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: vNxi1 Predicate (overloaded)
## * arg1: i8 Address size [MBC]
## * arg2: i8 Element size [MBC]
## * arg3: i8 Vector size (number of elements per SIMD lane) [MBC]
##         i8 Channel mask (for quad intrinsics) [MBC]
## * arg4: vNi8 Cache controls, where N is a number of supported cache levels [MBC]
## * arg5: i64 Address base (for stateless)
##         i32 BTI (for stateful)
## * arg6: vNxi32 or vNxi64 Address indices (overloaded)
## * arg7: i16 Address scale [MBC]
## * arg8: i32 Address immediate offset [MBC]
##
## * Return value: void
##
    "lsc_prefetch_bti": { "result": "void",
                          "arguments": [
                              "anyint", # vNxi1, predicate
                              "char",   # address size
                              "char",   # element size
                              "char",   # vector size
                              "anyint", # cache controls
                              "int",    # i32 BTI
                              "anyint", # vNi32 address offsets
                              "short",  # address scale
                              "int",    # address immediate offset
                          ],
                          "attributes": "SideEffects", },
    "lsc_prefetch_bss": { "result": "void",
                          "arguments": [
                              "anyint", # vNxi1, predicate
                              "char",   # address size
                              "char",   # element size
                              "char",   # vector size
                              "anyint", # cache controls
                              "int",    # i32 BSS
                              "anyint", # vNi32 address offsets
                              "short",  # address scale
                              "int",    # address immediate offset
                          ],
                          "attributes": "SideEffects", },
    "lsc_prefetch_ugm": { "result": "void",
                          "arguments": [
                              "anyint", # vNxi1, predicate
                              "char",   # address size
                              "char",   # element size
                              "char",   # vector size
                              "anyint", # cache controls
                              "long",   # i64 address base
                              "anyint", # vNi32 or vNi64 address offsets
                              "short",  # address scale
                              "int",    # address immediate offset
                          ],
                          "attributes": "SideEffects", },

    "lsc_prefetch_quad_bti": { "result": "void",
                               "arguments": [
                                   "anyint", # vNxi1, predicate
                                   "char",   # address size
                                   "char",   # element size
                                   "char",   # channel mask
                                   "anyint", # cache controls
                                   "int",    # i32 BTI
                                   "anyint", # vNi32 address offsets
                                   "short",  # address scale
                                   "int",    # address immediate offset
                               ],
                               "attributes": "SideEffects", },
    "lsc_prefetch_quad_bss": { "result": "void",
                               "arguments": [
                                   "anyint", # vNxi1, predicate
                                   "char",   # address size
                                   "char",   # element size
                                   "char",   # channel mask
                                   "anyint", # cache controls
                                   "int",    # i32 BSS
                                   "anyint", # vNi32 address offsets
                                   "short",  # address scale
                                   "int",    # address immediate offset
                               ],
                               "attributes": "SideEffects", },
    "lsc_prefetch_quad_ugm": { "result": "void",
                               "arguments": [
                                   "anyint", # vNxi1, predicate
                                   "char",   # address size
                                   "char",   # element size
                                   "char",   # channel mask
                                   "anyint", # cache controls
                                   "long",   # i64 address base
                                   "anyint", # vNi32 or vNi64 address offsets
                                   "short",  # address scale
                                   "int",    # address immediate offset
                               ],
                               "attributes": "SideEffects", },

## ``llvm.vc.internal.lsc.store.*`` : LSC store intrinsics
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * arg0: vNxi1 Predicate (overloaded)
## * arg1: i8 Address size [MBC]
## * arg2: i8 Element size [MBC]
## * arg3: i8 Vector size (number of elements per SIMD lane) [MBC]
##         i8 Channel mask (for quad intrinsics) [MBC]
## * arg4: vNi8 Cache controls, where N is a number of supported cache levels [MBC]
## * arg5: i64 Address base (for stateless)
##         i32 BTI (for stateful)
## * arg6: vNxi32 or vNxi64 Address indices (overloaded)
## * arg7: i16 Address scale [MBC]
## * arg8: i32 Address immediate offset [MBC]
## * arg9: Data to write (overloaded)
##
## * Return value: void
##
    "lsc_store_bti": { "result": "void",
                       "arguments": [
                           "anyint", # vNxi1, predicate
                           "char",   # address size
                           "char",   # element size
                           "char",   # vector size
                           "anyint", # cache controls
                           "int",    # i32 BTI
                           "anyint", # vNi32 address offsets
                           "short",  # address scale
                           "int",    # address immediate offset
                           "anyvector", # Data to write
                       ],
                       "attributes": "WriteMem", },
    "lsc_store_bss": { "result": "void",
                       "arguments": [
                           "anyint", # vNxi1, predicate
                           "char",   # address size
                           "char",   # element size
                           "char",   # vector size
                           "anyint", # cache controls
                           "int",    # i32 BSS
                           "anyint", # vNi32 address offsets
                           "short",  # address scale
                           "int",    # address immediate offset
                           "anyvector", # Data to write
                       ],
                       "attributes": "WriteMem", },
    "lsc_store_slm": { "result": "void",
                       "arguments": [
                           "anyint", # vNxi1, predicate
                           "char",   # address size
                           "char",   # element size
                           "char",   # vector size
                           "anyint", # cache controls
                           "int",    # i32 address base
                           "anyint", # vNi32 address offsets
                           "short",  # address scale
                           "int",    # address immediate offset
                           "anyvector", # Data to write
                       ],
                       "attributes": "WriteMem", },
    "lsc_store_ugm": { "result": "void",
                       "arguments": [
                           "anyint", # vNxi1, predicate
                           "char",   # address size
                           "char",   # element size
                           "char",   # vector size
                           "anyint", # cache controls
                           "long",   # i64 address base
                           "anyint", # vNi32 or vNi64 address offsets
                           "short",  # address scale
                           "int",    # address immediate offset
                           "anyvector", # Data to write
                       ],
                       "attributes": "WriteMem", },

    "lsc_store_quad_bti": { "result": "void",
                            "arguments": [
                                "anyint", # vNxi1, predicate
                                "char",   # address size
                                "char",   # element size
                                "char",   # channel mask
                                "anyint", # cache controls
                                "int",    # i32 BTI
                                "anyint", # vNi32 address offsets
                                "short",  # address scale
                                "int",    # address immediate offset
                                "anyvector", # Data to write
                            ],
                            "attributes": "WriteMem", },
    "lsc_store_quad_bss": { "result": "void",
                            "arguments": [
                                "anyint", # vNxi1, predicate
                                "char",   # address size
                                "char",   # element size
                                "char",   # channel mask
                                "anyint", # cache controls
                                "int",    # i32 BSS
                                "anyint", # vNi32 address offsets
                                "short",  # address scale
                                "int",    # address immediate offset
                                "anyvector", # Data to write
                            ],
                            "attributes": "WriteMem", },
    "lsc_store_quad_slm": { "result": "void",
                            "arguments": [
                                "anyint", # vNxi1, predicate
                                "char",   # address size
                                "char",   # element size
                                "char",   # channel mask
                                "anyint", # cache controls
                                "int",    # i32 address base
                                "anyint", # vNi32 address offsets
                                "short",  # address scale
                                "int",    # address immediate offset
                                "anyvector", # Data to write
                            ],
                            "attributes": "WriteMem", },
    "lsc_store_quad_ugm": { "result": "void",
                            "arguments": [
                                "anyint", # vNxi1, predicate
                                "char",   # address size
                                "char",   # element size
                                "char",   # channel mask
                                "anyint", # cache controls
                                "long",   # i64 address base
                                "anyint", # vNi32 or vNi64 address offsets
                                "short",  # address scale
                                "int",    # address immediate offset
                                "anyvector", # Data to write
                            ],
                            "attributes": "WriteMem", },

### --------------------
### Thread ID intrinsics
### --------------------

## ``llvm.vc.internal.logical.thread.id`` : logical global thread ID
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
##
## * Return value: i32 logical global thread ID within a gpu tile
##
    "logical_thread_id" : { "result": "int",
                            "arguments": [],
                            "attributes": "NoMem", },

### ---------------------------
### Print and assert intrinsics
### ---------------------------

## ``llvm.vc.internal.assert.buffer`` : read stateless pointer to assert buffer
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
## ``llvm.vc.internal.assert.buffer`` : read implicit arg print assert ptr
##
## * return value: i64 address of assert buffer
##
    "assert_buffer" : { "result" : "long",
                        "arguments" : [],
                        "attributes" : "ReadMem", },

## ``llvm.vc.internal.print.buffer`` : read stateless pointer to print buffer
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
## ``llvm.vc.internal.print.buffer`` : read implicit arg print buffer ptr
##
## * return value: i64 address of print buffer
##
    "print_buffer" : { "result" : "long",
                       "arguments" : [],
                       "attributes" : "ReadMem", },

## ``llvm.vc.internal.print.format.index`` : add printf format string to collection
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
## ``llvm.vc.internal.print.format.index`` :  return index of printf format string
##
## * arg0: pointer for printf format string
##
## * Return value: the vector value read
##
    "print_format_index" : { "result" : "int",
                             "arguments" : ["anyptr"],
                             "attributes" : "NoMem", },
}
