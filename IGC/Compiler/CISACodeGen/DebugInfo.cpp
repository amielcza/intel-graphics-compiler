/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "GenCodeGenModule.h"
#include "Probe/Assertion.h"
#include "CLElfLib/ElfReader.h"

#include "Compiler/CISACodeGen/DebugInfo.hpp"
#include "Compiler/CISACodeGen/OpenCLKernelCodeGen.hpp"
#include "DebugInfo/DwarfDebug.hpp"
#include "DebugInfo/VISADebugInfo.hpp"
#include "Compiler/ScalarDebugInfo/VISAScalarModule.hpp"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/ADT/SmallVector.h"

using namespace llvm;
using namespace IGC;
using namespace IGC::IGCMD;
using namespace std;
// ElfReader related typedefs
using namespace CLElfLib;

char DebugInfoPass::ID = 0;
char CatchAllLineNumber::ID = 0;

// Register pass to igc-opt
#define PASS_FLAG1 "igc-debug-finalize"
#define PASS_DESCRIPTION1 "DebugInfo pass, llvmIR part(WAs)"
#define PASS_CFG_ONLY1 false
#define PASS_ANALYSIS1 false

IGC_INITIALIZE_PASS_BEGIN(DebugInfoPass, PASS_FLAG1, PASS_DESCRIPTION1, PASS_CFG_ONLY1, PASS_ANALYSIS1)
IGC_INITIALIZE_PASS_END(DebugInfoPass, PASS_FLAG1, PASS_DESCRIPTION1, PASS_CFG_ONLY1, PASS_ANALYSIS1)

// Used for opt testing, could be removed if KernelShaderMap is moved to ctx.
static CShaderProgram::KernelShaderMap KernelShaderMap;

// Default ctor used for igc-opt testing
DebugInfoPass::DebugInfoPass() :
    ModulePass(ID),
    kernels(KernelShaderMap)
{ }

DebugInfoPass::DebugInfoPass(CShaderProgram::KernelShaderMap& k) :
    ModulePass(ID),
    kernels(k)
{
    initializeMetaDataUtilsWrapperPass(*PassRegistry::getPassRegistry());
}

DebugInfoPass::~DebugInfoPass()
{
}

std::vector<CShader *> DebugInfoPass::findShaderCandidates() {
  auto isCandidate = [](CShaderProgram *shaderProgram, SIMDMode m,
                        ShaderDispatchMode mode =
                            ShaderDispatchMode::NOT_APPLICABLE) {
    auto currShader = shaderProgram->GetShader(m, mode);
    if (!currShader || !currShader->GetDebugInfoData().m_pDebugEmitter)
      return (CShader *)nullptr;

    if (currShader->ProgramOutput()->m_programSize == 0)
      return (CShader *)nullptr;

    return currShader;
  };

  std::vector<CShader *> shaders;
  for (auto &k : kernels) {
    auto shaderProgram = k.second;
    auto simd8 = isCandidate(shaderProgram, SIMDMode::SIMD8);
        auto simd16 = isCandidate(shaderProgram, SIMDMode::SIMD16);
        auto simd32 = isCandidate(shaderProgram, SIMDMode::SIMD32);

        if (simd8)
          shaders.push_back(simd8);
        if (simd16)
          shaders.push_back(simd16);
        if (simd32)
          shaders.push_back(simd32);
    }
    return shaders;
}

void setVISAModuleType(VISAModule *v, const DbgDecoder &decodedDbg) {
  auto firstInst = (v->GetInstInfoMap()->begin())->first;
  auto funcName = firstInst->getParent()->getParent()->getName();

  for (auto &item : decodedDbg.compiledObjs) {
    auto &name = item.kernelName;
    if (funcName.compare(name) == 0) {
      if (item.relocOffset == 0)
        v->SetType(VISAModule::ObjectType::KERNEL);
      else
        v->SetType(VISAModule::ObjectType::STACKCALL_FUNC);
      return;
    }
    for (auto &sub : item.subs) {
      auto &subName = sub.name;
      if (funcName.compare(subName) == 0) {
        v->SetType(VISAModule::ObjectType::SUBROUTINE);
        return;
      }
    }
  }
}

unsigned int getGenOffset(
    const std::vector<std::pair<unsigned int, unsigned int>> &indexMap,
    unsigned int VISAIndex) {
  // indexMap is VISA index <-> gen offset mapping.
  unsigned retval = 0;
  for (auto &item : indexMap) {
    if (item.first == VISAIndex) {
      retval = item.second;
    }
  }
  return retval;
}

unsigned int getLastGenOffset(IGC::VISAModule *v,
                              const DbgDecoder &decodedDbg,
                              const DenseMap<uint32_t, unsigned int> &genOffsetToLastVISAindex) {

  // Iterate through dbg info objects to find the one which matches the given VISAModule.
  for (auto &item : decodedDbg.compiledObjs) {
    // Name of the kernel or stack call function.
    auto &objectName = item.kernelName;

    // First instruction that went through the LLVM IR -> VISA translation.
    auto firstInst = (v->GetInstInfoMap()->begin())->first;

    // Parent function of firstInst. It can be kernel, stackcall or subroutine.
    auto funcName = firstInst->getParent()->getParent()->getName();

    if (funcName.compare(objectName) == 0) {
      // firstInst is inside a function having no subroutines -> last VISA is trivial.
      if (item.subs.size() == 0)
        return item.CISAIndexMap.back().second;
      // firstInst is inside a function having subroutines. Use created mapping.
      return getGenOffset(item.CISAIndexMap, genOffsetToLastVISAindex.lookup(item.relocOffset));
    }

    // Check if firstInst is inside a subroutine.
    for (auto &sub : item.subs) {
      auto &subName = sub.name;
      if (funcName.compare(subName) == 0)
        return getGenOffset(item.CISAIndexMap, sub.endVISAIndex);
    }
  }

  return 0;
}

void DebugInfoPass::getSortedVISAModules(
    llvm::SmallVector<VISAIndexToModule, 8> &sortedModules,
                          const DbgDecoder &decodedDbg) {

  // This structure contains mapping:
  // gen offset <-> VISA index of the last instruction
  // for every compile object (kernel or stack call function) in the shader.
  //
  // Last VISA id is the smallest from: last VISA id from object's index map
  // and start VISA indexes of object's subroutines - 1.
  // We use the fact that subroutines are always placed after kernel / stack
  // call function.
  llvm::DenseMap<uint32_t, unsigned int> genOffsetToLastVISAindex;

  for (auto &item : decodedDbg.compiledObjs) {
    genOffsetToLastVISAindex[item.relocOffset] = item.CISAIndexMap.back().first;
    for (auto &subroutine : item.subs) {
      auto subStartVISAIndex = subroutine.startVISAIndex;
      if (genOffsetToLastVISAindex[item.relocOffset] > subStartVISAIndex)
        genOffsetToLastVISAindex[item.relocOffset] = subStartVISAIndex - 1;
    }
  }

  for (auto &m : m_currShader->GetDebugInfoData().m_VISAModules) {
    // Deduce and set correct module type: KERNEL, STACKCALL_FUNC or
    // SUBROUTINE.
    setVISAModuleType(m.second, decodedDbg);

    // getLastGenOffset returns zero if debug info for given function
    // was not found, skip the function in such case. This can happen,
    // when the function was optimized away but the definition is still
    // present inside the module.
    // Otherwise, it returns offset of the last instruction of the function.

    unsigned int lastGenOffset =
        getLastGenOffset(m.second, decodedDbg, genOffsetToLastVISAindex);
    if (lastGenOffset == 0)
      continue;
    sortedModules.push_back(std::make_pair(lastGenOffset, m.second));
  }

  // We sort VISA modules by their gen offset.
  std::sort(sortedModules.begin(), sortedModules.end(),
            [](VISAIndexToModule &p1, VISAIndexToModule &p2) {
              return p1.first < p2.first;
            });
}

bool DebugInfoPass::runOnModule(llvm::Module &M) {
  // This loop is just a workaround till we add support for DIArgList metadata.
  // If we implement DIArgList support, it should be deleted.
#if LLVM_VERSION_MAJOR > 12
  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *dbgInst = dyn_cast<DbgVariableIntrinsic>(&I)) {
          if (dbgInst->getNumVariableLocationOps() > 1) {
            dbgInst->setUndef();
          }
        }
      }
    }
  }
#endif
  // Early out
  if (kernels.empty())
    return false;

  std::vector<CShader *> shaders = findShaderCandidates();

  DwarfDISubprogramCache DISPCache;

  for (auto &currShader : shaders) {
    // Look for the right CShaderProgram instance
    m_currShader = currShader;

    MetaDataUtils *pMdUtils = m_currShader->GetMetaDataUtils();
    if (!isEntryFunc(pMdUtils, m_currShader->entry))
      continue;

    m_pDebugEmitter = m_currShader->GetDebugInfoData().m_pDebugEmitter;

    IGC::VISADebugInfo VisaDbgInfo(
        m_currShader->ProgramOutput()->m_debugDataGenISA);
    const DbgDecoder &decodedDbg = VisaDbgInfo.getRawDecodedData();

    // This vector contains VISAModules sorted by their last gen offset.
    // It means they are sorted in order of their placement in binary.
    llvm::SmallVector<VISAIndexToModule, 8> sortedVISAModules;
    getSortedVISAModules(sortedVISAModules, decodedDbg);

    m_pDebugEmitter->SetDISPCache(&DISPCache);
    for (auto &m : sortedVISAModules) {
      m_pDebugEmitter->registerVISA(m.second);
    }

    unsigned int size = sortedVISAModules.size();
    bool finalize = false;
    for (auto &m : sortedVISAModules) {
      if (--size == 0)
        finalize = true;
      m_pDebugEmitter->setCurrentVISA(m.second);
      emitDebugInfo(finalize, VisaDbgInfo);
    }

    // set VISA dbg info to nullptr to indicate 1-step debug is enabled
    if (currShader->ProgramOutput()->m_debugDataGenISA) {
      IGC::aligned_free(currShader->ProgramOutput()->m_debugDataGenISA);
    }
    currShader->ProgramOutput()->m_debugDataGenISASize = 0;
    currShader->ProgramOutput()->m_debugDataGenISA = nullptr;

    m_currShader->GetContext()->metrics.CollectDataFromDebugInfo(
        m_currShader->entry, &m_currShader->GetDebugInfoData(), &VisaDbgInfo);

    IDebugEmitter::Release(m_pDebugEmitter);
  }

  return false;
}

static void debugDump(const CShader *Shader, llvm::StringRef Ext,
                      ArrayRef<char> Blob) {
  if (Blob.empty())
    return;

  auto ExtStr = Ext.str();
  std::string DumpName = IGC::Debug::GetDumpName(Shader, ExtStr.c_str());
  FILE *const DumpFile = fopen(DumpName.c_str(), "wb+");
  if (nullptr == DumpFile)
    return;

  fwrite(Blob.data(), Blob.size(), 1, DumpFile);
  fclose(DumpFile);
}

void DebugInfoPass::emitDebugInfo(bool finalize,
                                  const IGC::VISADebugInfo &VisaDbgInfo) {
  IGC_ASSERT(m_pDebugEmitter);

  std::vector<char> buffer = m_pDebugEmitter->Finalize(finalize, VisaDbgInfo);

  if (IGC_IS_FLAG_ENABLED(ShaderDumpEnable) ||
      IGC_IS_FLAG_ENABLED(ElfDumpEnable))
    debugDump(m_currShader, "elf", {buffer.data(), buffer.size()});

  const std::string &DbgErrors = m_pDebugEmitter->getErrors();
  if (IGC_IS_FLAG_ENABLED(ShaderDumpEnable))
    debugDump(m_currShader, "dbgerr", {DbgErrors.data(), DbgErrors.size()});

  void *dbgInfo = IGC::aligned_malloc(buffer.size(), sizeof(void *));
  if (dbgInfo)
    memcpy_s(dbgInfo, buffer.size(), buffer.data(), buffer.size());

  SProgramOutput *pOutput = m_currShader->ProgramOutput();
  pOutput->m_debugData = dbgInfo;
  pOutput->m_debugDataSize = dbgInfo ? buffer.size() : 0;
}

// Register pass to igc-opt
#define PASS_FLAG "igc-catch-all-linenum"
#define PASS_DESCRIPTION "CatchAllLineNumber pass"
#define PASS_CFG_ONLY false
#define PASS_ANALYSIS false
IGC_INITIALIZE_PASS_BEGIN(CatchAllLineNumber, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)
IGC_INITIALIZE_PASS_END(CatchAllLineNumber, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)

CatchAllLineNumber::CatchAllLineNumber() :
    FunctionPass(ID)
{
    initializeMetaDataUtilsWrapperPass(*PassRegistry::getPassRegistry());
}

CatchAllLineNumber::~CatchAllLineNumber()
{
}

bool CatchAllLineNumber::runOnFunction(llvm::Function& F)
{
    // Insert placeholder intrinsic instruction in each kernel/stack call function.
    if (!F.getSubprogram() || F.isDeclaration() ||
      IGC_IS_FLAG_ENABLED(NoCatchAllDebugLine))
        return false;

    if (F.getCallingConv() != llvm::CallingConv::SPIR_KERNEL &&
        !F.hasFnAttribute("visaStackCall"))
        return false;

    llvm::IRBuilder<> Builder(F.getParent()->getContext());
    DIBuilder di(*F.getParent());
    Function* lineNumPlaceholder = GenISAIntrinsic::getDeclaration(F.getParent(), GenISAIntrinsic::ID::GenISA_CatchAllDebugLine);
    auto intCall = Builder.CreateCall(lineNumPlaceholder);

    auto line = F.getSubprogram()->getLine();
    auto scope = F.getSubprogram();

    auto dbg = DILocation::get(F.getParent()->getContext(), line, 0, scope);

    intCall->setDebugLoc(dbg);

    intCall->insertBefore(&*F.getEntryBlock().getFirstInsertionPt());

    return true;
}
