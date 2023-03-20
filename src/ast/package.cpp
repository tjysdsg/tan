#include "ast/package.h"

#include <utility>

using namespace tanlang;

Package::Package(str package_name, vector<Program *> sources)
    : _package_name(std::move(package_name)), _sources(std::move(sources)) {}

Package *Package::Create(str package_name, vector<Program *> sources) {
  return new Package(std::move(package_name), std::move(sources));
}

const str &Package::get_package_name() const { return _package_name; }
const vector<Program *> &Package::get_sources() const { return _sources; }
