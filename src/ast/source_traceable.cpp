#include "ast/source_traceable.h"
#include "source_file/token.h"

using namespace tanlang;

TokenSpan::TokenSpan(uint32_t start, uint32_t end) : _start(start), _end(end) {}

SourceTraceable::SourceTraceable(SourceFile *src) : _span(TokenSpan(0, 0)) {}

const TokenSpan &SourceTraceable::span() const { return _span; }

void SourceTraceable::set_span(TokenSpan span) { _span = span; }
