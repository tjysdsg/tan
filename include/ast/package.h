#ifndef __TAN_SRC_AST_PACKAGE_H__
#define __TAN_SRC_AST_PACKAGE_H__
#include "base.h"

namespace tanlang {

class Program;
class SourceManager;

class Package {
public:
  static Package *Create(str package_name, vector<Program *> sources, vector<SourceManager *> sms);

  const str &get_package_name() const;
  const vector<Program *> &get_asts() const;
  const vector<SourceManager *> &get_source_managers() const;

protected:
  Package() = default;

private:
  str _package_name;
  vector<Program *> _sources;
  vector<SourceManager *> _sms;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_PACKAGE_H__
