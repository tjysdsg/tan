bool AnalyzerImpl::CanImplicitlyConvert(Type *from, Type *to) {
  TAN_ASSERT(from && to);

  if (from == to) {
    return true;
  }

  int s1 = from->get_size_bits();
  int s2 = to->get_size_bits();
  if (from->is_int() && to->is_int()) {
    bool u1 = from->is_unsigned();
    bool u2 = to->is_unsigned();

    if (u1 ^ u2) { // rule #1
      return s2 >= s1;
    } else {
      return s2 > s1; // rule #2 and #3
    }
  } else if (from->is_float() && to->is_float()) { // rule #4
    return s2 >= s1;
  }

  // rule #5
  else if (from->is_int() && to->is_float()) {
    return true;
  }

  // # rule 6
  else if ((from->is_num() || from->is_pointer()) && to->is_bool()) {
    return true;
  }

  // # rule 7
  else if (from->is_bool() && to->is_num()) {
    return true;
  }

  // TODO: rule #8 and #9
  else {
    return false;
  }
}

Type *AnalyzerImpl::ImplicitTypePromote(Type *t1, Type *t2) {
  TAN_ASSERT(t1 && t2);

  if (t1 == t2) {
    return t1;
  }

  int s1 = t1->get_size_bits();
  int s2 = t2->get_size_bits();
  if (t1->is_int() && t2->is_int()) {
    bool u1 = t1->is_unsigned();
    bool u2 = t2->is_unsigned();

    if (u1 ^ u2) { // rule #1
      return s1 > s2 ? t1 : t2;
    } else {
      // let t1 be the unsigned, t2 be the signed
      if (!u1) {
        std::swap(t1, t2);
        std::swap(s1, s2);
        std::swap(u1, u2);
      }

      if (s2 > s1) { // rule #2
        return t2;
      } else if (s1 > s2) { // rule #3
        return t1;
      } else {
        return nullptr;
      }
    }
  } else if (t1->is_float() && t2->is_float()) { // rule #4
    return s1 >= s2 ? t1 : t2;
  }

  // rule #5
  else if (t1->is_float() && t2->is_int()) {
    return t1;
  } else if (t1->is_int() && t2->is_float()) {
    return t2;
  }

  // # rule 6
  else if (t1->is_bool() && (t2->is_num() || t2->is_pointer())) {
    return t1;
  } else if ((t1->is_num() || t1->is_pointer()) && t2->is_bool()) {
    return t2;
  }

  // TODO: rule #8 and #9
  else {
    return nullptr;
  }
}
