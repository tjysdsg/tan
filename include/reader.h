#ifndef __TAN_READER_READER_H__
#define __TAN_READER_READER_H__
#include <string>
namespace tanlang {

class Reader final {
public:
    Reader() = default;
    ~Reader() = default;

private:
    void open(const std::string& file_name);
    bool set_encoding(const std::string& encoding);
    std::string next_line();
    std::string get_line(size_t idx);
    unsigned get_line_number() const;
    std::string get_filename() const;
    bool eof() const;
};

}  // namespace tanlang

#endif  // __TAN_READER_READER_H__
