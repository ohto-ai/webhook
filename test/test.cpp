#include "pch.h"
#include "../ohtoai/string_tools.hpp"

TEST(OhtoAi_StringTool, TestSplit) {
	using ohtoai::tool::string::split;
	{
		auto result = split("a,b,c", ",");
		testing::internal::Strings expected = { "a", "b", "c" };
		EXPECT_EQ(result, expected);
	}
	{
		auto result = split("a b c", " ");
		testing::internal::Strings expected = { "a", "b", "c" };
		EXPECT_EQ(result, expected);
	}
	{
		auto result = split("a b,c", " ,");
		testing::internal::Strings expected = { "a", "b", "c" };
		EXPECT_EQ(result, expected);
	}
}

TEST(OhtoAi_StringTool, TestTrimmed) {
	using ohtoai::tool::string::trimmed;
	{
		auto result = trimmed(" a b c    ");
		EXPECT_EQ(result, "a b c");
	}
	{
		auto result = trimmed("\n\r\ta b c\t \n \r ");
		EXPECT_EQ(result, "a b c");
	}
}

TEST(OhtoAi_StringTool, TestStartWith) {
	using ohtoai::tool::string::start_with;
	EXPECT_TRUE(start_with("a b c", "a"));
	EXPECT_TRUE(start_with("a\nwww", "a\n"));
	EXPECT_TRUE(start_with("abv", "abv"));
	EXPECT_FALSE(start_with("abv", "abv "));
}

TEST(OhtoAi_StringTool, TestEndWith) {
	using ohtoai::tool::string::end_with;
	EXPECT_TRUE(end_with("a b c", "c"));
	EXPECT_TRUE(end_with("a b \n\tc", "\tc"));
	EXPECT_TRUE(end_with("a b c", "a b c"));
	EXPECT_FALSE(end_with("a b c", " a b c"));
}