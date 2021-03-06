#ifndef _e2e0ed4b_0b66_402f_b499_d89bd5c736dc
#define _e2e0ed4b_0b66_402f_b499_d89bd5c736dc

#include "MyWindows.h"
#include <string>
#include <map>

namespace gandharva
{

class CProfile
{
public:
	explicit CProfile(std::wstring const& sProfileFile);

	bool Save();

	void Set(std::wstring const& key, std::wstring const& value);
	void Set(std::wstring const& key, int value){ this->SetInt(key, value); }
	void SetInt(std::wstring const& key, int iValue);

	std::wstring const& Get(std::wstring const& key, LPCWSTR lpoptDefaultValue = nullptr);
	int GetInt(std::wstring const& key, int iDefaultValue = 0);

private:
	CProfile(const CProfile&);
	void operator=(const CProfile&);

	std::map<std::wstring, std::wstring> dict_;
	std::wstring                         sPath_;
	bool                                 bUpdated_;
};


namespace Profile
{
	enum ProfileID
	{
		// 頻繁には書き換わらないモノ
		// e.g. 拡張子の設定、
		Config,

		// 頻繁に書き換わるモノ
		// e.g. ウィンドウが閉じだ時の位置
		State,
	};

	template <ProfileID id>
	CProfile& Init(std::wstring const& sProfilePath);

	template <ProfileID id>
	CProfile& Get();
}

} // namespace gandharva
#endif//_e2e0ed4b_0b66_402f_b499_d89bd5c736dc
