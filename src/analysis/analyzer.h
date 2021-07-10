#ifndef __TAN_SRC_ANALYSIS_ANALYZER_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_H__
#include "base.h"
#include "src/ast/fwd.h"

namespace tanlang {

class AnalyzerImpl;

class Analyzer {
public:
  Analyzer(ASTContext *ctx);
  ~Analyzer();
  void analyze(ASTBase *p);

private:
  AnalyzerImpl *_analyzer_impl = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_H__
