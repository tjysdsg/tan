#ifndef __TAN_SRC_ANALYSIS_ANALYZER_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_H__
#include "base.h"

namespace tanlang {

class ParsableASTNode;
using ParsableASTNodePtr = ptr<ParsableASTNode>;
class AnalyzerImpl;

class Analyzer {
public:
  Analyzer(CompilerSession *cs);
  ~Analyzer();
  void analyze(ParsableASTNodePtr p);

private:
  AnalyzerImpl *_analyzer_impl = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_H__
