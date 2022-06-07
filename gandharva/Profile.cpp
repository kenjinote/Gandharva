#include "Profile.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <new>

#include "LanguageUtil.h"

namespace gandharva
{


CProfile::CProfile(std::wstring const& sProfileFile)
	: dict_    ()
	, sPath_   (sProfileFile)
	, bUpdated_(false)
{
	// ファイルをロード
	HANDLE hFile = ::CreateFile(
		sProfileFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	if(hFile == INVALID_HANDLE_VALUE) return;
	auto cleanFileHandle = Cleaner([&]{ ::CloseHandle(hFile); });

	std::wstring fileBuf;
	DWORD dwFileSize = ::GetFileSize(hFile, nullptr);
	if(dwFileSize < 100 * 1000 * 1000){
		fileBuf.resize((dwFileSize + sizeof(WCHAR) - 1) / sizeof(WCHAR));
		DWORD dwRead = 0;
		::ReadFile(hFile, const_cast<WCHAR*>(fileBuf.data()), dwFileSize, &dwRead, nullptr);
		fileBuf.resize(dwRead / sizeof(WCHAR));
	}

	cleanFileHandle.Invoke();

	// 内容を解析
	auto it = fileBuf.begin();
	if(*it == 0xfeff) ++it;

	auto GetEndOfLine = [&](std::wstring::iterator begL)
		-> std::wstring::iterator
	{
		for(; begL != fileBuf.end(); ++begL){
			if(*begL == L'\r' || *begL == L'\n') return begL;
		}
		return begL;
	};

	while(it != fileBuf.end()){
		//行を取得
		auto begL = it;
		auto endL = GetEndOfLine(begL);

		// とりあえずイテレータを進めておく
		it = (endL == fileBuf.end()) ? endL : endL + 1;

		//セパレータ (=) を取得
		auto itSep = std::find(begL, endL, L'=');

		// もしこの行に = が含まれていなければ、読み飛ばす
		if(itSep == endL) continue;

		// 登録
		dict_[std::wstring(begL, itSep)] = std::wstring(itSep+1, endL);
	}

	return;
}


bool CProfile::Save()
{
	if(! bUpdated_) return true;

	HANDLE hFile = ::CreateFile(
		sPath_.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	if(hFile == INVALID_HANDLE_VALUE) return false;
	auto cleanFileHandle = Cleaner([&]{ ::CloseHandle(hFile); });

	{	// Write BOM
		WCHAR bom = 0xfeff;
		DWORD dw = 0;
		::WriteFile(hFile, &bom, sizeof(bom), &dw, nullptr);
		if(dw != sizeof(bom)) return false; 
	}

	for(auto& item: this->dict_){
		DWORD cb = 0;
		::WriteFile(hFile, item.first.c_str(), item.first.size()*sizeof(WCHAR), &cb, nullptr);
		if(cb != item.first.size()*sizeof(WCHAR)) return false;

		cb = 0;
		::WriteFile(hFile, L"=", sizeof(WCHAR), &cb, nullptr);
		if(cb != sizeof(WCHAR)) return false;

		cb = 0;
		::WriteFile(hFile, item.second.c_str(), item.second.size()*sizeof(WCHAR), &cb, nullptr);
		if(cb != item.second.size()*sizeof(WCHAR)) return false;

		cb = 0;
		::WriteFile(hFile, L"\r\n", 2*sizeof(WCHAR), &cb, nullptr);
		if(cb != 2*sizeof(WCHAR)) return false;

	}

	return true;
}


void CProfile::Set(std::wstring const& key, std::wstring const& value)
{
	dict_[key] = value;
	bUpdated_  = true;
}


void CProfile::SetInt(std::wstring const& key, int iValue)
{
	WCHAR sz[64];
	int len = wnsprintf(sz, sizeof(sz)/sizeof(sz[0]), L"%d", iValue);
	this->Set(key, std::wstring(sz, sz + len));
}


std::wstring const& CProfile::Get(std::wstring const& key, LPCWSTR lpoptDefaultValue)
{
	auto it = dict_.find(key);
	if(it == dict_.end()){
		if(lpoptDefaultValue){
			it = dict_.insert(std::make_pair(key, std::wstring(lpoptDefaultValue))).first;
		}
		else{
			it = dict_.insert(std::make_pair(key, std::wstring())).first;
		}
		bUpdated_  = true;
	}

	return it->second;
}


int CProfile::GetInt(std::wstring const& key, int iDefaultValue)
{
	auto it = dict_.find(key);
	if(it == dict_.end()){
		WCHAR sz[64];
		int len = wnsprintf(sz, sizeof(sz)/sizeof(sz[0]), L"%d", iDefaultValue);

		dict_.insert(std::make_pair(key, std::wstring(sz, sz + len)));

		bUpdated_  = true;
		return iDefaultValue;
	}

	return StrToIntW(it->second.c_str());
}


namespace Profile
{
	namespace 
	{
		template <ProfileID id>
		struct GlobalVar
		{
			static char profileMem[sizeof(CProfile)];
		};

		template <ProfileID id>
		char GlobalVar<id>::profileMem[sizeof(CProfile)] = {};
	}

	template <ProfileID id>
	CProfile& Init(std::wstring const& sProfilePath)
	{
		return *(
			new(GlobalVar<id>::profileMem) CProfile(sProfilePath)
		);
	}


	template <ProfileID id>
	CProfile& Get()
	{
		return *static_cast<CProfile*>(operator new(0, GlobalVar<id>::profileMem));
	}


	template CProfile& Init<Config>(std::wstring const& sProfilePath);
	template CProfile& Init<State >(std::wstring const& sProfilePath);

	template CProfile& Get<Config>();
	template CProfile& Get<State >();

}

} // namespace gandharva
