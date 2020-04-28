#include "src/ast/ast_import.h"
#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "libtanc.h"
#include "compiler.h"
#include "parser.h"
#include "base.h"

namespace tanlang {

ASTImport::ASTImport(Token *token, size_t token_index) : ASTNode(ASTType::IMPORT, 0, 0, token, token_index) {}

std::string ASTImport::to_string(bool print_prefix) const {
  std::string ret = "";
  if (print_prefix) {
    ret += ASTNode::to_string(print_prefix) + " ";
  }
  ret += _file;
  return ret;
}

Value *ASTImport::codegen(CompilerSession *cm) {
  cm->set_current_debug_location(_token->l, _token->c);
  // TODO: import path resolve system
  // TODO: path containing non-ASCII characters?
  auto import_path = fs::path(_file);
  import_path = fs::relative(import_path);
  auto path = import_path.string();
  auto funcs = CompilerSession::get_public_functions(import_path.string());
  if (funcs.empty()) { Compiler::ParseFile(path); }
  funcs = CompilerSession::get_public_functions(import_path.string());
  for (auto &n: funcs) {
    auto f = ast_cast<ASTFunction>(n);
    assert(f);
    /// do nothing for already defined intrinsics
    auto *func = cm->get_module()->getFunction(f->get_name());
    if (!func) { f->codegen_prototype(cm); } else { f->set_func(func); }
    cm->add_function(f);
  }
  return nullptr;
}

size_t ASTImport::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "import"
  auto rhs = parser->peek(_end_index);
  if (rhs->_type != ASTType::STRING_LITERAL) {
    report_code_error(_token, "Invalid import statement");
  }
  _end_index = rhs->parse(parser);
  _file = ast_cast<ASTStringLiteral>(rhs)->get_string();
  return _end_index;
}

} // namespace tanlang
