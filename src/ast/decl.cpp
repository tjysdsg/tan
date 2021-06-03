#include "src/ast/decl.h"
#include "src/ast/expr.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

/// \section Decl

Decl::Decl(ASTNodeType type, int lbp) : Expr(type, lbp) {}

/// \section ArgDecl

ArgDecl::ArgDecl() : Decl(ASTNodeType::ARG_DECL, 0) {}

ptr<ArgDecl> ArgDecl::Create() { return make_ptr<ArgDecl>(); }

ptr<ArgDecl> ArgDecl::Create(const str &name, const ASTTypePtr &ty) {
  auto ret = make_ptr<ArgDecl>();
  ret->set_name(name);
  ret->set_type(ty);
  return ret;
}

/// \section VarDecl

VarDecl::VarDecl() : Decl(ASTNodeType::VAR_DECL, 0) {}

ptr<VarDecl> VarDecl::Create() { return make_ptr<VarDecl>(); }

ptr<VarDecl> VarDecl::Create(const str &name, const ASTTypePtr &ty) {
  auto ret = make_ptr<VarDecl>();
  ret->set_name(name);
  ret->set_type(ty);
  return ret;
}

/// \section FunctionDecl

FunctionDecl::FunctionDecl() : Decl(ASTNodeType::FUNC_DECL, 0) {}

// TODO: move this to analysis
FunctionDeclPtr FunctionDecl::GetCallee(CompilerSession *cs, const str &name, const vector<ptr<Expr>> &args) {
  FunctionDeclPtr ret = nullptr;
  auto func_candidates = cs->get_functions(name);
  /// always prefer the function with lowest cost if multiple candidates are callable
  /// one implicit cast -> +1 cost
  /// however, if two (or more) functions have the same score, an error is raise (ambiguous call)
  auto cost = (size_t) -1;
  for (const auto &f : func_candidates) {
    size_t n = f->get_n_args();
    if (n != args.size()) { continue; }
    bool good = true;
    size_t c = 0;
    for (size_t i = 0; i < n; ++i) { /// check every argument (return type not checked)
      auto actual_arg = args[i];
      /// allow implicit cast from actual_arg to arg, but not in reverse
      auto t1 = f->get_arg_type(i);
      auto t2 = actual_arg->get_type();
      if (*t1 != *t2) {
        if (0 != TypeSystem::CanImplicitCast(cs, t1, t2)) {
          good = false;
          break;
        }
        ++c;
      }
    }
    if (good) {
      if (c < cost) {
        ret = f;
        cost = c;
      } else if (c == cost) {
        throw std::runtime_error("Ambiguous function call: " + name);
        // FIXME: report_error(cs, p, "Ambiguous function call: " + name);
      }
    }
  }
  if (!ret) {
    throw std::runtime_error("Unknown function call: " + name);
    // FIXME: report_error(cs, p, "Unknown function call: " + name);
  }
  return ret;
}

FunctionDeclPtr FunctionDecl::Create() { return make_ptr<FunctionDecl>(); }

FunctionDeclPtr FunctionDecl::Create(const str &name,
    const ASTTypePtr &ret_type,
    vector<ASTTypePtr> arg_types,
    bool is_external,
    bool is_public) {
  TAN_ASSERT(!arg_types.empty());
  auto ret = make_ptr<FunctionDecl>();

  /// name
  ret->set_name(name);

  /// return type
  ret->_ret_type = ret_type;

  /// args
  ret->_arg_types.reserve(arg_types.size());
  if (arg_types.size() > 1) {
    ret->_arg_types.insert(ret->_arg_types.end(), arg_types.begin() + 1, arg_types.end());
  }

  /// flags
  ret->_is_external = is_external;
  ret->_is_public = is_public;
  return ret;
}

ASTTypePtr FunctionDecl::get_ret_ty() const { return _ret_type; }

str FunctionDecl::get_arg_name(size_t i) const { return _arg_names[i]; }

ASTTypePtr FunctionDecl::get_arg_type(size_t i) const { return _arg_types[i]; }

size_t FunctionDecl::get_n_args() const { return _arg_names.size(); }

void FunctionDecl::set_body(StmtPtr body) { _body = body; }

void FunctionDecl::set_ret_type(ASTTypePtr type) { _ret_type = type; }

void FunctionDecl::set_arg_names(const vector<str> &names) { _arg_names = names; }

void FunctionDecl::set_arg_types(const vector<ASTTypePtr> &types) { _arg_types = types; }

bool FunctionDecl::is_public() const { return _is_public; }

bool FunctionDecl::is_external() const { return _is_external; }

StmtPtr FunctionDecl::get_body() const { return _body; }

void FunctionDecl::set_external(bool is_external) { _is_external = is_external; }

void FunctionDecl::set_public(bool is_public) { _is_public = is_public; }

const vector<ptr<ArgDecl>> &FunctionDecl::get_arg_decls() const { return _arg_decls; }

void FunctionDecl::set_arg_decls(const vector<ptr<ArgDecl>> &arg_decls) { _arg_decls = arg_decls; }

/// \section StructDecl

StructDecl::StructDecl() : Decl(ASTNodeType::STRUCT_DECL, 0) {}

ptr<StructDecl> StructDecl::Create() { return make_ptr<StructDecl>(); }

void StructDecl::set_is_forward_decl(bool is_forward_decl) { _is_forward_decl = is_forward_decl; }

bool StructDecl::is_forward_decl() const { return _is_forward_decl; }

const vector<ExprPtr> &StructDecl::get_member_decls() const { return _member_decls; }

void StructDecl::set_member_decls(const vector<ExprPtr> &member_decls) { _member_decls = member_decls; }
