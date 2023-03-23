#include "ast/package.h"

#include <utility>

using namespace tanlang;

Package *Package::Create(str package_name, vector<Program *> sources, vector<SourceManager *> sms) {
  auto *ret = new Package();
  ret->_package_name = std::move(package_name);
  ret->_sources = std::move(sources);
  ret->_sms = std::move(sms);
  return ret;
}

const str &Package::get_package_name() const { return _package_name; }

const vector<Program *> &Package::get_asts() const { return _sources; }

const vector<SourceManager *> &Package::get_source_managers() const { return _sms; }
