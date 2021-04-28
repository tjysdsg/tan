#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_member_access.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include "src/parser/parser_impl.h"
#include <memory>
#include <utility>

using namespace tanlang;

Parser::Parser(vector<Token *> tokens, str filename, CompilerSession *cs) : _impl(new ParserImpl(tokens,
    filename,
    cs)) {
}

ASTNodePtr Parser::parse() {
  return _impl->parse();
}

str Parser::get_filename() const {
  return _impl->get_filename();
}
