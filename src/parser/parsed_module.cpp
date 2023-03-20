#include <utility>
#include "parser/parsed_module.h"

using namespace tanlang;

ParsedModule::ParsedModule(Program *program, SourceManager *sm, str filename, str package_name)
    : _program(program), _sm(sm), _filename(std::move(filename)), _package_name(std::move(package_name)) {}
