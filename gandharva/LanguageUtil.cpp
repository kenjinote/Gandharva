#include "LanguageUtil.h"

#include "MyWindows.h"
#include <algorithm>


namespace gandharva
{

std::wstring TimeToString(long seconds)
{
	std::wstring str(64, L'\0');
	auto mem = const_cast<LPWSTR>(str.data());

	if(seconds < 60){
		int len = wsprintfW(mem, L"%d", (int)seconds);
		str.resize(len);
		return str;
	}

	int minutes = seconds / 60;
		seconds %= 60;

	if(minutes < 60){
		int len = wsprintfW(mem, L"%d:%.2d", minutes, (int)seconds);
		str.resize(len);
		return str;
	}

	int hours = minutes / 60;
		minutes %= 60;

	int len = wsprintfW(mem, L"%d:%.2d:%.2d", hours, minutes, (int)seconds);
	str.resize(len);
	return str;
}


std::vector<std::wstring> Split(std::wstring const& str, wchar_t separator)
{
	std::vector<std::wstring> vec;
	auto it = str.begin();
	while(1){
		auto end = std::find(it, str.end(), separator);
		if(end - it > 0){
			vec.emplace_back(it, end);
		}

		if(end == str.end()) break;
		it = end + 1;
	}

	return vec;
}


} // namespace gandharva
