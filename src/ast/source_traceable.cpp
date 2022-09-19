#include "source_traceable.h"
#include "token.h"
#include <iostream>

using namespace tanlang;

SourceTraceable::SourceTraceable(SrcLoc loc) : _loc(loc) {}

const SrcLoc &SourceTraceable::loc() const { return _loc; }

void SourceTraceable::set_loc(SrcLoc loc) { _loc = loc; }
