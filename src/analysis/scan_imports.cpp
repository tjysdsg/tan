#include "ast/package.h"
#include "ast/stmt.h"
#include "analysis/scan_imports.h"
#include "driver/driver.h"

using namespace tanlang;

uset<str> ScanImports::run_impl(Package *package) {
  uset<str> ret{};

  for (auto *node : package->get_children()) {
    if (node->get_node_type() == ASTNodeType::IMPORT) {
      auto *p = pcast<Import>(node);
      ret.insert(p->get_name());
    }
  }

  return ret;
}
