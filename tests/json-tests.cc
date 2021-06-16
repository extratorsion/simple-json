#include <gtest/gtest.h>
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
    "inner": { "ints": [1, 2, 43 , 5]}, 
    "int": 20,
    "contain-bool": { "bools" : [true ,  false, true , true, true] }
        } 
  
  )";

  Json json(simple_json);
  EXPECT_TRUE(json.valid());

  EXPECT_TRUE(!json["non_exists"]["value"].has_value());

  EXPECT_STREQ(json["{}"]->toString().data(), "paar");
  EXPECT_EQ(json["int"]->toInt(), 20);
  EXPECT_EQ(int(json["inner"]["ints"]->toList().size()), 4);
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
  };

  for (const auto& invlaid_json: invalid_json_list) {
    Json json(invlaid_json);
    EXPECT_FALSE(json.valid());
  }

  Json bool_json("{\"value\": true}");
  EXPECT_TRUE(bool_json.valid());
  EXPECT_TRUE(bool_json["value"]->toBool());
}
