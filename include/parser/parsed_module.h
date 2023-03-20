#ifndef __TAN_SRC_PARSER_PARSED_MODULE_H__
#define __TAN_SRC_PARSER_PARSED_MODULE_H__
#include "base.h"

namespace tanlang {
class Program;
class SourceManager;

/**
 * \brief Parsed module holds all the information we know about a single tan source file after parsing.
 */
struct ParsedModule {
  ParsedModule() = default;
  ParsedModule(Program *program, SourceManager *sm, str filename, str package_name);

  Program *_program = nullptr;
  SourceManager *_sm = nullptr;
  str _filename;
  str _package_name;
};

} // namespace tanlang

#endif //__TAN_SRC_PARSER_PARSED_MODULE_H__
