#include <string_view>
#include <vector>

#include "nth/commandline/commandline.h"
#include "nth/debug/log/log.h"
#include "nth/debug/log/file_log_sink.h"

int main(int argc, char const* argv[]) {
  nth::register_log_sink(nth::stderr_log_sink);

  std::vector<std::string_view> arguments;
  for (int i = 0; i < argc; ++i) {
    if (argv[i] == nullptr) { continue; }
    arguments.push_back(argv[i]);
  }

  return nth::InvokeCommandline(arguments).code();
}
