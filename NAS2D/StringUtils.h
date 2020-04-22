// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2019 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgment of your use of NAS2D is appreciated but is not required.
// ==================================================================================
#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace NAS2D
{
	template<typename T>
	std::string stringFrom(T value)
	{
		if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) {
			return value;
		} else if constexpr (std::is_same_v<T, bool>) {
			return value ? "true" : "false";
		} else {
			return std::to_string(value);
		}
	}

	std::string toLowercase(std::string str);
	std::string toUppercase(std::string str);
	std::vector<std::string> split(std::string str, char delim = ',');
	std::vector<std::string> splitSkipEmpty(std::string str, char delim = ',');
	std::pair<std::string, std::string> splitOnFirst(const std::string& str, char delim);
	std::pair<std::string, std::string> splitOnLast(const std::string& str, char delim);
	std::string join(std::vector<std::string> strs);
	std::string join(std::vector<std::string> strs, char delim);
	std::string joinSkipEmpty(std::vector<std::string> strs);
	std::string joinSkipEmpty(std::vector<std::string> strs, char delim);
	std::string trimWhitespace(std::string string);
	bool startsWith(std::string_view string, std::string_view start) noexcept;
	bool endsWith(std::string_view string, std::string_view end) noexcept;
	bool startsWith(std::string_view string, char start) noexcept;
	bool endsWith(std::string_view string, char end) noexcept;

	/**
 * \typedef StringList
 * \brief	A list of std::string's.
 *
 * The StringList is provided primarily as a convenience typedef
 * but is also used by some of NAS2D's functions.
 */
	using StringList = std::vector<std::string>;

} // namespace NAS2D

