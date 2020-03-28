#ifndef TAN_SRC_COMPILER_INTERPRETER_H_
#define TAN_SRC_COMPILER_INTERPRETER_H_
#include <memory>
#include <utility>
#include <vector>
#include "parser.h"
#include "src/llvm_include.h"

namespace tanlang {

class Interpreter : public Parser {
public:
  explicit Interpreter(std::vector<Token *> tokens);
  Expected<JITEvaluatedSymbol> lookup(StringRef Name);
  Error evaluate(std::unique_ptr<Module> module = nullptr) override;
  void dump() const override;
};

} // namespace tanlang

#endif /*TAN_SRC_COMPILER_INTERPRETER_H_*/
