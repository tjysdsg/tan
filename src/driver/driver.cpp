#include "driver/driver.h"
#include "lexer/lexer.h"
#include "source_file/token.h"
#include "analysis/type_check.h"
#include "analysis/register_declarations.h"
#include "analysis/organize_packages.h"
#include "analysis/type_precheck.h"
#include "codegen/code_generator.h"
#include "common/compilation_unit.h"
#include "include/ast/package.h"
#include "ast/intrinsic.h"
#include "ast/stmt.h"
#include "ast/package.h"
#include "source_file/source_file.h"
#include "parser/parser.h"
#include "llvm_api/clang_frontend.h"
#include "linker/linker.h"
#include "llvm_api/llvm_ar.h"
#include <filesystem>

#include "llvm/ADT/StringRef.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Support/CodeGen.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Option/OptTable.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <llvm/MC/MCAsmBackend.h>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCCodeEmitter.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCObjectWriter.h>
#include <llvm/MC/MCParser/MCAsmParser.h>
#include <llvm/MC/MCParser/MCTargetAsmParser.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCSectionMachO.h>
#include <llvm/MC/MCStreamer.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/StringSaver.h>
#include <llvm/Object/Archive.h>
#include <llvm/Object/IRObjectFile.h>
#include <llvm/Support/ConvertUTF.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/LineIterator.h>
#include <lld/Common/Driver.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Passes/PassBuilder.h>

using namespace tanlang;
namespace fs = std::filesystem;

/// \see https://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/Overall-Options.html
static constexpr std::array CXX_EXTS{".cpp", ".CPP", ".cxx", ".c",  ".cc",  ".C",   ".c++", ".cp",  ".i",  ".ii",
                                     ".h",   ".hh",  ".H",   ".hp", ".hxx", ".hpp", ".HPP", ".h++", ".tcc"};
static constexpr str_view TAN_EXT = ".tan";

void verify_dirs(const vector<str> &dirs);

/**
 * \brief Compile CXX files using clang frontend and return a list of object files
 */
vector<str> compile_cxx(const vector<str> &files, TanCompilation config);

static str search_library(const vector<str> &lib_dirs, const str &lib_name);

CompilerDriver::~CompilerDriver() { singleton = nullptr; }

CompilerDriver::CompilerDriver(TanCompilation config) {
  // Verify config
  verify_dirs(config.lib_dirs);
  verify_dirs(config.import_dirs);
  _config = config;

  // Register import dirs
  size_t n_import = _config.import_dirs.size();
  CompilerDriver::import_dirs.reserve(n_import);
  CompilerDriver::import_dirs.insert(CompilerDriver::import_dirs.begin(), _config.import_dirs.begin(),
                                     _config.import_dirs.end());

  // Initialize LLVM
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
  auto target_triple = llvm::sys::getDefaultTargetTriple();
  str error;
  auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
  if (!target) {
    Error err(error);
    err.raise();
  }

  auto CPU = "generic";
  auto features = "";
  llvm::TargetOptions opt;
  /// relocation model
  auto RM = llvm::Reloc::Model::PIC_;
  _target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);

  singleton = this;
}

void CompilerDriver::run(const vector<str> &files) {
  // Check if files exist
  // and separate cxx and tan source files based on their file extensions
  vector<str> tan_files{};
  vector<str> cxx_files{};
  for (size_t i = 0; i < files.size(); ++i) {
    fs::path f = fs::path(files[i]);
    str ext = f.extension().string();

    if (!fs::exists(f))
      Error(ErrorType::FILE_NOT_FOUND, fmt::format("File not found: {}", files[i])).raise();

    bool is_cxx = std::any_of(CXX_EXTS.begin(), CXX_EXTS.end(), [=](const str &e) { return e == ext; });
    if (is_cxx) {
      cxx_files.push_back(files[i]);
    } else if (ext == TAN_EXT) {
      tan_files.push_back(files[i]);
    } else {
      Error(ErrorType::GENERIC_ERROR, fmt::format("Unrecognized source file: {}", files[i])).raise();
    }
  }

  // Compiling
  auto cxx_objs = compile_cxx(cxx_files, _config);
  auto tan_objs = compile_tan(tan_files);

  // Linking
  vector<str> obj_files(cxx_objs.size() + tan_objs.size());
  size_t i = 0;
  for (const str &o : cxx_objs)
    obj_files[i++] = o;
  for (const str &o : tan_objs)
    obj_files[i++] = o;

  link(obj_files);
}

Package *CompilerDriver::get_package(const str &name) {
  auto q = _packages.find(name);
  if (q != _packages.end()) {
    return q->second;
  }
  return nullptr;
}

void CompilerDriver::register_package(const str &name, Package *package) { _packages[name] = package; }

vector<str> CompilerDriver::compile_tan(const vector<str> &files) {
  bool print_ir_code = _config.verbose >= 1;
  size_t n_files = files.size();
  vector<str> ret(n_files);

  // Parse
  auto cu = parse(files);

  // (Optional): Print AST tree
  if (_config.verbose >= 2) {
    for (auto *c : cu) {
      std::cout << fmt::format("AST Tree of {}:\n{}", c->filename(), c->ast()->repr());
    }
  }

  // Register all declarations in their local contexts
  for (auto *c : cu) {
    RegisterDeclarations rd;
    rd.run(c);
  }

  // Organize input files into modules
  OrganizePackages op;
  vector<Package *> packages = op.run(cu);

  // register all the packages BEFORE running semantic analysis, so that we can search for them during analysis
  for (auto *p : packages) {
    register_package(p->get_name(), p);
  }

  // Semantic analysis
  for (auto *p : packages) {
    TypePrecheck tp;
    tp.run(p);

    TypeCheck analyzer;
    analyzer.run(p);
  }

  // Code generation
  size_t i = 0;
  for (auto *c : cu) {
    std::cout << fmt::format("Compiling TAN file: {}\n", c->filename());

    // IR
    auto *cg = codegen(c, print_ir_code);

    // object file
    str ofile = ret[i] = fs::path(c->filename() + ".o").filename().string();
    emit_object(cg, ofile);

    ++i;

    delete cg;
  }

  for (auto *c : cu) {
    delete c;
  }
  return ret;
}

vector<str> compile_cxx(const vector<str> &files, TanCompilation config) {
  vector<str> obj_files{};

  if (!files.empty()) {
    std::cout << "Compiling " << files.size() << " CXX file(s): ";
    std::for_each(files.begin(), files.end(), [=](auto f) { std::cout << f << " "; });
    std::cout << "\n";

    auto err_code = clang_compile(files, &config);
    if (err_code)
      Error(ErrorType::GENERIC_ERROR, "Failed to compile CXX files").raise();

    // object file paths
    size_t n = files.size();
    obj_files.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      auto p = fs::path(str(files[i])).replace_extension(".o").filename();
      obj_files.push_back(p.string());
    }
  }

  return obj_files;
}

void CompilerDriver::emit_object(CodeGenerator *cg, const str &out_file) { cg->emit_to_file(out_file); }

static umap<TanOptLevel, llvm::CodeGenOpt::Level> tan_to_llvm_opt_level{
    {O0, llvm::CodeGenOpt::None      },
    {O1, llvm::CodeGenOpt::Less      },
    {O2, llvm::CodeGenOpt::Default   },
    {O3, llvm::CodeGenOpt::Aggressive},
};

CodeGenerator *CompilerDriver::codegen(CompilationUnit *cu, bool print_ir) {
  _target_machine->setOptLevel(tan_to_llvm_opt_level[_config.opt_level]);
  auto *cg = new CodeGenerator(_target_machine);

  cg->run(cu);

  if (print_ir)
    cg->dump_ir();

  return cg;
}

vector<CompilationUnit *> CompilerDriver::parse(const vector<str> &files) {
  vector<CompilationUnit *> cu{};

  for (const str &file : files) {
    SourceFile *source = new SourceFile();
    source->open(file);

    // tokenization
    auto tokens = tokenize(source);

    auto *sm = new TokenizedSourceFile(file, tokens);
    auto *parser = new Parser(sm);
    auto *ast = parser->parse();

    // register top-level declarations
    // TODO: put intrinsics into a dedicated module
    auto intrinsic_funcs = Intrinsic::GetIntrinsicFunctionDeclarations();
    for (auto *f : intrinsic_funcs) {
      ast->ctx()->set_function_decl(f);
    }

    cu.push_back(new CompilationUnit(source, sm, ast));
  }

  return cu;
}

vector<str> CompilerDriver::resolve_import(const str &callee_path, const str &import_name) {
  vector<str> ret{};
  auto import_path = fs::path(import_name);
  /// search relative to callee's path
  {
    auto p = fs::path(callee_path).parent_path() / import_path;
    p = p.lexically_normal();
    if (fs::exists(p)) {
      ret.push_back(p.string());
    }
  }
  /// search relative to directories in CompilerDriver::import_dirs
  for (const auto &rel : CompilerDriver::import_dirs) {
    auto p = fs::path(rel) / import_path;
    p = p.lexically_normal();
    if (fs::exists(p)) {
      ret.push_back(p.string());
    }
  }
  return ret;
}

void CompilerDriver::link(const std::vector<str> &files) {
  if (_config.type == SLIB) { // static
    // also add files specified by -l option
    vector<str> all_files(files.begin(), files.end());
    for (const auto &lib : _config.link_files) {
      str path = search_library(_config.lib_dirs, lib);

      if (path.empty())
        Error(ErrorType::LINK_ERROR, fmt::format("Unable to find library: {}", lib)).raise();

      all_files.push_back(path);
    }

    llvm_ar_create_static_lib(_config.out_file, all_files);
    return;
  }

  // shared, obj, or exe
  using tanlang::Linker;
  Linker linker;
  linker.add_files(files);
  linker.add_flag("-o" + str(_config.out_file));
  if (_config.type == EXE) {
    linker.add_flags({"-fPIE"});
  } else if (_config.type == DLIB) {
    linker.add_flags({"-shared"});
  }

  // -L
  size_t n_lib_dirs = _config.lib_dirs.size();
  for (size_t i = 0; i < n_lib_dirs; ++i) {
    auto p = fs::absolute(fs::path(_config.lib_dirs[i]));
    linker.add_flag("-L" + p.string());
    linker.add_flag("-Wl,-rpath," + p.string());
  }

  // -l
  size_t n_link_files = _config.link_files.size();
  for (size_t i = 0; i < n_link_files; ++i) {
    linker.add_flag("-l" + std::string(_config.link_files[i]));
  }
  linker.add_flag(opt_level_to_string(_config.opt_level));

  if (!linker.link())
    Error(ErrorType::LINK_ERROR, "Failed linking").raise();
}

/**
 * \section Helpers
 */

void verify_dirs(const vector<str> &dirs) {
  for (size_t i = 0; i < dirs.size(); ++i) {
    fs::path p = fs::path(dirs[i]);

    if (!fs::exists(p))
      Error(ErrorType::FILE_NOT_FOUND, fmt::format("File not found: {}", dirs[i])).raise();

    if (!fs::is_directory(p))
      Error(ErrorType::FILE_NOT_FOUND, fmt::format("Not a directory: {}", dirs[i])).raise();
  }
}

str search_library(const std::vector<str> &lib_dirs, const str &lib_name) {
  // TODO: platform specific extensions
  for (const str &dir : lib_dirs) {
    vector<fs::path> candidates = {
        /// possible filenames
        fs::path(dir) / fs::path(lib_name),                 //
        fs::path(dir) / fs::path(lib_name + ".a"),          //
        fs::path(dir) / fs::path(lib_name + ".so"),         //
        fs::path(dir) / fs::path("lib" + lib_name + ".a"),  //
        fs::path(dir) / fs::path("lib" + lib_name + ".so"), //
    };

    for (const auto &p : candidates) {
      if (fs::exists(p)) {
        return p.string();
      }
    }
  }

  return "";
}
