set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})
find_package(
        LLVM 10 REQUIRED
        PATHS
        ${CUSTOM_LLVM_CMAKE_DIR}
)

add_definitions(-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

set(LLVM_LIBS
        LLVMPolly
        LLVMgold
        Fuzzer
        LLVM
        LLVMAArch64AsmParser
        LLVMAArch64CodeGen
        LLVMAArch64Desc
        LLVMAArch64Disassembler
        LLVMAArch64Info
        LLVMAArch64Utils
        LLVMAMDGPUAsmParser
        LLVMAMDGPUCodeGen
        LLVMAMDGPUDesc
        LLVMAMDGPUDisassembler
        LLVMAMDGPUInfo
        LLVMAMDGPUUtils
        LLVMARMAsmParser
        LLVMARMCodeGen
        LLVMARMDesc
        LLVMARMDisassembler
        LLVMARMInfo
        LLVMARMUtils
        LLVMAVRAsmParser
        LLVMAVRCodeGen
        LLVMAVRDesc
        LLVMAVRDisassembler
        LLVMAVRInfo
        LLVMAggressiveInstCombine
        LLVMAnalysis
        LLVMAsmParser
        LLVMAsmPrinter
        LLVMBPFAsmParser
        LLVMBPFCodeGen
        LLVMBPFDesc
        LLVMBPFDisassembler
        LLVMBPFInfo
        LLVMBinaryFormat
        LLVMBitReader
        LLVMBitWriter
        LLVMBitstreamReader
        LLVMCFGuard
        LLVMCodeGen
        LLVMCore
        LLVMCoroutines
        LLVMCoverage
        LLVMDWARFLinker
        LLVMDebugInfoCodeView
        LLVMDebugInfoDWARF
        LLVMDebugInfoGSYM
        LLVMDebugInfoMSF
        LLVMDebugInfoPDB
        LLVMDemangle
        LLVMDlltoolDriver
        LLVMExecutionEngine
        LLVMExtensions
        LLVMFrontendOpenMP
        LLVMFuzzMutate
        LLVMGlobalISel
        LLVMHexagonAsmParser
        LLVMHexagonCodeGen
        LLVMHexagonDesc
        LLVMHexagonDisassembler
        LLVMHexagonInfo
        LLVMIRReader
        LLVMInstCombine
        LLVMInstrumentation
        LLVMInterpreter
        LLVMJITLink
        LLVMLTO
        LLVMLanaiAsmParser
        LLVMLanaiCodeGen
        LLVMLanaiDesc
        LLVMLanaiDisassembler
        LLVMLanaiInfo
        LLVMLibDriver
        LLVMLineEditor
        LLVMLinker
        LLVMMC
        LLVMMCA
        LLVMMCDisassembler
        LLVMMCJIT
        LLVMMCParser
        LLVMMIRParser
        LLVMMSP430AsmParser
        LLVMMSP430CodeGen
        LLVMMSP430Desc
        LLVMMSP430Disassembler
        LLVMMSP430Info
        LLVMMipsAsmParser
        LLVMMipsCodeGen
        LLVMMipsDesc
        LLVMMipsDisassembler
        LLVMMipsInfo
        LLVMNVPTXCodeGen
        LLVMNVPTXDesc
        LLVMNVPTXInfo
        LLVMObjCARCOpts
        LLVMObject
        LLVMObjectYAML
        LLVMOption
        LLVMOrcError
        LLVMOrcJIT
        LLVMPasses
        LLVMPerfJITEvents
        LLVMPowerPCAsmParser
        LLVMPowerPCCodeGen
        LLVMPowerPCDesc
        LLVMPowerPCDisassembler
        LLVMPowerPCInfo
        LLVMProfileData
        LLVMRISCVAsmParser
        LLVMRISCVCodeGen
        LLVMRISCVDesc
        LLVMRISCVDisassembler
        LLVMRISCVInfo
        LLVMRISCVUtils
        LLVMRemarks
        LLVMRuntimeDyld
        LLVMScalarOpts
        LLVMSelectionDAG
        LLVMSparcAsmParser
        LLVMSparcCodeGen
        LLVMSparcDesc
        LLVMSparcDisassembler
        LLVMSparcInfo
        LLVMSupport
        LLVMSymbolize
        LLVMSystemZAsmParser
        LLVMSystemZCodeGen
        LLVMSystemZDesc
        LLVMSystemZDisassembler
        LLVMSystemZInfo
        LLVMTableGen
        LLVMTarget
        LLVMTextAPI
        LLVMTransformUtils
        LLVMVectorize
        LLVMWebAssemblyAsmParser
        LLVMWebAssemblyCodeGen
        LLVMWebAssemblyDesc
        LLVMWebAssemblyDisassembler
        LLVMWebAssemblyInfo
        LLVMWindowsManifest
        LLVMX86AsmParser
        LLVMX86CodeGen
        LLVMX86Desc
        LLVMX86Disassembler
        LLVMX86Info
        LLVMX86Utils
        LLVMXCoreCodeGen
        LLVMXCoreDesc
        LLVMXCoreDisassembler
        LLVMXCoreInfo
        LLVMXRay
        LLVMipo
        LTO
        Polly
        PollyISL
        PollyPPCG
        Remarks
        archer
        archer_static
        c++
        c++abi
        clang-10
        clang-cpp
        clang
        clang
        clangARCMigrate
        clangAST
        clangASTMatchers
        clangAnalysis
        clangApplyReplacements
        clangBasic
        clangChangeNamespace
        clangCodeGen
        clangCrossTU
        clangDaemon
        clangDaemonTweaks
        clangDependencyScanning
        clangDirectoryWatcher
        clangDoc
        clangDriver
        clangDynamicASTMatchers
        clangEdit
        clangFormat
        clangFrontend
        clangFrontendTool
        clangHandleCXX
        clangHandleLLVM
        clangIncludeFixer
        clangIncludeFixerPlugin
        clangIndex
        clangLex
        clangMove
        clangParse
        clangQuery
        clangReorderFields
        clangRewrite
        clangRewriteFrontend
        clangSema
        clangSerialization
        clangStaticAnalyzerCheckers
        clangStaticAnalyzerCore
        clangStaticAnalyzerFrontend
        clangTidy
        clangTidyAbseilModule
        clangTidyAndroidModule
        clangTidyBoostModule
        clangTidyBugproneModule
        clangTidyCERTModule
        clangTidyCppCoreGuidelinesModule
        clangTidyDarwinModule
        clangTidyFuchsiaModule
        clangTidyGoogleModule
        clangTidyHICPPModule
        clangTidyLLVMModule
        clangTidyLinuxKernelModule
        clangTidyMPIModule
        clangTidyMiscModule
        clangTidyModernizeModule
        clangTidyObjCModule
        clangTidyOpenMPModule
        clangTidyPerformanceModule
        clangTidyPlugin
        clangTidyPortabilityModule
        clangTidyReadabilityModule
        clangTidyUtils
        clangTidyZirconModule
        clangTooling
        clangToolingASTDiff
        clangToolingCore
        clangToolingInclusions
        clangToolingRefactoring
        clangToolingSyntax
        clangTransformer
        findAllSymbols
        gomp
        iomp5
        lldCOFF
        lldCommon
        lldCore
        lldDriver
        lldELF
        lldMachO
        lldMinGW
        lldReaderWriter
        lldWasm
        lldYAML
        lldb
        lldbIntelFeatures
        omp-10
        omp
        omptarget
        )

find_package(
        Clang REQUIRED  # ClangConfig.cmake doesn't seem to specify version
        PATHS
        ${CUSTOM_CLANG_CMAKE_DIR}
)

if (NOT DEFINED CLANG_INCLUDE_DIRS)
    message(FATAL_ERROR "Cannot find clang")
else ()
    message(STATUS "Clang headers found in: ${CLANG_INCLUDE_DIRS}")
endif ()

macro(FIND_AND_ADD_CLANG_LIB _libname_)
    string(TOUPPER ${_libname_} _prettylibname_)
    message(STATUS "Checking LLVM lib: ${_libname_}")
    find_library(CLANG_${_prettylibname_}_LIB NAMES ${_libname_}
            PATHS
            ${CLANG_LIBDIRS}
            /usr/lib/llvm/10/lib
            /usr/lib/llvm-10/lib
            /usr/lib/llvm-10.0/lib
            /usr/local/llvm100/lib
            /usr/local/llvm10/lib
            /mingw64/lib
            /c/msys64/mingw64/lib
            c:\\msys64\\mingw64\\lib
            )
    if (CLANG_${_prettylibname_}_LIB)
        set(LLVM_LIB_FILES ${LLVM_LIB_FILES} ${CLANG_${_prettylibname_}_LIB})
    else ()
        message(WARNING "Checking LLVM lib: ${_libname_} failed")
    endif ()
endmacro(FIND_AND_ADD_CLANG_LIB)

foreach (LIBNAME ${LLVM_LIBS})
    FIND_AND_ADD_CLANG_LIB(${LIBNAME})
endforeach ()

message(STATUS "LLVM libraries to link against: ${LLVM_LIB_FILES}")

# CLION + WSL not handling symlink correctly
set(tmp ${LLVM_INCLUDE_DIRS}/llvm)
set(LLVM_INCLUDE_DIRS "")
foreach (d ${tmp})
    get_filename_component(d ${d} REALPATH)
    get_filename_component(d ${d} DIRECTORY)
    set(LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS} ${d})
endforeach ()

set(DEP_INCLUDES
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${DEP_DIR}/filesystem/include
        ${DEP_DIR}/libbacktrace
        PARENT_SCOPE)

if (MSVC)
    set(DEP_LIBS
            ${LLVM_LIBS}
            ${LLVM_LIB_FILES}
            PARENT_SCOPE)
else ()
    set(DEP_LIBS
            ${LLVM_LIBS}
            ${LLVM_LIB_FILES}
            z dl rt tinfo pthread m xml2
            gcov
            backtrace
            PARENT_SCOPE)
endif ()

set(DEP_LIB_DIRS
        ${LLVM_LIBRARY_DIR}
        ${CLANG_LIBDIRS}
        ${DEP_DIR}/libbacktrace/.libs
        PARENT_SCOPE)

if (NOT LLVM_TOOLS_BINARY_DIR)
    message(FATAL_ERROR "Cannot find LLVM binary dir")
endif ()
set(LLVM_BIN_DIR "${LLVM_TOOLS_BINARY_DIR}" PARENT_SCOPE)
message(DEBUG "LLVM binary dir found at: ${LLVM_BIN_DIR}")
