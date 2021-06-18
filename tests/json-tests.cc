#include <gtest/gtest.h>
#include <vector>
#include "simple_json.hpp"

TEST(SimpleJson, Parse) {
  using namespace std;
  using namespace json;
  string simple_json = R"({
                "na\tme": "obj1", "obj": {
                        "1": 1,
                        "2": 2
                },
    "{}": "paar",
    "inner": { "ints": [0b101, 2, +43 , -5, 034, -0X33],
    "doubles": [-2e-20, -.4, .0, 23.3, +4., 45.23e2]
    }, 
    "int": 20,
    "contain-bool": { "bools" : [true ,  false, true , true, true] }
        } 
  
  )";

  Json json(simple_json);
  EXPECT_TRUE(json.valid());

  EXPECT_TRUE(!json["non_exists"]["value"].has_value());

  EXPECT_STREQ(json["{}"]->toString().data(), "paar");
  EXPECT_EQ(json["int"]->toInt(), 20);

  EXPECT_EQ(int(json["inner"]["ints"]->toList().size()), 6);
  EXPECT_EQ(json["inner"]["ints"][0]->toInt(), 0b101);
  EXPECT_EQ(json["inner"]["ints"][1]->toInt(), 2);
  EXPECT_EQ(json["inner"]["ints"][2]->toInt(), 43);
  EXPECT_EQ(json["inner"]["ints"][3]->toInt(), -5);
  EXPECT_EQ(json["inner"]["ints"][4]->toInt(), 28);
  EXPECT_EQ(json["inner"]["ints"][5]->toInt(), -0x33);

  EXPECT_EQ(int(json["inner"]["doubles"]->toList().size()), 6);

  EXPECT_DOUBLE_EQ(json["inner"]["doubles"][0]->toFloat(), -2e-20);
  EXPECT_DOUBLE_EQ(json["inner"]["doubles"][1]->toFloat(), -0.4);
  EXPECT_DOUBLE_EQ(json["inner"]["doubles"][2]->toFloat(), 0.);
  EXPECT_DOUBLE_EQ(json["inner"]["doubles"][3]->toFloat(), 23.3);
  EXPECT_DOUBLE_EQ(json["inner"]["doubles"][4]->toFloat(), 4.0);
  EXPECT_DOUBLE_EQ(json["inner"]["doubles"][5]->toFloat(), 45.23e2);


  EXPECT_EQ(int(json["contain-bool"]["bools"]->toList().size()), 5);
  EXPECT_EQ(json["contain-bool"]["bools"][1]->toBool(), false);
  EXPECT_EQ(json["contain-bool"]["bools"][2]->toBool(), true);
  EXPECT_EQ(json["inner"]["ints"][2]->toInt(), 43);

  vector<string> invalid_json_list = {
    R"({"34^"})",
    R"({"34^":})",
    R"({"34^": *})",
    R"({"34^"* *})",
    R"({"34^": [})",
    R"({"34": 34, }})",
    R"({"34": 34,})",
  };

  for (const auto& invlaid_json: invalid_json_list) {
    Json json(invlaid_json);
    EXPECT_FALSE(json.valid());
  }

  Json bool_json("{\"value\": true}");
  EXPECT_TRUE(bool_json.valid());
  EXPECT_TRUE(bool_json["value"]->toBool());
}


TEST(SimpleJson, Generater) {
  using namespace std;
  using namespace json;

  JsonNode vec_json;
  vec_json.insert("name", JsonNode("generator"));
  auto ints = [] {
    vector<JsonNode> tmp;
    for (int i = 1; i <= 10; ++i) {
      tmp.push_back(JsonNode(i));
    }
    return tmp;
  }();
  vec_json.insert("numbers", move(ints));
  vec_json["numbers"]->push(JsonNode(450));

  const char* gen_str =
      R"({"name": "generator", "numbers": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 450]})";
  EXPECT_STREQ(vec_json.str().data(), gen_str);
}