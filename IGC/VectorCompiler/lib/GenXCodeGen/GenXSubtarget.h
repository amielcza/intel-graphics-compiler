/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//
/// GenXSubtarget : subtarget information
/// -------------------------------------
///
/// GenXSubtarget is the GenX-specific subclass of TargetSubtargetInfo. It takes
/// features detected by the front end (what the Gen architecture is),
/// and exposes flags to the rest of the GenX backend for
/// various features (e.g. whether 64 bit operations are supported).
///
/// Where subtarget features are used is noted in the documentation of GenX
/// backend passes.
///
/// The flags exposed to the rest of the GenX backend are as follows. Most of
/// these are currently not used.
///
//===----------------------------------------------------------------------===//

#ifndef GENXSUBTARGET_H
#define GENXSUBTARGET_H

#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/Pass.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "visa_igc_common_header.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#define GET_SUBTARGETINFO_ENUM
#include "GenXGenSubtargetInfo.inc"

namespace llvm {
class GlobalValue;
class Instruction;
class StringRef;
class TargetMachine;

class GenXSubtarget final : public GenXGenSubtargetInfo {

protected:
  // TargetTriple - What processor and OS we're targeting.
  Triple TargetTriple;

  enum GenXTag {
    GENX_GENERIC,
    GENX_BDW,
    GENX_SKL,
    GENX_BXT,
    GENX_KBL,
    GENX_GLK,
    GENX_CNL,
    GENX_ICLLP,
    GENX_TGLLP,
    GENX_DG1,
    XE_HP_SDV,
  };

  // GenXVariant - GenX Tag identifying the variant to compile for
  GenXTag GenXVariant;

private:

  // EmitCisa Builder - True if we should generate CISA instead of VISA
  bool EmitCisa;

  // HasLongLong - True if subtarget supports long long type
  bool HasLongLong;

  // HasFP64 - True if subtarget supports double type
  bool HasFP64;

  // DisableJmpi - True if jmpi is disabled.
  bool DisableJmpi;

  // DisableVectorDecomposition - True if vector decomposition is disabled.
  bool DisableVectorDecomposition;

  // DisableJumpTables - True if switch to jump tables lowering is disabled.
  bool DisableJumpTables;

  // Only generate warning when callable is used in the middle of the kernel
  bool WarnCallable;
  // Some targets do not support i64 ops natively, we have an option to emulate
  bool EmulateLongLong;

  // True if target supports native 64-bit add
  bool HasAdd64;

  // True if it is profitable to use native DxD->Q multiplication
  bool UseMulDDQ;

  // True if codegenerating for OCL runtime.
  bool OCLRuntime;

  // True if subtarget supports switchjmp visa instruction
  bool HasSwitchjmp;

  // True if subtarget requires WA for nomask instructions under divergent
  // control flow
  bool WaNoMaskFusedEU;

  // True if subtarget supports 32-bit integer division
  bool HasIntDivRem32;

  // True if subtarget supports 32-bit rol/ror instructions
  bool HasBitRotate;

  // True if subtarget gets HWTID from predefined variable
  bool GetsHWTIDFromPredef;

  // Shows which surface should we use for stack
  PreDefined_Surface StackSurf;

public:
  // This constructor initializes the data members to match that
  // of the specified triple.
  //
  GenXSubtarget(const Triple &TT, const std::string &CPU,
                const std::string &FS);

  unsigned getGRFWidth() const { return 32; }

  bool isOCLRuntime() const { return OCLRuntime; }

  // ParseSubtargetFeatures - Parses features string setting specified
  // subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU,
#if LLVM_VERSION_MAJOR >= 12
                              StringRef TuneCPU,
#endif
                              StringRef FS);

  // \brief Reset the features for the GenX target.
  void resetSubtargetFeatures(StringRef CPU, StringRef FS);

public:

  /// * isBDW - true if target is BDW
  bool isBDW() const { return GenXVariant == GENX_BDW; }

  /// * isBDWplus - true if target is BDW or later
  bool isBDWplus() const { return GenXVariant >= GENX_BDW; }

  /// * isSKL - true if target is SKL
  bool isSKL() const { return GenXVariant == GENX_SKL; }

  /// * isSKLplus - true if target is SKL or later
  bool isSKLplus() const { return GenXVariant >= GENX_SKL; }

  /// * isBXT - true if target is BXT
  bool isBXT() const { return GenXVariant == GENX_BXT; }

  /// * isKBL - true if target is KBL
  bool isKBL() const { return GenXVariant == GENX_KBL; }

  /// * isGLK - true if target is GLK
  bool isGLK() const { return GenXVariant == GENX_GLK; }

  /// * isCNL - true if target is CNL
  bool isCNL() const { return GenXVariant == GENX_CNL; }

  /// * isCNLplus - true if target is CNL or later
  bool isCNLplus() const { return GenXVariant >= GENX_CNL; }

  /// * isICLLP - true if target is ICL LP
  bool isICLLP() const { return GenXVariant == GENX_ICLLP; }
  /// * isTGLLP - true if target is TGL LP
  bool isTGLLP() const { return GenXVariant == GENX_TGLLP; }
  /// * isDG1 - true if target is DG1
  bool isDG1() const { return GenXVariant == GENX_DG1; }
  /// * isXEHP - true if target is XEHP
  bool isXEHP() const {
    return GenXVariant == XE_HP_SDV;
  }
  /// * translateMediaWalker - true if translate media walker APIs
  bool translateMediaWalker() const { return GenXVariant >= XE_HP_SDV; }
  // TODO: consider implementing 2 different getters
  /// * has add3 and bfn instructions
  bool hasAdd3Bfn() const { return GenXVariant >= XE_HP_SDV; }
  int dpasWidth() const {
    return 8;
  }
  unsigned bfMixedModeWidth() const {
    return 8;
  }

  /// * emulateLongLong - true if i64 emulation is requested
  bool emulateLongLong() const { return EmulateLongLong; }

  /// * hasLongLong - true if target supports long long
  bool hasLongLong() const { return HasLongLong; }

  /// * hasFP64 - true if target supports double fp
  bool hasFP64() const { return HasFP64; }

  /// * hasAdd64 - true if target supports native 64-bit add/sub
  bool hasAdd64() const { return HasAdd64; }

  /// * useMulDDQ - true if is desired to emit DxD->Q mul instruction
  bool useMulDDQ() const { return UseMulDDQ; }

  /// * disableJmpi - true if jmpi is disabled.
  bool disableJmpi() const { return DisableJmpi; }

  /// * WaNoA32ByteScatteredStatelessMessages - true if there is no A32 byte
  ///   scatter stateless message.
  bool WaNoA32ByteScatteredStatelessMessages() const { return !isCNLplus(); }

  /// * disableVectorDecomposition - true if vector decomposition is disabled.
  bool disableVectorDecomposition() const { return DisableVectorDecomposition; }

  /// * disableJumpTables - true if switch to jump tables lowering is disabled.
  bool disableJumpTables() const { return DisableJumpTables; }

  /// * has switchjmp instruction
  bool hasSwitchjmp() const { return HasSwitchjmp; }

  /// * needsWANoMaskFusedEU() - true if we need to apply WA for NoMask ops
  bool needsWANoMaskFusedEU() const { return WaNoMaskFusedEU; }

  /// * has integer div/rem instruction
  bool hasIntDivRem32() const { return HasIntDivRem32; }

  /// * warnCallable() - true if compiler only generate warning for
  ///   callable in the middle
  bool warnCallable() const { return WarnCallable; }

  /// * hasIndirectGRFCrossing - true if target supports an indirect region
  ///   crossing one GRF boundary
  bool hasIndirectGRFCrossing() const { return isSKLplus(); }

  /// * getMaxSlmSize - returns maximum allowed SLM size (in KB)
  unsigned getMaxSlmSize() const {
    if (isXEHP())
      return 128;
    return 64;
  }

  bool hasThreadPayloadInMemory() const {
    if (isXEHP())
      return true;
    return false;
  }

  /// * hasSad2Support - returns true if sad2/sada2 are supported by target
  bool hasSad2Support() const {
    if (isICLLP() || isTGLLP())
      return false;
    if (isDG1())
      return false;
    if (isXEHP())
      return false;
    return true;
  }

  bool hasBitRotate() const { return HasBitRotate; }

  /// * hneedsArgPatching - some subtarget require special treatment of
  // certain argument types, returns *true* if this is the case.
  bool needsArgPatching() const {
    if (isOCLRuntime())
      return false;
    if (isXEHP())
      return true;
    return false;
  }

  /// * getsHWTIDFromPredef - some subtargets get HWTID from
  // predefined variable instead of sr0, returns *true* for such ones.
  bool getsHWTIDFromPredef() const { return GetsHWTIDFromPredef; }

  // Generic helper functions...
  const Triple &getTargetTriple() const { return TargetTriple; }

  bool isTargetDarwin() const { return TargetTriple.isOSDarwin(); }
  bool isTargetLinux() const { return TargetTriple.isOSLinux(); }

  bool isTargetWindowsMSVC() const {
    return TargetTriple.isWindowsMSVCEnvironment();
  }

  bool isTargetKnownWindowsMSVC() const {
    return TargetTriple.isKnownWindowsMSVCEnvironment();
  }

  bool isTargetWindowsCygwin() const {
    return TargetTriple.isWindowsCygwinEnvironment();
  }

  bool isTargetWindowsGNU() const {
    return TargetTriple.isWindowsGNUEnvironment();
  }

  bool isTargetCygMing() const { return TargetTriple.isOSCygMing(); }

  bool isOSWindows() const { return TargetTriple.isOSWindows(); }

  TARGET_PLATFORM getVisaPlatform() const {
    switch (GenXVariant) {
    case GENX_BDW:
      return TARGET_PLATFORM::GENX_BDW;
    case GENX_SKL:
      return TARGET_PLATFORM::GENX_SKL;
    case GENX_BXT:
      return TARGET_PLATFORM::GENX_BXT;
    case GENX_CNL:
      return TARGET_PLATFORM::GENX_CNL;
    case GENX_ICLLP:
      return TARGET_PLATFORM::GENX_ICLLP;
    case GENX_TGLLP:
      return TARGET_PLATFORM::GENX_TGLLP;
    case GENX_DG1:
      return TARGET_PLATFORM::GENX_TGLLP;
    case XE_HP_SDV:
      return TARGET_PLATFORM::XeHP_SDV;
    case GENX_KBL:
      return TARGET_PLATFORM::GENX_SKL;
    case GENX_GLK:
      return TARGET_PLATFORM::GENX_BXT;
    default:
      return TARGET_PLATFORM::GENX_NONE;
    }
  }

  /// * stackSurface - return a surface that should be used for stack.
  PreDefined_Surface stackSurface() const { return StackSurf; }
};

} // End llvm namespace

#endif
