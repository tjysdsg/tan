#include "ast/source_traceable.h"
#include "source_file/token.h"

using namespace tanlang;

TokenSpan::TokenSpan(uint32_t start, uint32_t end) : _start(start), _end(end) {}

SourceTraceable::SourceTraceable(SourceFile *src) : _span(TokenSpan(0, 0)), _src(src) {}

uint32_t SourceTraceable::start() const { return _span._start; }

uint32_t SourceTraceable::end() const { return _span._end; }

void SourceTraceable::set_start(uint32_t val) { _span._start = val; }

void SourceTraceable::set_end(uint32_t val) { _span._end = val; }

SourceFile *SourceTraceable::src() const { return _src; }
