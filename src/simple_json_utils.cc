#include "simple_json_utils.h"
#include <sstream>
#include <iomanip>

namespace json {

using std::string;

const auto DefaultRegOpt =
    std::regex_constants::ECMAScript;

const std::regex FloatPat("([+-]?[0-9]+\\.[0-9]*|[+-]?[0-9]*\\.[0-9]+)",
                          std::regex_constants::icase);
const std::regex IntPat("([+-]?[0-9]+|0x[0-9a-f]+|0[0-7]+)");

const std::regex BoolPat("(true|false)");

const std::regex StringListPat(R"(\[\s*("(\\"|\S)*"\s*,?\s*)+\])", DefaultRegOpt);

const std::regex IntListPat(
    R"(\[\s*(([+-]?[0-9]+|0x[0-9a-f]+|0[0-7]+)\s*,?\s*)+\])", DefaultRegOpt);

const std::regex FloatListPat(
    R"(\[\s*(([+-]?[0-9]+\.[0-9]*|[+-]?[0-9]*\.[0-9]+)\s*,?\s*)+\])",
    DefaultRegOpt);

const std::regex BoolListPat(R"(\[\s*((true|false)\s*,?\s*)+\])",
                             DefaultRegOpt);

const std::regex ObjListPat(R"(\[\s*(\{.*\}\s*,?\s*)+\])", DefaultRegOpt);

const std::regex ObjItemPat(R"((\{.*\}))", DefaultRegOpt);

auto EscapeJson(const string& raw_str) -> string {
  std::ostringstream oss;
  for (auto c = raw_str.begin(); c != raw_str.end(); ++c) {
    switch (*c) {
      case '\\':
        oss << "\\\\";
        break;
      case '\b':
        oss << "\\b";
        break;
      case '\f':
        oss << "\\f";
        break;
      case '\n':
        oss << "\\n";
        break;
      case '\r':
        oss << "\\r";
        break;
      case '"':
        oss << "\\\"";
        break;
      case '\t':
        oss << "\\t";
        break;
      default:
        if ('\x00' <= *c && *c <= '\x1f') {
          oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
              << static_cast<int>(*c);
        } else {
          oss << *c;
        }
    }
  }
  return oss.str();
}

}