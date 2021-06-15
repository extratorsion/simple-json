#ifndef SIMPLE_JSON_UTILS
#define SIMPLE_JSON_UTILS

#include <string>
#include <vector>
#include <regex>

namespace json {

extern const std::regex FloatPat;
extern const std::regex IntPat;
extern const std::regex StringListPat;
extern const std::regex IntListPat;
extern const std::regex FloatListPat;
extern const std::regex ObjListPat;
extern const std::regex ObjItemPat;

auto EscapeJson(const std::string& raw_str) -> std::string;

template <typename StringT = std::string>
auto SplitString(const StringT& content, char sep) -> std::vector<StringT> {
  // return: start_pos, finish_pos
  static auto strip = [](const char* str,
                         size_t len) -> std::pair<const char*, const char*> {
    if (len <= 0) return {str, str};
    int start = 0, end = int(len) - 1;
    for (; start < int(len); ++start) {
      if (!(str[start] == ' ' || str[start] == '\t' || str[start] == '\n'))
        break;
    }
    for (; end >= 0; --end) {
      if (!(str[end] == ' ' || str[end] == '\t' || str[end] == '\n')) break;
    }
    return {str + start, str + end + 1};
  };

  std::vector<StringT> results;
  const char* cstr = content.data();
  size_t start = 0, finish = 0;
  for (size_t i = 0; i < content.size(); ++i) {
    if (sep == content[i] || i == content.size() - 1) {
      finish = (sep == content[i] ? i : content.size());
      if (start < finish) {
        auto [start_pos, finish_pos] = strip(cstr + start, finish - start);
        if (start_pos && finish_pos && start_pos < finish_pos) {
          results.push_back(StringT(start_pos, finish_pos - start_pos));
        }
      }
      start = i + 1;
    }
  }
  return results;
}

}
#endif