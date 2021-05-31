#ifndef __TAN_SRC_ANALYSIS_ANALYZER_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_H__
#include "base.h"

namespace tanlang {

class ASTBase;
using ASTBasePtr = ptr<ASTBase>;
class AnalyzerImpl;

class Analyzer {
public:
  Analyzer(CompilerSession *cs);
  ~Analyzer();
  void analyze(ASTBasePtr p);

private:
  AnalyzerImpl *_analyzer_impl = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_H__
