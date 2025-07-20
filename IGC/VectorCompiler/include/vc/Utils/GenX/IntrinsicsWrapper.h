/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VC_UTILS_INTRINSICS_WRAPPER_H
#define VC_UTILS_INTRINSICS_WRAPPER_H

#include "vc/InternalIntrinsics/InternalIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include <optional>

namespace vc {
// getAnyIntrinsicID: Return LLVM, GenX or Internal intrinsic ID
unsigned getAnyIntrinsicID(const llvm::Function *F);
unsigned getAnyIntrinsicID(const llvm::Value *V);

// isOverloadedRet(ID) - Return true if return type
//  in intrinsic is overloaded
// Note: input of LLVM intrinsic not expected
bool isOverloadedRet(unsigned ID);
// isOverloadedArg(ID, ArgNum) - Return true if ArgumentNum
//  in intrinsic overloaded
// Note: input of LLVM intrinsic not expected
bool isOverloadedArg(unsigned ID, unsigned ArgumentNum);

// Is any of GenX or Internal VC BE intrinsic not equal to not_any_intrinsic
bool isAnyVcIntrinsic(unsigned ID);
bool isAnyVcIntrinsic(const llvm::Function *F);
bool isAnyVcIntrinsic(const llvm::Value *V);

/// isAnyNonTrivialIntrinsic(id) - Is Internal, GenX or LLVM intrinsic, which is
/// not equal to not_any_intrinsic
bool isAnyNonTrivialIntrinsic(unsigned ID);

inline bool isAnyNonTrivialIntrinsic(const llvm::Function *F) {
  return isAnyNonTrivialIntrinsic(getAnyIntrinsicID(F));
}

inline bool isAnyNonTrivialIntrinsic(const llvm::Value *V) {
  return isAnyNonTrivialIntrinsic(getAnyIntrinsicID(V));
}

// getAnyDeclaration - create a LLVM Function declaration
// for an intrinsic, and return it.
//
// The Tys parameter is for intrinsics with overloaded types
//  (e.g., those using iAny, fAny, vAny, or iPTRAny).
// For a declaration of an overloaded intrinsic,
// Tys must provide exactly one type for each overloaded type in the intrinsic.
llvm::Function *getAnyDeclaration(
    llvm::Module *M, unsigned ID,
    llvm::ArrayRef<llvm::Type *> Tys = llvm::ArrayRef<llvm::Type *>());

std::string getAnyName(unsigned id, llvm::ArrayRef<llvm::Type *> Tys);

llvm::Function *getAnyDeclarationForArgs(llvm::Module *M, unsigned ID,
                                         llvm::Type *RetTy,
                                         llvm::ArrayRef<llvm::Value *> Args);

bool isAbsIntrinsic(unsigned ID);

inline bool isAbsIntrinsic(const llvm::Function *F) {
  return isAbsIntrinsic(getAnyIntrinsicID(F));
}

inline bool isAbsIntrinsic(const llvm::Value *V) {
  return isAbsIntrinsic(getAnyIntrinsicID(V));
}

int getTwoAddrOpIndex(llvm::CallInst *CI);

} // namespace vc

#endif /* end of include guard: VC_UTILS_INTRINSICS_WRAPPER_H */
