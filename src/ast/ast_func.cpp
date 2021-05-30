#include "src/ast/ast_func.h"
#include "src/ast/ast_type.h"
#include "src/analysis/type_system.h"
#include "parser.h"
#include "reader.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

ASTFunctionPtr ASTFunction::GetCallee(CompilerSession *cs, const str &name, const vector<ASTBasePtr> &args) {
  ASTFunctionPtr ret = nullptr;
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
      auto arg = f->get_arg(i);
      TAN_ASSERT(arg->_is_typed);
      auto actual_arg = args[i];
      /// allow implicit cast from actual_arg to arg, but not in reverse
      auto t1 = arg->_type;
      auto t2 = actual_arg->_type;
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

ASTFunctionPtr ASTFunction::CreateExtern(const str &name, vector<ASTTypePtr> types) {
  TAN_ASSERT(!types.empty());
  auto ret = make_ptr<ASTFunction>();
  ret->set_data(name);
  ret->get_children().reserve(types.size() + 1);
  ret->get_children().push_back(types[0]);
  if (types.size() > 1) {
    ret->get_children().insert(ret->get_children().end(), types.begin() + 1, types.end());
  }
  ret->_is_external = true;
  ret->_is_public = false;
  ret->set_data(name);
  return ret;
}

ASTTypePtr ASTFunction::get_ret_ty() const {
  return get_child_at<ASTType>(0);
}

ASTNodePtr ASTFunction::get_arg(size_t i) const {
  return get_child_at<ASTNode>(i + 1);
}

size_t ASTFunction::get_n_args() const {
  return get_children().size() - 1 - !_is_external;
}
