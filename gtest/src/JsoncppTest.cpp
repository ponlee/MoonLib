#include <iostream>

#include "gtest/gtest.h"

#include <json/json.h>

using namespace std;

TEST(Jsoncpp, test1)
{
    Json::Value json;
    Json::Reader reader;
   // cout << reader.parse("{\"chart\":{},\"title\":{\"text\":\"Combination chart\"},\"xAxis\":{\"categories\":[\"小肥春\",\"Oranges\",\"Pears\",\"Bananas\",\"Plums\"]},\"tooltip\":{\"formatter\":function(){var s;if(this.point.name){s=\"\"+this.point.name+\": \"+this.y+\" fruits\"}else{s=\"\"+this.x+\": \"+this.y}return s}},\"labels\":{\"items\":[{\"html\":\"Total fruit consumption\",\"style\":{\"left\":\"40px\",\"top\":\"8px\",\"color\":\"black\"}}]}}", json);
    cout << reader.parse("{\"uin1\":\"123\"}", json);

    //Json::Value tempJson = json["xAxis"]["categories"];
    if (json["uin1"].isString())
    {
        cout << json["uin"].asString() << endl;
    }
    cout << "2" << endl;

    //EXPECT_EQ(5, tempJson.size());
}