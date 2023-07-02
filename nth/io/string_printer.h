#ifndef NTH_IO_STRING_PRINTER_H
#define NTH_IO_STRING_PRINTER_H

#include <string>
#include <string_view>

namespace nth {

struct string_printer {
  explicit constexpr string_printer(std::string& s) : s_(s) {}
  void write(size_t n, char c) { s_.append(n, c); }
  void write(std::string_view s) { s_.append(s); }

 private:
  std::string& s_;
};

}  // namespace nth

#endif  // NTH_IO_STRING_PRINTER_H
