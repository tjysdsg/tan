#include "src/ast/ast_var_decl.h"
#include "compiler.h"
#include "token.h"
#include "compiler_session.h"
#include "src/common.h"
#include "src/ast/ast_identifier.h"

namespace tanlang {

tanlang::ASTVarDecl::ASTVarDecl(Token *token, size_t token_index) : ASTNode(ASTType::VAR_DECL,
    0,
    0,
    token,
    token_index) {}

size_t tanlang::ASTVarDecl::_nud(Parser *parser) {
  _children.push_back(parser->parse<ASTType::ID>(_end_index, true)); /// name
  // TODO: type inference for variable declarations
  parser->peek(_end_index, TokenType::PUNCTUATION, ":");
  ++_end_index;
  /// type
  _ty = ast_cast<ASTTy>(parser->parse<ASTType::TY>(_end_index, true));
  _ty->set_is_lvalue(true);
  _children.push_back(_ty);
  auto *cm = Compiler::get_compiler_session(_parser->get_filename());
  cm->add(this->get_name(), this->shared_from_this());
  return _end_index;
}

size_t ASTVarDecl::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "var"
  return _nud(parser);
}

Value *ASTVarDecl::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  assert(_children[0]->is_named());
  std::string name = this->get_name();
  Type *type = ast_cast<ASTTy>(_children[1])->to_llvm_type(compiler_session);
  Value *var = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), type, name);

  /// set initial value
  if (_has_initial_val) {
    compiler_session->get_builder()->CreateStore(_children[2]->codegen(compiler_session), var);
  }
  this->_llvm_value = var;
  return _llvm_value;
}

std::string ASTVarDecl::get_name() const {
  auto n = ast_cast<ASTIdentifier>(_children[0]);
  assert(n);
  return n->get_name();
}

std::string ASTVarDecl::get_type_name() const {
  return _children[1]->get_type_name();
}

std::shared_ptr<ASTTy> tanlang::ASTVarDecl::get_ty() const {
  return _ty;
}

llvm::Type *ASTVarDecl::to_llvm_type(CompilerSession *compiler_session) const {
  auto t = ast_cast<ASTTy>(_children[1]);
  return t->to_llvm_type(compiler_session);
}

llvm::Value *ASTVarDecl::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

bool tanlang::ASTVarDecl::is_typed() const { return true; }

bool tanlang::ASTVarDecl::is_named() const { return true; }

bool tanlang::ASTVarDecl::is_lvalue() const { return true; }

} // namespace tanlang
