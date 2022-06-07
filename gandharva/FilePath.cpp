#include "FilePath.h"


namespace gandharva
{

namespace Path
{

std::wstring Join(std::wstring const& dir, std::wstring const& file)
{
	if(dir.empty()) return file;

	if(dir.back() == L'\\'){
		return dir + file;
	}
	else{
		return dir + L"\\" + file;
	}
}


std::wstring GetFullName(std::wstring const& relativePath)
{
	auto cch = ::GetFullPathName(relativePath.c_str(), 0, nullptr, nullptr);

	std::wstring fullPath(cch, L'\0');

	cch = ::GetFullPathName(
		relativePath.c_str(), cch, const_cast<LPWSTR>(fullPath.data()), nullptr
	);

	fullPath.resize(cch);
	return fullPath;
}


std::wstring GetBaseName(std::wstring const& path)
{
	wchar_t const* it = ::PathFindFileName(path.data());
	return std::wstring(it, path.data() + path.size());
}


std::wstring GetDirName (std::wstring const& path)
{
	std::wstring dir = path;
	if(::PathRemoveFileSpec(const_cast<LPWSTR>(dir.c_str()))){
		dir.resize(::lstrlen(dir.c_str()));
	}
	else{
		dir.clear();
	}

	return dir;
}


std::wstring RemoveExtension(std::wstring const& path)
{
	LPCWSTR pExt = ::PathFindExtension(path.c_str());
	return std::wstring(path.c_str(), pExt);
}


std::wstring GetExePath()
{
	std::wstring path(MAX_PATH, L'\0');

	while(1){
		DWORD len = ::GetModuleFileName(nullptr, const_cast<LPWSTR>(path.data()), path.size());
		if(len >= (DWORD)path.size()) continue;

		path.resize(len);
		return path;
	}
}


bool IsMatchFilters(std::vector<std::wstring> const& filters, std::wstring const& path)
{
	for(auto& f: filters){
		if(::PathMatchSpec(path.c_str(), f.c_str())) return true;
	}

	return false;
}

} // namespace Path
} // namespace gandharva
