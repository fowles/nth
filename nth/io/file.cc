#include "nth/io/file.h"

#include <cstdio>

#include "nth/debug/trace.h"

namespace nth {

size_t file::tell() const { return std::ftell(file_ptr_); }

void file::rewind() { std::rewind(file_ptr_); }

bool file::close() {
  if (std::fclose(file_ptr_) == 0) {
    file_ptr_ = nullptr;
    return true;
  } else {
    return false;
  }
}

file& file::operator=(file&& f) {
  NTH_ASSERT((v.harden), file_ptr_ == nullptr);
  file_ptr_ = std::exchange(f.file_ptr_, nullptr);
  return *this;
}

file::~file() { NTH_ASSERT((v.harden), file_ptr_ == nullptr); }

file TemporaryFile() {
  std::FILE* f = std::tmpfile();
  NTH_ASSERT((v.harden), f != nullptr);
  return file(f);
}

}  // namespace nth