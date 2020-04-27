#include "src/ast/ast_struct.h"
#include "compiler_session.h"
#include "parser.h"
#include "src/llvm_include.h"
#include "compiler.h"

namespace tanlang {

size_t ASTStruct::nud(Parser *parser) {
  auto *compiler_session = Compiler::get_compiler_session(parser->get_filename());
  _end_index = _start_index + 1; /// skip "struct"
  /// struct typename
  auto id = parser->parse<ASTType::ID>(_end_index, true);
  assert(id->is_named());
  _type_name = id->get_name();
  _children.push_back(id);
  compiler_session->add(id->get_name(), this->shared_from_this()); /// add self to current scope

  /// struct body
  auto comp_statements = parser->peek(_end_index);
  _end_index = comp_statements->parse(parser); // TODO: parse forward declaration
  _children.insert(_children.begin() + 1, comp_statements->_children.begin(), comp_statements->_children.end());

  /// resolve members
  ASTNodePtr var_decl = nullptr;
  for (size_t i = 1; i < _children.size(); ++i) {
    if (_children[i]->_type == ASTType::VAR_DECL) { /// member variable without initial value
      var_decl = _children[i];
    } else if (_children[i]->_type == ASTType::ASSIGN) { /// member variable with an initial value
      // TODO: remember initial value
      var_decl = _children[i]->_children[0];
    } else { report_code_error(_token, "Invalid struct member"); }
    _members.push_back(var_decl);
    _member_indices[var_decl->get_name()] = i - 1;
  }
  return _end_index;
}

ASTStruct::ASTStruct(Token *token, size_t token_index) : ASTNode(ASTType::STRUCT_DECL, 0, 0, token, token_index) {}

Value *ASTStruct::codegen(CompilerSession *compiler_session) {
  auto *struct_type = StructType::create(*compiler_session->get_context(), _type_name);
  std::vector<Type *> body{};
  size_t n = _members.size();
  body.reserve(n);
  for (const auto &m: _members) { body.push_back(m->to_llvm_type(compiler_session)); }
  struct_type->setBody(body);
  _llvm_type = struct_type;
  return nullptr;
}

size_t ASTStruct::get_member_index(std::string name) {
  if (_member_indices.find(name) == _member_indices.end()) {
    throw std::runtime_error("Unknown member of struct '" + get_type_name() + "'");
  }
  return _member_indices[name];
}

std::string ASTStruct::get_type_name() const {
  return _type_name;
}

llvm::Type *ASTStruct::to_llvm_type(CompilerSession *) const {
  return _llvm_type;
}

ASTNodePtr ASTStruct::get_member(size_t i) {
  return _children[i + 1];
}

bool ASTStruct::is_typed() const { return true; }

} // namespace tanlang
