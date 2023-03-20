#ifndef __TAN_SRC_AST_PACKAGE_H__
#define __TAN_SRC_AST_PACKAGE_H__
#include "base.h"

namespace tanlang {

class Program;

class Package {
public:
  static Package *Create(str package_name, vector<Program *> sources);

  const str &get_package_name() const;
  const vector<Program *> &get_sources() const;

protected:
  explicit Package(str package_name, vector<Program *> sources);

private:
  str _package_name;
  vector<Program *> _sources;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_PACKAGE_H__
