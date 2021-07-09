#include "source_traceable.h"
#include "token.h"
#include <iostream>

using namespace tanlang;

SourceTraceable::SourceTraceable(SourceIndex loc) { _loc = loc; }

const SourceIndex &SourceTraceable::get_loc() const { return _loc; }

void SourceTraceable::set_loc(SourceIndex loc) { _loc = loc; }
