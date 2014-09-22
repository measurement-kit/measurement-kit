/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Light testing to make sure the embedded JsonCpp works
//

#include "src/ext/Catch/single_include/catch.hpp"

#include <jsoncpp/json/json.h>

#include <iostream>

TEST_CASE("We can parse a simple JSON document", "[JsonCpp]") {
	std::string document = "{\"key\":314}";
	Json::Value root;
	Json::Reader reader;
	REQUIRE(reader.parse(document, root));
	REQUIRE(root["key"] == 314);
}

TEST_CASE("We can write a simple JSON document", "[JsonCpp]") {
	Json::Value root;
	Json::FastWriter writer;
	root["key"] = 314;
	REQUIRE(writer.write(root) == "{\"key\":314}\n");
}
