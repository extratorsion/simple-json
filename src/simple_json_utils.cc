#include "simple_json_utils.h"

#include <iomanip>
#include <sstream>

namespace json {

using std::string;

const auto DefaultRegOpt = std::regex_constants::ECMAScript;

#define _FLOAT_PAT "([+-]?([0-9]*\\.[0-9]+|[0-9]+\\.?[0-9]*(e[+-]?[0-9]+)?))"
#define _INT_PAT "([+-]?([0-9]+|0x[0-9a-f]+|0[0-7]+|0b[01]+))"
#define _STR_PAT "(\\\"|\S)*"
#define _OBJ_PAT "\\{.*\\}"

const std::regex FloatPat({"(" _FLOAT_PAT ")"}, std::regex_constants::icase);
const std::regex IntPat({"(" _INT_PAT ")"}, std::regex_constants::icase);
const std::regex BoolPat("(true|false)");

const std::regex StringListPat({"\\["
                                "(\\s*" _STR_PAT "\\s*,)"
                                "\\s*" _STR_PAT "\\s*\\]"},
                               DefaultRegOpt);

const std::regex IntListPat({"\\["
                             "(\\s*" _INT_PAT "\\s*,)*"
                             "\\s*" _INT_PAT "\\s*\\]"},
                            DefaultRegOpt | std::regex_constants::icase);

const std::regex FloatListPat({"\\["
                               "(\\s*" _FLOAT_PAT "\\s*,)*"
                               "\\s*" _FLOAT_PAT "\\s*\\]"},
                              DefaultRegOpt | std::regex_constants::icase);

const std::regex BoolListPat(R"(\[(\s*(true|false)\s*,)*\s*(true|false)\s*\])",
                             DefaultRegOpt);

const std::regex ObjListPat({"\\["
                             "(\\s*" _OBJ_PAT "\\s*,)*"
                             "\\s*" _OBJ_PAT "\\s*"
                             "\\]"},
                            DefaultRegOpt);

const std::regex ObjItemPat({"(" _OBJ_PAT ")"}, DefaultRegOpt);

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

int str2int(const string& str) {
  if (str.empty()) {
    return 0;
  } 

  int have_sign = int(str[0] == '+' || str[0] == '-');

  if (str.size() > 1 && str[have_sign] == '0') {
    std::istringstream iss(str);
    int value = 0;
    char base = std::tolower(str[1 + have_sign]);
    if (base == 'x') {
      iss >> std::hex >> value;
    } else if (base == 'b') {
      for (int i = 0; i < str.size() - 2 - have_sign; ++i) {
        value |= ((str[str.size() - i - 1] - '0') << i);
      }
      if (have_sign && str[0] == '-') {
        value = -value;
      }
    } else {
      iss >> std::oct >> value;
    }
    return value;
  } else {
    return atoi(str.data());
  }
}

}  // namespace json
