set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})
if (${LLVM_ROOT_DIR})
    message(STATUS "Using custom LLVM at ${LLVM_ROOT_DIR}")
endif ()
find_package(LLVM 10 REQUIRED
        PATHS ${LLVM_ROOT_DIR})

set(LLVM_LIBS
        # LLVM
        LLVMXRay
        LLVMWindowsManifest
        LLVMTextAPI
        LLVMTableGen
        LLVMSymbolize
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
        LLVMCFGuard
        # LLVMXCoreDisassembler
        # LLVMXCoreCodeGen
        # LLVMXCoreDesc
        # LLVMXCoreInfo
        LLVMX86Disassembler
        LLVMX86AsmParser
        LLVMX86CodeGen
        LLVMX86Desc
        LLVMX86Utils
        LLVMX86Info
        # LLVMWebAssemblyDisassembler
        # LLVMWebAssemblyCodeGen
        # LLVMWebAssemblyDesc
        # LLVMWebAssemblyAsmParser
        # LLVMWebAssemblyInfo
        # LLVMSystemZDisassembler
        # LLVMSystemZCodeGen
        # LLVMSystemZAsmParser
        # LLVMSystemZDesc
        # LLVMSystemZInfo
        # LLVMSparcDisassembler
        # LLVMSparcCodeGen
        # LLVMSparcAsmParser
        # LLVMSparcDesc
        # LLVMSparcInfo
        # LLVMRISCVDisassembler
        # LLVMRISCVCodeGen
        # LLVMRISCVAsmParser
        # LLVMRISCVDesc
        # LLVMRISCVUtils
        # LLVMRISCVInfo
        # LLVMPowerPCDisassembler
        # LLVMPowerPCCodeGen
        # LLVMPowerPCAsmParser
        # LLVMPowerPCDesc
        # LLVMPowerPCInfo
        # LLVMNVPTXCodeGen
        # LLVMNVPTXDesc
        # LLVMNVPTXInfo
        # LLVMMSP430Disassembler
        # LLVMMSP430CodeGen
        # LLVMMSP430AsmParser
        # LLVMMSP430Desc
        # LLVMMSP430Info
        # LLVMMipsDisassembler
        # LLVMMipsCodeGen
        # LLVMMipsAsmParser
        # LLVMMipsDesc
        # LLVMMipsInfo
        # LLVMLanaiDisassembler
        # LLVMLanaiCodeGen
        # LLVMLanaiAsmParser
        # LLVMLanaiDesc
        # LLVMLanaiInfo
        # LLVMHexagonDisassembler
        # LLVMHexagonCodeGen
        # LLVMHexagonAsmParser
        # LLVMHexagonDesc
        # LLVMHexagonInfo
        # LLVMBPFDisassembler
        # LLVMBPFCodeGen
        # LLVMBPFAsmParser
        # LLVMBPFDesc
        # LLVMBPFInfo
        # LLVMARMDisassembler
        # LLVMARMCodeGen
        # LLVMARMAsmParser
        # LLVMARMDesc
        # LLVMARMUtils
        # LLVMARMInfo
        # LLVMAMDGPUDisassembler
        # LLVMAMDGPUCodeGen
        # LLVMAMDGPUAsmParser
        # LLVMAMDGPUDesc
        # LLVMAMDGPUUtils
        # LLVMAMDGPUInfo
        # LLVMAArch64Disassembler
        # LLVMAArch64CodeGen
        # LLVMAArch64AsmParser
        # LLVMAArch64Desc
        # LLVMAArch64Utils
        # LLVMAArch64Info
        LLVMMIRParser
        LLVMipo
        LLVMInstrumentation
        LLVMVectorize
        LLVMLinker
        LLVMIRReader
        LLVMAsmParser
        LLVMMCDisassembler
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
        LLVMMCParser
        LLVMMC
        LLVMDebugInfoCodeView
        LLVMDebugInfoMSF
        LLVMBinaryFormat
        LLVMSupport
        LLVMDemangle
        )

if (${CLANG_ROOT_DIR})
    message(STATUS "Using custom LLVM at ${CLANG_ROOT_DIR}")
endif ()
find_package(Clang REQUIRED
        PATHS ${CLANG_ROOT_DIR})

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
                ${CLANG_ROOT_DIR}/lib
                )
        if (CLANG_${_prettylibname_}_LIB)
            set(CLANG_LIBRARIES ${CLANG_LIBRARIES} ${CLANG_${_prettylibname_}_LIB})
        endif ()
    endmacro(FIND_AND_ADD_CLANG_LIB)

    FIND_AND_ADD_CLANG_LIB(clang)
    FIND_AND_ADD_CLANG_LIB(clangBasic)
    FIND_AND_ADD_CLANG_LIB(clangCodeGen)
    FIND_AND_ADD_CLANG_LIB(clangDriver)
    FIND_AND_ADD_CLANG_LIB(clangTooling)
    FIND_AND_ADD_CLANG_LIB(clangFrontend)
    FIND_AND_ADD_CLANG_LIB(clangFrontendTool)
    FIND_AND_ADD_CLANG_LIB(clangSerialization)
    FIND_AND_ADD_CLANG_LIB(clangFrontendTool)
    FIND_AND_ADD_CLANG_LIB(clangCodeGen)
    FIND_AND_ADD_CLANG_LIB(clangFrontend)
    FIND_AND_ADD_CLANG_LIB(clangDriver)
    FIND_AND_ADD_CLANG_LIB(clangSerialization)
    FIND_AND_ADD_CLANG_LIB(clangSema)
    FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerFrontend)
    FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCheckers)
    FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCore)
    FIND_AND_ADD_CLANG_LIB(clangAnalysis)
    FIND_AND_ADD_CLANG_LIB(clangASTMatchers)
    FIND_AND_ADD_CLANG_LIB(clangAST)
    FIND_AND_ADD_CLANG_LIB(clangParse)
    FIND_AND_ADD_CLANG_LIB(clangSema)
    FIND_AND_ADD_CLANG_LIB(clangBasic)
    FIND_AND_ADD_CLANG_LIB(clangEdit)
    FIND_AND_ADD_CLANG_LIB(clangLex)
    FIND_AND_ADD_CLANG_LIB(clangARCMigrate)
    FIND_AND_ADD_CLANG_LIB(clangRewriteFrontend)
    FIND_AND_ADD_CLANG_LIB(clangRewrite)
    FIND_AND_ADD_CLANG_LIB(clangCrossTU)
    FIND_AND_ADD_CLANG_LIB(clangIndex)
    FIND_AND_ADD_CLANG_LIB(Polly)
    FIND_AND_ADD_CLANG_LIB(PollyISL)
    FIND_AND_ADD_CLANG_LIB(PollyPPCG)
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

message(STATUS "LLVM headers location: ${LLVM_INCLUDE_DIRS}")

# ================== other deps ==================
add_subdirectory(fmt)

# build libbacktrace
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
# ================================================

set(DEP_INCLUDES
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${DEP_DIR}/filesystem/include
        ${DEP_DIR}/fmt/include
        ${DEP_DIR}/libbacktrace
        CACHE INTERNAL "Include dirs of dependencies")

set(DEP_LIBS ${LLVM_LIBS}
        ${CLANG_LIBRARIES}
        fmt::fmt)
if (WIN32)
    set(DEP_LIBS ${DEP_LIBS}
            Version
            CACHE INTERNAL "Dependency libs")
else ()
    set(DEP_LIBS ${DEP_LIBS}
            z dl rt tinfo pthread m xml2
            gcov
            backtrace
            CACHE INTERNAL "Dependency libs")
endif ()

set(DEP_LIB_DIRS
        ${LLVM_LIBRARY_DIR}
        ${CLANG_LIBDIRS}
        PARENT_SCOPE)
