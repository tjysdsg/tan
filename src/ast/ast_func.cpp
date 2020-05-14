#include "src/ast/ast_func.h"
#include "src/ast/ast_arg_decl.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_ty.h"
#include "parser.h"
#include "reader.h"
#include "token.h"
#include "intrinsic.h"
#include "compiler_session.h"
#include "src/type_system.h"

namespace tanlang {

Value *ASTFunction::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  Metadata *ret_meta = _children[0]->to_llvm_meta(cs);
  /// generate prototype
  Function *F = (Function *) codegen_prototype(cs);

  cs->push_scope(_scope); /// push scope
  /// set function arg types
  std::vector<Metadata *> arg_metas;
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    auto type_name = ast_cast<ASTTy>(_children[i]->_children[1]);
    arg_metas.push_back(type_name->to_llvm_meta(cs));
  }

  /// get function name
  str func_name = this->get_name();
  /// function implementation
  if (!_is_external) {
    /// create a new basic block to start insertion into
    BasicBlock *main_block = BasicBlock::Create(*cs->get_context(), "func_entry", F);
    cs->get_builder()->SetInsertPoint(main_block);

    /// debug information
    DIScope *di_scope = cs->get_current_di_scope();
    auto *di_file = cs->get_di_file();
    auto *di_func_t = create_function_type(cs, ret_meta, arg_metas);
    DISubprogram *subprogram = cs->get_di_builder()
        ->createFunction(di_scope,
            func_name,
            func_name,
            di_file,
            (unsigned) _token->l + 1,
            di_func_t,
            (unsigned) _token->l + 1,
            DINode::FlagPrototyped,
            DISubprogram::SPFlagDefinition,
            nullptr,
            nullptr,
            nullptr);
    F->setSubprogram(subprogram);
    cs->push_di_scope(subprogram);
    /// reset debug emit location
    cs->get_builder()->SetCurrentDebugLocation(DebugLoc());

    /// add all function arguments to scope
    size_t i = 2;
    for (auto &a : F->args()) {
      auto arg_name = _children[i]->get_name();
      auto *arg_val = _children[i]->codegen(cs);
      cs->get_builder()->CreateStore(&a, arg_val);
      /// create a debug descriptor for the arguments
      auto *arg_meta = ast_cast<ASTTy>(_children[i]->_children[1])->to_llvm_meta(cs);
      llvm::DILocalVariable *di_arg = cs->get_di_builder()
          ->createParameterVariable(subprogram,
              arg_name,
              (unsigned) i + 1,
              di_file,
              (unsigned) _token->l + 1,
              (DIType *) arg_meta,
              true);
      cs->get_di_builder()
          ->insertDeclare(arg_val,
              di_arg,
              cs->get_di_builder()->createExpression(),
              llvm::DebugLoc::get((unsigned) _token->l + 1, (unsigned) _token->c + 1, subprogram),
              cs->get_builder()->GetInsertBlock());
      ++i;
    }

    /// set debug emit location to function body
    cs->get_builder()
        ->SetCurrentDebugLocation(DebugLoc::get((unsigned) _children[_children.size() - 1]->_token->l + 1,
            (unsigned) _children[_children.size() - 1]->_token->c + 1,
            subprogram));
    /// generate function body
    _children[_children.size() - 1]->codegen(cs);

    /// create a return instruction if there is none, the return value is the default value of the return type
    if (!(cs->get_builder()->GetInsertBlock()->back().getOpcode() & llvm::Instruction::Ret)) {
      auto ret_ty = _children[0]->get_ty();
      if (ret_ty->_tyty == Ty::VOID) { cs->get_builder()->CreateRetVoid(); }
      else {
        auto *ret_val = _children[0]->get_llvm_value(cs);
        TAN_ASSERT(ret_val);
        cs->get_builder()->CreateRet(ret_val);
      }
    }
    cs->pop_di_scope();
    /// restore parent code block
    cs->get_builder()->SetInsertPoint(main_block);
  }
  cs->pop_scope(); /// pop scope
  return nullptr;
}

Value *ASTFunction::codegen_prototype(CompilerSession *compiler_session, bool import) {
  Type *ret_type = _children[0]->to_llvm_type(compiler_session);
  std::vector<Type *> arg_types;
  /// set function arg types
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    auto type_name = ast_cast<ASTTy>(_children[i]->_children[1]);
    arg_types.push_back(type_name->to_llvm_type(compiler_session));
  }
  /// create function prototype
  FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
  auto linkage = Function::InternalLinkage;
  if (_is_external) { linkage = Function::ExternalWeakLinkage; }
  if (_is_public) {
    if (import) {
      linkage = Function::ExternalWeakLinkage;
    } else {
      linkage = Function::ExternalLinkage;
    }
  }
  _func = Function::Create(FT, linkage, this->get_name(), compiler_session->get_module());
  _func->setCallingConv(llvm::CallingConv::C);

  /// set argument names
  auto args = _func->args().begin();
  for (size_t i = 2, j = 0; i < _children.size() - !_is_external; ++i, ++j) {
    /// rename this since the real function argument is stored as variables
    (args + j)->setName("_" + _children[i]->get_name());
  }
  return _func;
}

Value *ASTFunctionCall::codegen(CompilerSession *cm) {
  /// args
  size_t n_args = _children.size();
  std::vector<Value *> arg_vals;
  std::vector<Type *> arg_types;
  for (size_t i = 0; i < n_args; ++i) {
    Type *expected_type = get_callee()->get_arg(i)->to_llvm_type(cm);
    auto *a = _children[i]->codegen(cm);
    if (!a) { report_code_error(_children[i]->_token, "Invalid function call argument"); }

    /// implicit cast
    a = TypeSystem::ConvertTo(cm, expected_type, a, _children[i]->is_lvalue());
    Type *a_type = a->getType();
    arg_vals.push_back(a);
    arg_types.push_back(a_type);
  }
  _llvm_value = cm->get_builder()->CreateCall(get_callee()->get_func(), arg_vals);
  return _llvm_value;
}

ASTNodePtr ASTFunction::get_ret() const {
  TAN_ASSERT(_children.size());
  return _children[0];
}

ASTNodePtr ASTFunction::get_arg(size_t i) const {
  TAN_ASSERT(i + 2 < _children.size());
  return _children[i + 2];
}

size_t ASTFunction::get_n_args() const {
  TAN_ASSERT(_children.size() >= (_is_external ? 2 : 3)); /// minus return, function name (, function body)
  return _children.size() - (_is_external ? 2 : 3);
}

Function *ASTFunction::get_func() const { return _func; }

void ASTFunction::set_func(Function *f) { _func = f; }

size_t ASTFunction::nud() {
  if (_parser->at(_start_index)->value == "fn") {
    /// skip "fn"
    _end_index = _start_index + 1;
  } else if (_parser->at(_start_index)->value == "pub") {
    _is_public = true;
    /// skip "pub fn"
    _end_index = _start_index + 2;
  } else if (_parser->at(_start_index)->value == "extern") {
    _is_external = true;
    /// skip "pub fn"
    _end_index = _start_index + 2;
  } else { TAN_ASSERT(false); }

  /// function return type, set later
  _children.push_back(nullptr);

  /// function name
  auto name = ast_cast<ASTIdentifier>(_parser->parse<ASTType::ID>(_end_index, true));
  _name = name->get_name();
  _children.push_back(name);

  /// add self to function table
  if (_is_public || _is_external) {
    CompilerSession::AddPublicFunction(_parser->get_filename(), this->shared_from_this());
  }
  /// ... and internal function table
  _cs->add_function(this->shared_from_this());

  /// only push scope if having function body
  if (!_is_external) { _scope = _cs->push_scope(); } else { _scope = _cs->get_current_scope(); }

  /// arguments
  _parser->peek(_end_index, TokenType::PUNCTUATION, "(");
  ++_end_index;
  if (_parser->at(_end_index)->value != ")") {
    while (!_parser->eof(_end_index)) {
      ASTNodePtr arg = std::make_shared<ASTArgDecl>(_parser->at(_end_index), _end_index);
      _end_index = arg->parse(_parser, _cs); /// this will add args to the current scope
      _children.push_back(arg);
      if (_parser->at(_end_index)->value == ",") {
        ++_end_index;
      } else { break; }
    }
  }
  _parser->peek(_end_index, TokenType::PUNCTUATION, ")");
  ++_end_index;
  _parser->peek(_end_index, TokenType::PUNCTUATION, ":");
  ++_end_index;
  _children[0] = _parser->parse<ASTType::TY>(_end_index, true); /// return type
  // TODO: set _tyty

  /// body
  if (!_is_external) {
    auto body = _parser->peek(_end_index, TokenType::PUNCTUATION, "{");
    _end_index = body->parse(_parser, _cs);
    _children.push_back(body);
    _cs->pop_scope();
  }
  return _end_index;
}

bool ASTFunction::is_named() const { return true; }

bool ASTFunction::is_typed() const { return true; }

size_t ASTFunctionCall::nud() {
  if (_parsed) { return _end_index; }
  _end_index = _start_index + 1; /// skip function name
  auto *token = _parser->at(_end_index);
  if (token->value != "(") { report_code_error(token, "Invalid function call"); }
  ++_end_index;
  while (!_parser->eof(_end_index) && _parser->at(_end_index)->value != ")") {
    _children.push_back(_parser->next_expression(_end_index));
    if (_parser->at(_end_index)->value == ",") { /// skip ,
      ++_end_index;
    } else { break; }
  }
  _parser->peek(_end_index, TokenType::PUNCTUATION, ")");
  ++_end_index;
  _ty = ast_cast<ASTTy>(get_callee()->get_ret());
  return _end_index;
}

ASTFunctionPtr ASTFunctionCall::get_callee() const {
  if (!_callee) {
    auto func_candidates = _cs->get_functions(_name);
    /// always prefer the function with lowest cost if multiple candidates are callable
    /// one implicit cast -> +1 cost
    /// however, if two (or more) functions have the same score, an error is raise (ambiguous call)
    size_t cost = (size_t) -1;
    for (const auto &f : func_candidates) {
      size_t n = f->get_n_args();
      if (n != _children.size()) { continue; }
      bool good = true;
      size_t c = 0;
      for (size_t i = 0; i < n; ++i) { /// check every argument (return type not checked)
        auto arg = f->get_arg(i);
        TAN_ASSERT(arg->is_typed());
        auto actual_arg = _children[i];
        /// allow implicit cast from actual_arg to arg, but not in reverse
        auto t1 = arg->get_ty();
        auto t2 = actual_arg->get_ty();
        if (*t1 != *t2) {
          if (0 != TypeSystem::CanImplicitCast(t1, t2)) {
            good = false;
            break;
          }
          ++c;
        }
      }
      if (good) {
        if (c < cost) {
          _callee = f;
          cost = c;
        } else if (c == cost) { report_code_error(_token, "Ambiguous function call: " + _name); }
      }
    }
  }
  if (!_callee) { report_code_error(_token, "Unknown function call: " + _name); }
  return _callee;
}

bool ASTFunctionCall::is_named() const { return true; }

bool ASTFunctionCall::is_lvalue() const { return false; }

bool ASTFunctionCall::is_typed() const { return true; }

ASTFunctionCall::ASTFunctionCall(Token *t, size_t ti) : ASTNode(ASTType::FUNC_CALL, 0, 0, t, ti) {
  _name = t->value;
}

ASTFunction::ASTFunction(Token *token, size_t token_index) : ASTNode(ASTType::FUNC_DECL, 0, 0, token, token_index) {}

} // namespace tanlang
