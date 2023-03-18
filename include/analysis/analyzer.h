#ifndef __TAN_SRC_ANALYSIS_ANALYZER_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_H__
#include "base.h"
#include "ast/fwd.h"

namespace tanlang {

class AnalyzerImpl;

class Analyzer {
public:
  Analyzer() = delete;
  explicit Analyzer(SourceManager *sm);
  ~Analyzer();
  void analyze(ASTBase *p);

private:
  AnalyzerImpl *_analyzer_impl = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_ANALYZER_H__
