#include "analyzer.h"
#include "src/analysis/analyzer_impl.h"

using namespace tanlang;

void Analyzer::analyze(ParsableASTNodePtr p) {
  _analyzer_impl->analyze(p);
}

Analyzer::Analyzer(CompilerSession *cs) {
  _analyzer_impl = new AnalyzerImpl(cs);
}

Analyzer::~Analyzer() {
  if (_analyzer_impl) { delete _analyzer_impl; }
}
