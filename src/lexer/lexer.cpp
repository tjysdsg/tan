#include "lexer.h"
#include <include/reader.h>
#include "base.h"

namespace tanlang {
    std::vector<token *> tokenize(const line_info *const line,
                                  std::vector<rule> rules) {
        std::vector<token *> tokens;

        long start = 0;
        long end = line->code.size();
        size_t line_len = line->code.length();
        std::string match;
        while (start != end) {
            for (size_t i = 0; i < rules.size(); ++i) {
                const auto &r = rules[i];
                std::smatch results;
                auto str = line->code.substr(start, line_len - start + 1);
                if (std::regex_match(str, results, std::get<0>(r))) {
                    // TODO: cache regex results instead of only using the first
                    // match
                    match = results[0];
                    start = results.length(0) + results.position(0);
                    auto *t = new token(std::get<1>(r), match);
                    tokens.emplace_back(t);
                    break;
                } else {
                    ++start;
                }
                // if nothing matched
                if (i >= rules.size() - 1) {
                    report_code_error(line->code, line->lineno, start,
                                      "Syntax error");
                    exit(1);
                }
            }
        }
        return tokens;
    }
} // namespace tanlang
