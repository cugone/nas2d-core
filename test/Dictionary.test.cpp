#include "NAS2D/Dictionary.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>


TEST(Dictionary, setGet) {
	NAS2D::Dictionary dictionary;

	// Set some test values
	dictionary.set("Key1", "Some string value");
	dictionary.set("Key2", std::string{"Another string value"});
	dictionary.set("Key3", true);
	dictionary.set("Key4", 1);

	// Read back typed values
	EXPECT_EQ("Some string value", dictionary.get("Key1"));
	EXPECT_EQ("Another string value", dictionary.get("Key2"));
	EXPECT_EQ(true, dictionary.get<bool>("Key3"));
	EXPECT_EQ(1, dictionary.get<int>("Key4"));

	EXPECT_THROW(dictionary.get("KeyDoesNotExist"), std::out_of_range);
}

TEST(Dictionary, keys) {
	NAS2D::Dictionary dictionary;

	// Set some test values
	dictionary.set("Key1", "Some string value");
	dictionary.set("Key2", std::string{"Another string value"});
	dictionary.set("Key3", true);
	dictionary.set("Key4", 1);

	EXPECT_EQ((std::vector<std::string>{"Key1", "Key2", "Key3", "Key4"}), dictionary.keys());
}

TEST(Dictionary, OperatorAdd) {
	// Simple combination
	{
		NAS2D::Dictionary dictionary1;
		NAS2D::Dictionary dictionary2;

		dictionary1.set("Key1", 1);
		dictionary2.set("Key2", 2);

		const auto dictionaryCombined = dictionary1 + dictionary2;

		EXPECT_EQ(1, dictionaryCombined.get<int>("Key1"));
		EXPECT_EQ(2, dictionaryCombined.get<int>("Key2"));
	}

	// Right hand side overwrites left hand side
	{
		NAS2D::Dictionary dictionary1;
		NAS2D::Dictionary dictionary2;

		dictionary1.set("Key1", 1);
		dictionary1.set("Key2", 2);
		dictionary2.set("Key2", 10);
		dictionary2.set("Key3", 20);

		const auto dictionaryCombined = dictionary1 + dictionary2;

		EXPECT_EQ(1, dictionaryCombined.get<int>("Key1"));
		EXPECT_EQ(10, dictionaryCombined.get<int>("Key2"));
		EXPECT_EQ(20, dictionaryCombined.get<int>("Key3"));
	}
}