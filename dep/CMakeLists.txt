set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})
find_package(
        LLVM 10 REQUIRED
        PATHS
        ${CUSTOM_LLVM_CMAKE_DIR}
)

add_definitions(-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

set(LLVM_LIBS
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

find_library(CLANG_LIBRARIES
        NAMES
        clang-cpp-10.0
        clang-cpp100
        clang-cpp
        PATHS
        ${CLANG_LIBDIRS}
        /usr/lib/llvm-10/lib
        /usr/local/llvm100/lib
        /usr/local/llvm10/lib
        )

if (NOT CLANG_LIBRARIES)
    set(CLANG_LIBRARIES "")
    macro(FIND_AND_ADD_CLANG_LIB _libname_)
        string(TOUPPER ${_libname_} _prettylibname_)
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
            set(CLANG_LIBRARIES ${CLANG_LIBRARIES} ${CLANG_${_prettylibname_}_LIB})
        endif ()
    endmacro(FIND_AND_ADD_CLANG_LIB)

    FIND_AND_ADD_CLANG_LIB(clang)

    FIND_AND_ADD_CLANG_LIB(clangARCMigrate)
    FIND_AND_ADD_CLANG_LIB(clangAST)
    FIND_AND_ADD_CLANG_LIB(clangASTMatchers)
    FIND_AND_ADD_CLANG_LIB(clangAnalysis)
    FIND_AND_ADD_CLANG_LIB(clangApplyReplacements)

    FIND_AND_ADD_CLANG_LIB(clangBasic)

    FIND_AND_ADD_CLANG_LIB(clangCodeGen)
    FIND_AND_ADD_CLANG_LIB(clangChangeNamespace)
    FIND_AND_ADD_CLANG_LIB(clangCrossTU)
    FIND_AND_ADD_CLANG_LIB(clangDaemon)
    FIND_AND_ADD_CLANG_LIB(clangDaemonTweaks)
    FIND_AND_ADD_CLANG_LIB(clangDriver)
    FIND_AND_ADD_CLANG_LIB(clangDependencyScanning)
    FIND_AND_ADD_CLANG_LIB(clangDirectoryWatcher)
    FIND_AND_ADD_CLANG_LIB(clangDriver)
    FIND_AND_ADD_CLANG_LIB(clangDynamicASTMatchers)

    FIND_AND_ADD_CLANG_LIB(clangEdit)

    FIND_AND_ADD_CLANG_LIB(clangFormat)
    FIND_AND_ADD_CLANG_LIB(clangFrontend)
    FIND_AND_ADD_CLANG_LIB(clangFrontendTool)

    FIND_AND_ADD_CLANG_LIB(clangHandleCXX)
    FIND_AND_ADD_CLANG_LIB(clangHandleLLVM)

    FIND_AND_ADD_CLANG_LIB(clangIncludeFixer)
    FIND_AND_ADD_CLANG_LIB(clangIncludeFixerPlugin)
    FIND_AND_ADD_CLANG_LIB(clangIndex)

    FIND_AND_ADD_CLANG_LIB(clangLex)

    FIND_AND_ADD_CLANG_LIB(clangMove)

    FIND_AND_ADD_CLANG_LIB(clangParse)

    FIND_AND_ADD_CLANG_LIB(clangQuery)

    FIND_AND_ADD_CLANG_LIB(clangReorderFields)
    FIND_AND_ADD_CLANG_LIB(clangRewrite)
    FIND_AND_ADD_CLANG_LIB(clangRewriteFrontend)

    FIND_AND_ADD_CLANG_LIB(clangSema)
    FIND_AND_ADD_CLANG_LIB(clangSerialization)

    FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCheckers)
    FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCore)
    FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerFrontend)

    FIND_AND_ADD_CLANG_LIB(clangTooling)
    FIND_AND_ADD_CLANG_LIB(clangToolingASTDiff)
    FIND_AND_ADD_CLANG_LIB(clangToolingCore)
    FIND_AND_ADD_CLANG_LIB(clangToolingInclusions)
    FIND_AND_ADD_CLANG_LIB(clangToolingRefactoring)
    FIND_AND_ADD_CLANG_LIB(clangToolingSyntax)
    FIND_AND_ADD_CLANG_LIB(clangTransformer)

    FIND_AND_ADD_CLANG_LIB(Polly)
    FIND_AND_ADD_CLANG_LIB(PollyISL)
    FIND_AND_ADD_CLANG_LIB(PollyPPCG)

    FIND_AND_ADD_CLANG_LIB(findAllSymbols)
    FIND_AND_ADD_CLANG_LIB(Remarks)
endif ()

message(STATUS "Clang libraries to link against: ${CLANG_LIBRARIES}")

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
            ${CLANG_LIBRARIES}
            PARENT_SCOPE)
else ()
    set(DEP_LIBS
            ${LLVM_LIBS}
            ${CLANG_LIBRARIES}
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
