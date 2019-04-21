#ifndef __TAN_LEXER_LEXER_H__
#define __TAN_LEXER_LEXER_H__
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "reader.h"
#include "config.h"

namespace tanlang {

    // forward declarations
    enum TOKEN_TYPE : uint64_t;
    class Reader;
    struct token_info;

    class Lexer final {
      public:
        Lexer();
        ~Lexer();
        void open(const std::string &file_name);
        void lex();
        token_info *next_token() const;
        token_info *get_token(const unsigned idx) const;
#ifdef DEBUG_ENABLED
        void read_string(const std::string &code);
        friend std::ostream &operator<<(std::ostream &os, const Lexer &lexer);
#endif

      private:
        std::vector<token_info *> _token_infos{};
        std::unique_ptr<Reader> _reader;
        mutable unsigned _curr_token = 0;
    };
} // namespace tanlang
#endif // __TAN_LEXER_LEXER_H__
