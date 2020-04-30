#include "src/ast/ast_struct.h"
#include "src/type_system.h"
#include "src/common.h"
#include "compiler_session.h"
#include "parser.h"
#include "src/llvm_include.h"

namespace tanlang {

size_t ASTStruct::nud() {
  _end_index = _start_index + 1; /// skip "struct"
  /// struct typename
  auto id = _parser->parse<ASTType::ID>(_end_index, true);
  assert(id->is_named());
  _type_name = id->get_name();
  _children.push_back(id);
  _cs->add(id->get_name(), this->shared_from_this()); /// add self to current scope

  /// struct body
  auto comp_stmt = _parser->peek(_end_index);
  _end_index = comp_stmt->parse(_parser, _cs); // TODO: parse forward declaration
  auto members = comp_stmt->_children;

  /// resolve member names and types
  ASTNodePtr var_decl = nullptr;
  size_t n = comp_stmt->_children.size();
  _member_names.reserve(n);
  _children.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    if (members[i]->_type == ASTType::VAR_DECL) { /// member variable without initial value
      var_decl = members[i];
      _children.push_back(var_decl->get_ty());
    } else if (members[i]->_type == ASTType::ASSIGN) { /// member variable with an initial value
      var_decl = members[i]->_children[0];
      auto initial_value = members[i]->_children[1];
      if (!is_ast_type_in(initial_value->_type, TypeSystem::LiteralTypes)) {
        report_code_error(_token, "Invalid initial value of the member variable");
      }
      _children.push_back(initial_value->get_ty()); /// initial value is set to ASTTy in ASTLiteral::get_ty()
    } else { report_code_error(_token, "Invalid struct member"); }
    auto name = var_decl->get_name();
    _member_names.push_back(name);
    _member_indices[name] = i;
  }
  this->resolve();
  return _end_index;
}

ASTStruct::ASTStruct(Token *token, size_t token_index) : ASTTy(token, token_index) {
  _type = ASTType::STRUCT_DECL;
  _ty = Ty::STRUCT;
}

size_t ASTStruct::get_member_index(std::string name) {
  if (_member_indices.find(name) == _member_indices.end()) {
    throw std::runtime_error("Unknown member of struct '" + get_type_name() + "'");
  }
  return _member_indices[name];
}

std::string ASTStruct::get_type_name() const { return _type_name; }

// TODO: optimize this
llvm::Type *ASTStruct::to_llvm_type(CompilerSession *compiler_session) const {
  if (!_llvm_type) {
    auto *struct_type = StructType::create(*compiler_session->get_context(), _type_name);
    std::vector<Type *> body{};
    size_t n = _children.size();
    body.reserve(n);
    for (size_t i = 1; i < n; ++i) { body.push_back(_children[i]->to_llvm_type(compiler_session)); }
    struct_type->setBody(body);
    _llvm_type = struct_type;
  }
  return _llvm_type;
}

ASTNodePtr ASTStruct::get_member(size_t i) { return _children[i + 1]; }

llvm::Value *ASTStruct::get_llvm_value(CompilerSession *cs) const {
  std::vector<llvm::Constant *> values{};
  size_t n = _children.size();
  for (size_t i = 1; i < n; ++i) { values.push_back((llvm::Constant *) _children[i]->get_ty()->get_llvm_value(cs)); }
  return ConstantStruct::get((StructType *) to_llvm_type(cs), values);
}

} // namespace tanlang
