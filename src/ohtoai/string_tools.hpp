#pragma once
#include <vector>
#include <string>
#include <cctype>
#include <locale>

namespace ohtoai
{
	namespace tool
	{
		namespace string
		{
			std::vector<std::string> split(const std::string& s, const std::string& delimiters)
			{
				std::vector<std::string> tokens;
				std::string::size_type lastPos = s.find_last_not_of(delimiters, 0);
				std::string::size_type pos = s.find_first_of(delimiters, lastPos);
				while (std::string::npos != pos || std::string::npos != lastPos)
				{
					tokens.push_back(s.substr(lastPos, pos - lastPos));
					lastPos = s.find_first_not_of(delimiters, pos);
					pos = s.find_first_of(delimiters, lastPos);
				}
				return tokens;
			}

			std::string ltrimmed(const std::string& s) {
				return { std::find_if(s.begin(), s.end(), [](int ch) {
					return !std::isspace(ch);
					}) , s.end() };
			}

			std::string rtrimmed(const std::string& s) {
				return { s.begin(), std::find_if(s.rbegin(), s.rend(), [](int ch) {
					return !std::isspace(ch);
					}).base() };
			}

			std::string trimmed(const std::string& s)
			{
				return ltrimmed(rtrimmed(s));
			}

			bool start_with(const std::string& s1, const std::string& s2)
			{
				return s1.size() >= s2.size() && s1.compare(0, s2.size(), s2) == 0;
			}

			bool end_with(const std::string& s1, const std::string& s2)
			{
				return s1.size() >= s2.size() && s1.compare(s1.size() - s2.size(), s2.size(), s2) == 0;
			}

			std::string& transform_to_lower(std::string& s)
			{
				std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return std::tolower(ch); });
				return s;
			}

			std::string& transform_to_upper(std::string& s)
			{
				std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return std::toupper(ch); });
				return s;
			}
			
			std::string to_lower(const std::string& s)
			{
				std::string result{ s };
				std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) { return std::tolower(ch); });
				return result;
			}

			std::string to_upper(const std::string& s)
			{
				std::string result{ s };
				std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) { return std::toupper(ch); });
				return result;
			}

			bool contains(const std::string& s1, const std::string& s2)
			{
				return s1.find(s2) != std::string::npos;
			}
		}
	}
}