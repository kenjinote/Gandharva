#include "MyWindows.h"


namespace gandharva
{

std::string  WideCharToMultiByte(std::wstring const& wcs, DWORD codePage)
{
	int cb = ::WideCharToMultiByte(codePage, 0, wcs.c_str(), wcs.size(), nullptr, 0, nullptr, nullptr);
	if(cb <= 0) return std::string();

	std::string mbcs(cb, '\0');
	cb = ::WideCharToMultiByte(codePage, 0, wcs.c_str(), wcs.size(), const_cast<char*>(mbcs.c_str()), cb, nullptr, nullptr);

	mbcs.resize(cb);
	return mbcs;
}


std::wstring MultiByteToWideChar(std::string const& mbcs, DWORD codePage)
{
	int cch = ::MultiByteToWideChar(codePage, 0, mbcs.c_str(), mbcs.size(), nullptr, 0);
	if(cch <= 0) return std::wstring();

	std::wstring wcs(cch, L'\0');
	cch = ::MultiByteToWideChar(codePage, 0, mbcs.c_str(), mbcs.size(), const_cast<WCHAR*>(wcs.c_str()), cch);

	wcs.resize(cch);
	return wcs;
}


bool ExplorerOpen(std::wstring const& path)
{
	DWORD dwAttr = ::GetFileAttributes(path.c_str());
	if(dwAttr != (DWORD)-1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)){
		return 32 < (INT_PTR)::ShellExecute(
			NULL,
			NULL,
			path.c_str(),
			NULL,
			NULL,
			SW_SHOWNORMAL
		);
	}
	else{
		auto command = std::wstring(L"explorer /select,\"") + path + L"\"";

		PROCESS_INFORMATION pi;
		STARTUPINFO si = { sizeof(si) };

		if(::CreateProcess(
			NULL,
			const_cast<LPWSTR>(command.c_str()),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi
		)){
			::CloseHandle(pi.hThread);
			::CloseHandle(pi.hProcess);
			return true;
		}

		return false;
	}
}


} // namespace gandharva
