#ifndef __TAN_SRC_ANALYSIS_ANALYZER_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_H__
#include "base.h"

namespace tanlang {

class AnalyzerImpl;
class Program;
class SourceManager;
class ASTBase;

class Analyzer {
public:
  Analyzer() = delete;
  explicit Analyzer(SourceManager *sm);
  ~Analyzer();
  void analyze(Program *p);
  void analyze_top_level_declarations(Program *p);
  vector<ASTBase *> sorted_unresolved_symbols() const;

private:
  AnalyzerImpl *_analyzer_impl = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_ANALYZER_H__
