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