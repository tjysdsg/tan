#include "ast/package.h"
#include "ast/stmt.h"
#include "analysis/scan_imports.h"
#include "driver/driver.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace tanlang;

ScanImportsOutputType ScanImports::run_impl(Package *package) {
  ScanImportsOutputType ret{};

  CompilerDriver *driver = CompilerDriver::instance();
  for (auto *node : package->get_children()) {
    if (node->get_node_type() == ASTNodeType::IMPORT) {
      auto *p = pcast<Import>(node);
      str name = p->get_name();

      // Find source files of the package
      // TODO: should we look around callee_path?
      auto res = driver->resolve_import(node->src()->get_filename(), name);
      if (res.empty())
        error(ErrorType::IMPORT_ERROR, node, "Cannot find package: " + name);

      if (fs::is_directory(res[0])) { // files in the folder
        for (const auto &entry : fs::directory_iterator(res[0])) {
          auto path = entry.path();
          if (!fs::is_directory(path) && path.has_extension() && path.extension() == ".tan") {
            ret[name].insert(path);
          }
        }
      } else { // TODO: a tan source file with the package name?
        TAN_ASSERT(false);
      }
    }
  }

  return ret;
}
