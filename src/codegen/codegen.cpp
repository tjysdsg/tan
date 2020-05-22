#include <src/common.h>
#include "src/codegen/codegen.h"
#include "compiler_session.h"
#include "src/llvm_include.h"
#include "src/ast/ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/analysis/type_system.h"
#include "src/common.h"
#include "token.h"

namespace tanlang {

static Value *codegen_arithmetic(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  /// unary plus/minus
  if (p->_children.size() == 1) {
    if (!is_ast_type_in(p->_type, {ASTType::SUM, ASTType::SUBTRACT})) { error(cs, "Invalid unary operation"); }
    if (p->_type == ASTType::SUM) { return codegen(cs, p->_children[0]); }
    else {
      auto *r = codegen(cs, p->_children[0]);
      if (p->_children[0]->_ty->_is_lvalue) { r = builder->CreateLoad(r); }
      if (r->getType()->isFloatingPointTy()) { return builder->CreateFNeg(r); }
      return builder->CreateNeg(r);
    }
  }

  /// binary operator
  auto lhs = p->_children[0];
  auto rhs = p->_children[1];
  Value *l = codegen(cs, p->_children[0]);
  Value *r = codegen(cs, p->_children[1]);
  TAN_ASSERT(l && r);
  TAN_ASSERT(p->_children.size() > p->_dominant_idx);

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, lhs->_ty);
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, lhs->_ty);
  } else {
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, rhs->_ty);
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, rhs->_ty);
  }

  if (l->getType()->isFloatingPointTy()) {
    /// float arithmetic
    if (p->_type == ASTType::MULTIPLY) {
      p->_llvm_value = builder->CreateFMul(l, r, "mul_tmp");
    } else if (p->_type == ASTType::DIVIDE) {
      p->_llvm_value = builder->CreateFDiv(l, r, "div_tmp");
    } else if (p->_type == ASTType::SUM) {
      p->_llvm_value = builder->CreateFAdd(l, r, "sum_tmp");
    } else if (p->_type == ASTType::SUBTRACT) {
      p->_llvm_value = builder->CreateFSub(l, r, "sub_tmp");
    } else if (p->_type == ASTType::MOD) {
      p->_llvm_value = builder->CreateFRem(l, r, "mod_tmp");
    } else { TAN_ASSERT(false); }
  } else {
    /// integer arithmetic
    if (p->_type == ASTType::MULTIPLY) {
      p->_llvm_value = builder->CreateMul(l, r, "mul_tmp");
    } else if (p->_type == ASTType::DIVIDE) {
      auto ty = p->_children[0]->_ty;
      if (ty->_is_unsigned) { p->_llvm_value = builder->CreateUDiv(l, r, "div_tmp"); }
      else { p->_llvm_value = builder->CreateSDiv(l, r, "div_tmp"); }
    } else if (p->_type == ASTType::SUM) {
      p->_llvm_value = builder->CreateAdd(l, r, "sum_tmp");
    } else if (p->_type == ASTType::SUBTRACT) {
      p->_llvm_value = builder->CreateSub(l, r, "sub_tmp");
    } else if (p->_type == ASTType::MOD) {
      auto ty = p->_children[0]->_ty;
      if (ty->_is_unsigned) { p->_llvm_value = builder->CreateURem(l, r, "mod_tmp"); }
      else { p->_llvm_value = builder->CreateSRem(l, r, "mod_tmp"); }
    } else { TAN_ASSERT(false); }
  }
  return p->_llvm_value;
}

static Value *codegen_comparison(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  auto lhs = p->_children[0];
  auto rhs = p->_children[1];
  Value *l = codegen(cs, lhs);
  Value *r = codegen(cs, rhs);
  TAN_ASSERT(l && r);
  TAN_ASSERT(p->_children.size() > p->_dominant_idx);

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, lhs->_ty);
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, lhs->_ty);
  } else {
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, rhs->_ty);
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, rhs->_ty);
  }

  if (l->getType()->isFloatingPointTy()) {
    if (p->_type == ASTType::EQ) {
      p->_llvm_value = builder->CreateFCmpOEQ(l, r, "eq");
    } else if (p->_type == ASTType::NE) {
      p->_llvm_value = builder->CreateFCmpONE(l, r, "ne");
    } else if (p->_type == ASTType::GT) {
      p->_llvm_value = builder->CreateFCmpOGT(l, r, "gt");
    } else if (p->_type == ASTType::GE) {
      p->_llvm_value = builder->CreateFCmpOGE(l, r, "ge");
    } else if (p->_type == ASTType::LT) {
      p->_llvm_value = builder->CreateFCmpOLT(l, r, "lt");
    } else if (p->_type == ASTType::LE) {
      p->_llvm_value = builder->CreateFCmpOLE(l, r, "le");
    }
  } else {
    if (p->_type == ASTType::EQ) {
      p->_llvm_value = builder->CreateICmpEQ(l, r, "eq");
    } else if (p->_type == ASTType::NE) {
      p->_llvm_value = builder->CreateICmpNE(l, r, "ne");
    } else if (p->_type == ASTType::GT) {
      p->_llvm_value = builder->CreateICmpUGT(l, r, "gt");
    } else if (p->_type == ASTType::GE) {
      p->_llvm_value = builder->CreateICmpUGE(l, r, "ge");
    } else if (p->_type == ASTType::LT) {
      p->_llvm_value = builder->CreateICmpULT(l, r, "lt");
    } else if (p->_type == ASTType::LE) {
      p->_llvm_value = builder->CreateICmpULE(l, r, "le");
    }
  }
  return p->_llvm_value;
}

static Value *codegen_assignment(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  /// _codegen the rhs
  auto lhs = p->_children[0];
  auto rhs = p->_children[1];
  Value *from = codegen(cs, rhs);
  Value *to = codegen(cs, lhs);
  if (!from) { error(cs, "Invalid expression for right-hand operand of the assignment"); }
  if (!to) { error(cs, "Invalid left-hand operand of the assignment"); }
  if (!lhs->_ty->_is_lvalue) { error(cs, "Value can only be assigned to lvalue"); }

  from = TypeSystem::ConvertTo(cs, from, rhs->_ty, lhs->_ty);
  builder->CreateStore(from, to);
  p->_llvm_value = to;
  return to;
}

Value *codegen(CompilerSession *cs, ASTNodePtr p) {
  Value *ret = nullptr;
  switch (p->_type) {
    case ASTType::PROGRAM:
      for (const auto &e : p->_children) { codegen(cs, e); }
      ret = nullptr;
      break;
    case ASTType::SUM:
    case ASTType::SUBTRACT:
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD:
      ret = codegen_arithmetic(cs, p);
      break;
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
      ret = codegen_comparison(cs, p);
      break;
    case ASTType::ASSIGN:
      ret = codegen_assignment(cs, p);
      break;
    default:
      break;
  }
  return ret;
}

} // namespace tanlang
