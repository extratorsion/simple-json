#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP
#endif

#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <type_traits>

#include "simple_json_utils.h"

namespace json {

using std::move;
using std::string;
using std::string_view;

struct JsonNode;
class Json;


template <
    typename T,
    typename Checker = typename std::enable_if<std::is_same_v<typename std::remove_const_t<T>, JsonNode>, T>::type> 
    struct  JsonNodeRef {

  JsonNodeRef(T* node=nullptr)
    :node_(node)
  {}

  bool has_value() const {
    return node_ != nullptr;
  }

  T* value() {
    return node_;
  }

  const T* operator->() const {
    return node_;
  }

  operator bool() const {
    return has_value();
  }

  T* operator->() {
    return node_;
  }

 template <typename Key>
  JsonNodeRef operator[](const Key& key) {
    if (has_value()) {
      return (*node_)[key];
    } else {
      return JsonNodeRef();
    }
  }

  template <typename Key>
  JsonNodeRef<const JsonNode> at(const Key& key) const {
    if (has_value()) {
      return (*node_).at(key);
    } else {
      return JsonNodeRef<const JsonNode>();
    }
  }

private:
  T* node_ = nullptr;
};

struct JsonNode {
  using ObjType = std::map<string, JsonNode>;
  using ListType = std::vector<JsonNode>;
  using StringType = string;
  enum Type { Obj, List, /* String, */ OwnedString, Int, Float, Bool, Error };

  template <typename T>
  static void Reconstruct(JsonNode* node, T&& value) {
    node->~JsonNode();
    new(node)JsonNode(move(value));
  }

  void insert(string key, JsonNode&& value_node) {
    asObj().insert({move(key), move(value_node)});
  }

  void push(JsonNode&& value_node) {
    asList().push_back(move(value_node));
  }
  JsonNode() { type_ = Error; }

  JsonNode(JsonNode&& rhs) : type_(rhs.type_), data_(move(rhs.data_)) {
    rhs.type_ = Error;
  }

  JsonNode& operator=(JsonNode&& rhs) {
    type_ = rhs.type_;
    data_ = move(rhs.data_);
    rhs.type_ = Error;
    return *this;
  }

  JsonNode(const char* str) : type_(OwnedString), data_(string(str)) {}

  JsonNode(string str) : type_(OwnedString), data_(move(str)) {}

  JsonNode(ObjType&& objs) : type_(Obj), data_(move(objs)) {}

  JsonNode(int value) : type_(Int), data_(value) {}

  JsonNode(double value) : type_(Float), data_(value) {}

  JsonNode(bool value) : type_(Bool), data_(value) {}

  JsonNode(ListType&& list) : type_(List), data_(move(list)) {}

  JsonNode(const JsonNode& rhs) : type_(rhs.type_), data_(rhs.data_) {}

  JsonNode& operator=(const JsonNode& rhs) {
    type_ = rhs.type_;
    data_ = rhs.data_;
    return *this;
  }

  ~JsonNode() { type_ = Error; }

  JsonNode clone() { return *this; }

  JsonNodeRef<const JsonNode> at(size_t index) const {
    if (type_ == List) {
      return {&std::get<List>(data_).at(index)};
    } else {
      return {};
    }
  }

  JsonNodeRef<const JsonNode> at(const string& key) const {
    if (type_ == Obj && std::get<Obj>(data_).count(key)) {
      return {&std::get<Obj>(data_).at(key)};
    } else {
      return {};
    }
  }

  JsonNodeRef<JsonNode> operator[](size_t index) {
    if (type_ == List) {
      return {&std::get<List>(data_)[index]};
    } else {
      return {};
    }
  }

  JsonNodeRef<JsonNode> operator[](const string& key) {
    if (type_ == Obj && std::get<Obj>(data_).count(key)) {
      return {&std::get<Obj>(data_)[key]};
    } else {
      return {};
    }
  }

  double toFloat() const {
    if (type_ == Float) {
      return std::get<Float>(data_);
    } else {
      return 0.;
    }
  }

  int toInt() const {
    if (type_ == Int) {
      return std::get<Int>(data_);
    } else {
      return 0;
    }
  }

  bool toBool() const {
    if (type_ == Bool) {
      return std::get<Bool>(data_);
    } else {
      return false;
    }
  }

  std::vector<const JsonNode*> toList() const {
    if (type_ == List) {
      const auto& list_ref = std::get<ListType>(data_);
      std::vector<const JsonNode*> node_lists;
      node_lists.reserve(list_ref.size());
      for (const auto& node : list_ref) {
        node_lists.push_back(&node);
      }
      return node_lists;
    } else {
      return {};
    }
  }

  string toString() const {
    if (type_ == OwnedString) {
      return std::get<OwnedString>(data_);
    } else {
      return "";
    }
  }

  Type type() const { return type_; }

  bool isType(Type type) const { return type_ == type; }
  bool isString() const { return type_ == OwnedString; }
  bool isNumber() const { return type_ == Int || type_ == Float; }
  bool isObj() const { return type_ == Obj; }
  bool isList() const { return type_ == List; }
  bool isInt() const { return type_ == Int; }
  bool isFloat() const { return type_ == Float; }
  bool isBool() const { return type_ == Bool; }

  string str() const {
    if (type_ == Error) return "";
    string builder;
    bool first = true;
    switch (type_) {
      case Obj:
        builder.push_back('{');
        for (auto& [key, value] : std::get<Obj>(data_)) {
          if (first) {
            first = false;
          } else {
            builder.append(", ");
          }
          builder.push_back('"');
          builder.append(EscapeJson(key));
          builder.append("\": ");
          builder.append(value.str());
        }
        builder.push_back('}');
        break;
      case List:
        builder.push_back('[');
        for (auto& node : std::get<List>(data_)) {
          if (first) {
            first = false;
          } else {
            builder.append(", ");
          }
          builder.append(node.str());
        }
        builder.push_back(']');
        break;
      case OwnedString:
        builder.push_back('"');
        builder.append(EscapeJson(std::get<OwnedString>(data_)));
        builder.push_back('"');
        break;
      case Int:
        builder.append(std::to_string(std::get<Int>(data_)));
        break;
      case Float:
        builder.append(std::to_string(std::get<Float>(data_)));
        break;
      case Bool:
        builder.append(std::get<Bool>(data_) ? "true": "false");
        break;
      default:
        break;
    }
    return builder;
  }

 protected:
  ObjType& asObj() {
    type_ = Obj;
    if (ObjType* v = std::get_if<ObjType>(&data_)) {
      return *v;
    } else {
      data_ = ObjType();
    }
    return std::get<ObjType>(data_);
  }

  ListType& asList() {
    type_ = List;
    if (ListType* v = std::get_if<ListType>(&data_)) {
      return *v;
    } else {
      data_ = ListType();
    }
    return std::get<ListType>(data_);
  }

  StringType& asString() {
    type_ = OwnedString;
    if (StringType* v = std::get_if<StringType>(&data_)) {
      return *v;
    } else {
      data_ = StringType();
    }
    return std::get<StringType>(data_);
  }

  int& asInt() {
    type_ = Int;
    if (int* v = std::get_if<int>(&data_)) {
      return *v;
    } else {
      data_ = int();
    }
    return std::get<int>(data_);
  }

  double& asFloat() {
    type_ = Float;
    if (double* v = std::get_if<double>(&data_)) {
      return *v;
    } else {
      data_ = double();
    }
    return std::get<double>(data_);
  }

  bool& asBool() {
    type_ = Bool;
    if (bool* v = std::get_if<bool>(&data_)) {
      return *v;
    } else {
      data_ = bool();
    }
    return std::get<bool>(data_);
  } 

 protected:
  Type type_;
  std::variant<ObjType, ListType, StringType, int, double, bool> data_;
  friend class Json;
};

#define BREAK_ON_FAILED(expr) \
  if (!(expr)) {              \
    valid_ = false;           \
    break;                    \
  }


class Json {
 public:
 public:
  Json(string str) : raw_str_(move(str)) { valid_ = parse(raw_str_, &root_); }
  bool valid() { return valid_; }
  JsonNodeRef<JsonNode> operator[](const string& key) { return root_[key]; }
  JsonNodeRef<const JsonNode> at(const string& key) { return root_.at(key); }
  string str() const { return root_.str(); }

 private:
  using cstr_t = string_view::const_iterator;
  static void PrintView(string_view view) {
    for (const char ch: view) {
      putchar(ch);
    }
    putchar('\n');
  }

  bool parse(const string& str, JsonNode* root) {
    if (str.size() == 0) {
      return false;
    }
    auto view = Trim(str);
    // PrintView(view);
    return parseObj(view.begin(), view.end(), root);
  }

  static string_view Trim(const string& str) {
    string_view view(str);
    
    while(view.size() > 0 && std::isspace(view.front())) {
      view.remove_prefix(1);
    }
    while(view.size() > 0 && std::isspace(view.back())) {
      view.remove_suffix(1);
    }
    
    return view;
  }

  static string_view MakeView(cstr_t start, cstr_t finish) {
    if (finish > start) {
      return string_view(&*start, finish - start);
    } else {
      return string_view();
    }
  }

  static string View2String(string_view view) {
    return string(view.begin(), view.end());
  }

  bool parseObj(cstr_t start, cstr_t finish, JsonNode* root) {
    if (finish == findPairEdge(start, finish, '{', '}')) {
      return false;
    }
    auto& obj_map = root->asObj();

    cstr_t p_cur = start + 1;

    string_view last_key;

    JsonNode last_node;

    for (; p_cur < finish - 1;) {
      if (std::isspace(p_cur[0])) {
        ++p_cur;
        continue;
      }
      if (*p_cur == '"') {
        cstr_t key_edge = parseString(p_cur, finish);
        BREAK_ON_FAILED(key_edge != finish);
        last_key = MakeView(p_cur + 1, key_edge);
        p_cur = key_edge + 1;
      } else if (*p_cur == ':') {
		    p_cur = nextNoneSpacePos(p_cur + 1, finish); 
		    BREAK_ON_FAILED(p_cur != finish);
        p_cur = parseValue(p_cur, finish, &last_node);
			  BREAK_ON_FAILED(p_cur != finish);
        ++p_cur;
      } else if (*p_cur == ',') {
        obj_map.insert({string(last_key), move(last_node)});
        ++p_cur;
      } else {
        BREAK_ON_FAILED(false);
      }
    }

    if (valid_ && last_key.size() && !last_node.isType(JsonNode::Error)) {
      obj_map.insert({string(last_key), move(last_node)});
    } else {
      root->type_ = JsonNode::Error;
      valid_ = false;
    }

    return valid_;
  }

  cstr_t nextBlockPos(cstr_t start, cstr_t finish,
                      std::function<bool(const char)> pred) {
    while (start < finish) {
      if (pred(*start)) {
        return start;
      } else {
        ++start;
      }
    }
    return finish;
  }

  cstr_t nextNoneSpacePos(cstr_t start, cstr_t finish) {
    while (*start && start < finish) {
      if (!std::isspace(*start)) {
        return start;
      } else {
        ++start;
      }
    }
    return finish;
  }

  cstr_t parseValue(cstr_t start, cstr_t finish, JsonNode* value_node) {
    if (start >= finish) return finish;
    cstr_t p_cur = start;
    switch (*p_cur) {
      case '"':
        if (auto str_edge = parseString(p_cur, finish); str_edge != finish) {
          string value = View2String(MakeView(p_cur + 1, str_edge));
          JsonNode::Reconstruct(value_node, move(value));
          return str_edge;
        }
        return finish;
        break;
      case '[':
          if (auto edge = findPairEdge(p_cur, finish, '[', ']'); edge != finish) {
          if (parseInList(p_cur, edge + 1, value_node)) {
            return edge;
          }
        }
        return finish;
        break;
      case '{':
          if (auto edge = findPairEdge(p_cur, finish, '{', '}'); edge != finish) {
          if (parseObj(p_cur, edge + 1, value_node)) {
            return edge;
          }
        }
        return finish;
        break;
      default:
          if (auto val_edge = nextBlockPos(p_cur, finish, [](const char ch) {
              return ch == '}' || ch == '"' || ch == '[' || ch == ']' ||
                  ch == '{' || ch == ',' || std::isspace(ch);
              }); val_edge != finish) {
          string_view val_view = MakeView(p_cur, val_edge);
          if (std::regex_match(val_view.begin(), val_view.end(), BoolPat)) {
            string bool_val(val_view.begin(), val_view.end());
            value_node->asBool() = (bool_val == "true" ? true : false);
            return val_edge - 1;
          } else {
            string num_str(val_view);
            if (std::regex_match(num_str, IntPat)) {
              value_node->asInt() = str2int(num_str);
              return val_edge - 1;
            } else if (std::regex_match(num_str, FloatPat)) {
              value_node->asFloat() = atof(num_str.data());
              return val_edge - 1;
            }           
          }
        }
        return finish;
        break;
    }
  }

  bool parseInList(cstr_t start, cstr_t finish, JsonNode* node) {
    node->type_ = JsonNode::List;
    auto& node_list = node->asList();
    auto list_view = MakeView(start, finish);

    if (std::regex_match(list_view.begin(), list_view.end(), ObjListPat)) {
      std::match_results<std::string_view::const_iterator> obj_match;

      std::vector<string_view> obj_list;
      while (std::regex_search(list_view.begin(), list_view.end(), obj_match,
                               ObjItemPat)) {
        auto matched = obj_match[1];
        obj_list.push_back(matched.str());
        list_view = MakeView(matched.second, list_view.end());
      }

      for (const auto& obj : obj_list) {
        JsonNode obj_node;
        if (parseObj(obj.cbegin(), obj.cend(), &obj_node)) {
          node_list.push_back(move(obj_node));
        } else {
          node->type_ = JsonNode::Error;
          return false;
        }
      }
      return true;
    } else if (std::regex_match(list_view.begin(), list_view.end(),
                                StringListPat)) {
      list_view.remove_prefix(1);
      list_view.remove_suffix(1);

      cstr_t str_start = finish, str_finish = finish;
      bool in_string = false;
      bool view_changed = false;

      while (list_view[0] != '"') list_view.remove_prefix(1);

      cstr_t p_edge = finish;
      for (p_edge = findPairEdge(list_view.begin(), list_view.end(), '"', '"'); p_edge != list_view.end();) {
        auto str_view = MakeView(list_view.begin() + 1, p_edge);
        JsonNode str_node;
        stringNodeHandle(str_view, &str_node);
        node_list.push_back(move(str_node));
        list_view = MakeView(p_edge + 1, list_view.end());
      }

      return true;
    } else if (std::regex_match(list_view.begin(), list_view.end(),
                                IntListPat)) {
      list_view.remove_prefix(1);
      list_view.remove_suffix(1);
      std::vector<string_view> num_vec = SplitString<string_view>(list_view, ',');

      for (const auto& int_num : num_vec) {
        JsonNode int_node;
        string int_str(int_num);
        int val = str2int(int_str);
        int_node.asInt() = val;
        node_list.push_back(move(int_node));
      }
      return true;
    } else if (std::regex_match(list_view.begin(), list_view.end(),
                                FloatListPat)) {
      list_view.remove_prefix(1);
      list_view.remove_suffix(1);
      std::vector<string_view> num_vec = SplitString<string_view>(list_view, ',');

      for (const auto& float_num : num_vec) {
        JsonNode float_node;
        string float_str(float_num);
        double val = atof(float_num.data());
        float_node.asFloat() = val;
        node_list.push_back(move(float_node));
      }
      return true;
    } else if (std::regex_match(list_view.begin(), list_view.end(), BoolListPat)) {
      list_view.remove_prefix(1);
      list_view.remove_suffix(1);

      std::vector<string_view> num_vec = SplitString<string_view>(list_view, ',');

      for (const auto& bool_val : num_vec) {
        JsonNode bool_node;
        bool val = (string(bool_val) == "true" ? true : false);
        bool_node.asBool() = val;
        node_list.push_back(move(bool_node));
      }
      return true;

    }
    node->type_ = JsonNode::Error;
    return false;
  }

  cstr_t parseString(cstr_t start, cstr_t finish) {
    return findPairEdge(start, finish, '"', '"');
  }

  cstr_t findStrEdge(cstr_t start, cstr_t finish, bool ignore_espace = true) {
    cstr_t p_cur = start;
    if (*p_cur != '"') {
      return finish;
    }

    for (++p_cur; p_cur < finish; ++p_cur) {
      if (*p_cur == '\\') {
        ++p_cur;
      } else if (*p_cur == '"') {
        return p_cur;
      }
    }
    return finish;
  }

  cstr_t findPairEdge(cstr_t start, cstr_t finish, char open, char close,
                      bool ignore_espace = true) {
    if (open == '"' && open == close) {
      return findStrEdge(start, finish, ignore_espace);
    }

    std::stack<char> stack;
    auto p_cur = start;
    if (p_cur >= finish || *p_cur != open) {
      return finish;
    }

    stack.push(open);

    ++p_cur;
    for (; p_cur < finish;) {
      if (*p_cur == '"') {
          if (p_cur = findStrEdge(p_cur, finish, ignore_espace); p_cur == finish) {
          return finish;
        }
      } else if (*p_cur == close) {
        stack.pop();
      } else if (*p_cur == open) {
        stack.push(open);
      }
      if (stack.size() == 0) {
        return p_cur;
      }
      ++p_cur;
    }
    return finish;
  }

  void stringNodeHandle(const string_view& str_view, JsonNode* str_node) {
    auto& node_str = str_node->asString();
    if (needTranslate(str_view)) {
      node_str = str_view;
    } else {
      node_str = translate(str_view);
    }
  }

  bool needTranslate(const string_view& view) {
    for (const auto c : view) {
      if (c == '\\') {
        return false;
      }
    }
    return true;
  }

  string translate(const string_view& view) {
    string builder;
    for (auto c = view.begin(); c != view.end(); ++c) {
      if (*c != '\\') {
        builder.push_back(*c);
        continue;
      }
      if (++c == view.end()) {
        return builder;
      }
      switch (*c) {
        case '\\':
          builder.push_back('\\');
          break;
        case 'b':
          builder.push_back('\b');
          break;
        case 'f':
          builder.push_back('\f');
          break;
        case 'n':
          builder.push_back('\n');
          break;
        case 'r':
          builder.push_back('\r');
          break;
        case '"':
          builder.push_back('"');
          break;
        case 't':
          builder.push_back('\t');
          break;
      }
    }
    return builder;
  }

  JsonNode root_;
  string raw_str_;
  bool valid_ = true;
};

}  // namespace json
