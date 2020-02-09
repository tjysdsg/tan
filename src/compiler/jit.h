#ifndef TAN_SRC_COMPILER_JIT_H_
#define TAN_SRC_COMPILER_JIT_H_
#include <memory>
#include <utility>
#include "parser.h"
#include "src/llvm_include.h"

namespace tanlang {

class JIT : public Parser {
 public:
  explicit JIT(std::vector<Token *> tokens);
  Expected<JITEvaluatedSymbol> lookup(StringRef Name);
  Error evaluate(std::unique_ptr<Module> module = nullptr) override;
};

} // namespace tanlang

#endif /* TAN_SRC_COMPILER_JIT_H_ */
