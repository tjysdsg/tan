#include "src/ast/ast_import.h"
#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "libtanc.h"
#include "compiler.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

Value *ASTImport::_codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  for (auto &n: _imported_functions) {
    auto f = ast_cast<ASTFunction>(n);
    /// do nothing for already defined intrinsics
    auto *func = cs->get_module()->getFunction(f->get_name());
    if (!func) { f->codegen_prototype(cs); } else { f->set_func(func); }
  }
  return nullptr;
}

size_t ASTImport::nud() {
  _end_index = _start_index + 1; /// skip "import"
  auto rhs = _parser->peek(_end_index);
  if (rhs->_type != ASTType::STRING_LITERAL) { report_code_error(_token, "Invalid import statement"); }
  _end_index = rhs->parse(_parser, _cs);
  _file = ast_cast<ASTStringLiteral>(rhs)->get_string();

  // FIXME: path containing non-ASCII characters?
  auto imported = Compiler::resolve_import(_parser->get_filename(), _file);
  if (imported.empty()) { report_code_error(_token, "Cannot import: " + _file); }

  /// it might be already parsed
  _imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  if (_imported_functions.empty()) {
    Compiler::ParseFile(imported[0]);
    _imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  }
  for (auto &n: _imported_functions) {
    auto f = ast_cast<ASTFunction>(n);
    TAN_ASSERT(f);
    _cs->add_function(f);
  }
  return _end_index;
}

ASTImport::ASTImport(Token *token, size_t token_index) : ASTNode(ASTType::IMPORT, 0, 0, token, token_index) {}

str ASTImport::to_string(bool print_prefix) const {
  str ret = "";
  if (print_prefix) {
    ret += ASTNode::to_string(print_prefix) + " ";
  }
  ret += _file;
  return ret;
}

} // namespace tanlang
