set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})

# ===== llvm =====
find_package(LLVM 16 REQUIRED PATHS ${LLVM_ROOT_DIR})
set(_LLVM_LIBS
        LLVM
        LLVMXRay
        LLVMWindowsManifest
        LLVMWindowsDriver
        LLVMTextAPI
        LLVMSymbolize
        LLVMFrontendOpenMP
        LLVMCFGuard
        LLVMDebugInfoPDB
        LLVMOrcJIT
        LLVMJITLink
        LLVMObjectYAML
        LLVMMCA
        LLVMLTO
        LLVMPasses
        LLVMObjCARCOpts
        LLVMLineEditor
        LLVMLibDriver
        LLVMInterpreter
        LLVMFuzzMutate
        LLVMMCJIT
        LLVMExecutionEngine
        LLVMRuntimeDyld
        LLVMDlltoolDriver
        LLVMOption
        LLVMDebugInfoGSYM
        LLVMCoverage
        LLVMCoroutines
        LLVMXCoreDisassembler
        LLVMXCoreCodeGen
        LLVMXCoreDesc
        LLVMXCoreInfo
        LLVMX86Disassembler
        LLVMX86AsmParser
        LLVMX86CodeGen
        LLVMX86Desc
        LLVMX86Utils
        LLVMX86Info
        LLVMWebAssemblyDisassembler
        LLVMWebAssemblyCodeGen
        LLVMWebAssemblyDesc
        LLVMWebAssemblyAsmParser
        LLVMWebAssemblyInfo
        LLVMWebAssemblyUtils
        LLVMSystemZDisassembler
        LLVMSystemZCodeGen
        LLVMSystemZAsmParser
        LLVMSystemZDesc
        LLVMSystemZInfo
        LLVMSparcDisassembler
        LLVMSparcCodeGen
        LLVMSparcAsmParser
        LLVMSparcDesc
        LLVMSparcInfo
        LLVMRISCVDisassembler
        LLVMRISCVCodeGen
        LLVMRISCVAsmParser
        LLVMRISCVDesc
        LLVMRISCVUtils
        LLVMRISCVInfo
        LLVMPowerPCDisassembler
        LLVMPowerPCCodeGen
        LLVMPowerPCAsmParser
        LLVMPowerPCDesc
        LLVMPowerPCInfo
        LLVMNVPTXCodeGen
        LLVMNVPTXDesc
        LLVMNVPTXInfo
        LLVMMSP430Disassembler
        LLVMMSP430CodeGen
        LLVMMSP430AsmParser
        LLVMMSP430Desc
        LLVMMSP430Info
        LLVMMipsDisassembler
        LLVMMipsCodeGen
        LLVMMipsAsmParser
        LLVMMipsDesc
        LLVMMipsInfo
        LLVMLanaiDisassembler
        LLVMLanaiCodeGen
        LLVMLanaiAsmParser
        LLVMLanaiDesc
        LLVMLanaiInfo
        LLVMHexagonDisassembler
        LLVMHexagonCodeGen
        LLVMHexagonAsmParser
        LLVMHexagonDesc
        LLVMHexagonInfo
        LLVMBPFDisassembler
        LLVMBPFCodeGen
        LLVMBPFAsmParser
        LLVMBPFDesc
        LLVMBPFInfo
        LLVMARMDisassembler
        LLVMARMCodeGen
        LLVMARMAsmParser
        LLVMARMDesc
        LLVMARMUtils
        LLVMARMInfo
        LLVMAMDGPUDisassembler
        LLVMAMDGPUCodeGen
        LLVMMIRParser
        LLVMipo
        LLVMInstrumentation
        LLVMVectorize
        LLVMLinker
        LLVMIRReader
        LLVMAsmParser
        LLVMAMDGPUAsmParser
        LLVMAMDGPUDesc
        LLVMAMDGPUUtils
        LLVMAMDGPUInfo
        LLVMAArch64Disassembler
        LLVMMCDisassembler
        LLVMAArch64CodeGen
        LLVMGlobalISel
        LLVMSelectionDAG
        LLVMAsmPrinter
        LLVMDebugInfoDWARF
        LLVMCodeGen
        LLVMTarget
        LLVMScalarOpts
        LLVMInstCombine
        LLVMAggressiveInstCombine
        LLVMTransformUtils
        LLVMBitWriter
        LLVMAnalysis
        LLVMProfileData
        LLVMObject
        LLVMBitReader
        LLVMBitstreamReader
        LLVMCore
        LLVMRemarks
        LLVMAArch64AsmParser
        LLVMMCParser
        LLVMAArch64Desc
        LLVMMC
        LLVMDebugInfoCodeView
        LLVMDebugInfoMSF
        LLVMBinaryFormat
        LLVMAArch64Utils
        LLVMAArch64Info
        LLVMSupport
        LLVMDemangle
        LLVMVEAsmParser
        LLVMVECodeGen
        LLVMVectorize
        LLVMVEDesc
        LLVMVEDisassembler
        LLVMVEInfo
        LLVMAVRAsmParser
        LLVMAVRCodeGen
        LLVMAVRDesc
        LLVMAVRDisassembler
        LLVMAVRInfo
)
message(STATUS "LLVM headers location: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM library directory: ${LLVM_LIBRARY_DIR}")

macro(FIND_AND_ADD_LLVM_LIB libname libdir)
    find_library(LLVM_${libname}_LIB NAMES ${libname}
            PATHS
            ${libdir}
            NO_DEFAULT_PATH
    )
    if (LLVM_${libname}_LIB)
        set(LLVM_LIBS ${LLVM_LIBS} ${LLVM_${libname}_LIB})
    endif ()
endmacro()
foreach (LIB ${_LLVM_LIBS})
    FIND_AND_ADD_LLVM_LIB(${LIB} ${LLVM_LIBRARY_DIR})
endforeach ()

# ===== clang =====
find_package(Clang 16 REQUIRED PATHS ${CLANG_ROOT_DIR})
message(STATUS "Clang headers location: ${CLANG_INCLUDE_DIRS}")
set(CLANG_LIBRARY_DIR ${CLANG_INSTALL_PREFIX}/lib)
set(_CLANG_LIBRARIES
        clang
        clangBasic
        clangCodeGen
        clangDriver
        clangTooling
        clangToolingASTDiff
        clangToolingCore
        clangToolingInclusions
        clangToolingRefactoring
        clangToolingSyntax
        clangExtractAPI
        clangFrontend
        clangFrontendTool
        clangSerialization
        clangFrontendTool
        clangCodeGen
        clangFrontend
        clangDriver
        clangSerialization
        clangSema
        clangStaticAnalyzerFrontend
        clangStaticAnalyzerCheckers
        clangStaticAnalyzerCore
        clangAnalysis
        clangASTMatchers
        clangAST
        clangParse
        clangSema
        clangBasic
        clangEdit
        clangLex
        clangARCMigrate
        clangRewriteFrontend
        clangRewrite
        clangCrossTU
        clangIndex
        clangSupport
        # Polly
        # PollyISL
        # PollyPPCG
)
foreach (LIB ${_CLANG_LIBRARIES})
    FIND_AND_ADD_LLVM_LIB(${LIB} ${CLANG_LIBRARY_DIR})
endforeach ()

# ===== lld =====
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(_LLD_SEARCH_PATH ${CLANG_INSTALL_PREFIX}/lib/cmake/lld)
find_package(LLD 16 REQUIRED
        PATHS
        ${LLD_ROOT_DIR}
        ${_LLD_SEARCH_PATH} # otherwise won't work on Ubuntu if installed using LLVM's apt repo
)
set(LLD_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS})
set(LLD_LIBRARY_DIR ${LLVM_LIBRARY_DIR})
set(_LLD_LIBS
        lldCOFF
        lldCommon
        lldELF
        lldMachO
        lldMinGW
        lldWasm
)
message(STATUS "LLD headers location: ${LLD_INCLUDE_DIRS}")
message(STATUS "LLD library directory: ${LLD_LIBRARY_DIR}")
foreach (LIB ${_LLD_LIBS})
    FIND_AND_ADD_LLVM_LIB(${LIB} ${LLD_LIBRARY_DIR})
endforeach ()

# ================== other deps ==================
add_subdirectory(fmt)

if (WIN32)
    set(LIBBACKTRACE_LIBS "" CACHE INTERNAL "libbacktrace libraries")
    set(LIBBACKTRACE_INCLUDE_DIRS "" CACHE INTERNAL "libbacktrace include dirs")
    add_custom_target(libbacktrace ALL)
else ()
    set(LIBBACKTRACE_LIBS ${CMAKE_CURRENT_LIST_DIR}/libbacktrace/.libs/libbacktrace.a CACHE INTERNAL "libbacktrace libraries")
    set(LIBBACKTRACE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/libbacktrace CACHE INTERNAL "libbacktrace include dirs")
    add_custom_target(libbacktrace ALL DEPENDS ${LIBBACKTRACE_LIBS})
    add_custom_command(
            OUTPUT ${LIBBACKTRACE_LIBS}
            COMMAND ./configure
            COMMAND make
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libbacktrace
    )
endif ()

if (UNIX)
    set(LIBBACKTRACE_INCLUDE_DIR ${DEP_DIR}/libbacktrace)
else ()
    set(LIBBACKTRACE_INCLUDE_DIR "")
endif ()

find_package(ZLIB REQUIRED)
# ================================================

set(DEP_INCLUDES
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${DEP_DIR}/fmt/include
        CACHE INTERNAL "Include dependencies"
)
message(STATUS "Include dirs of dependencies: ${DEP_INCLUDES}")

set(DEP_LIBS ${LLVM_LIBS} fmt::fmt ${ZLIB_LIBRARIES})
if (WIN32)
    set(DEP_LIBS ${DEP_LIBS}
            Version
            CACHE INTERNAL "Dependency libs")
else ()
    set(DEP_LIBS ${DEP_LIBS}
            dl rt tinfo pthread m xml2
            CACHE INTERNAL "Dependency libs")
endif ()

set(DEP_LIB_DIRS
        ${LLVM_LIBRARY_DIR}
        ${CLANG_LIBRARY_DIR}
        PARENT_SCOPE)
