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

			// trim from start(in place)
			std::string ltrimmed(const std::string& s) {
				return { std::find_if(s.begin(), s.end(), [](int ch) {
					return !std::isspace(ch);
					}) , s.end() };
			}

			// trim from end(in place)
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
		}
	}
}
