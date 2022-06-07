#ifndef _56c4cc3c_85a9_4d36_80ad_c73849de84a7
#define _56c4cc3c_85a9_4d36_80ad_c73849de84a7

// Windows.h をインクルードして、ラッパー関数を定義する。
//
// _WIN32_WINNT のバージョンの問題があるので、
//  (例えば Rebar コントロールが動かなくなる)
// 個々のファイルは決して <Windows.h> を直接インクルードしてはいけない。
// 代わりにこのファイルをインクルードすること。

#define _WIN32_WINNT 0x0501
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <string>

namespace gandharva
{

class CLockWindowUpdate
{
public:
	CLockWindowUpdate()
		: bLocked_(FALSE)
	{}

	CLockWindowUpdate(HWND hWnd)
		: bLocked_(::LockWindowUpdate(hWnd))
	{}

	~CLockWindowUpdate(){
		this->Unlock();
	}

	BOOL Lock(HWND hWnd){
		this->Unlock();
		return bLocked_ = ::LockWindowUpdate(hWnd);
	}

	void Unlock(){
		if(bLocked_){
			::LockWindowUpdate(nullptr);
			bLocked_ = FALSE;
		}
	}

private:
	BOOL bLocked_;
};


class CTimer
{
public:
	CTimer(UINT id)
		: id_(id), bSet_(false)
	{}

	void Set(HWND hWnd, UINT uElapse){
		if(! this->bSet_){
			::SetTimer(hWnd, this->id_, uElapse, NULL);
			this->bSet_ = true;
		}
	}

	void Unset(HWND hWnd){
		if(this->bSet_){
			::KillTimer(hWnd, this->id_);
			this->bSet_ = false;
		}
	}

private:
	UINT id_;
	bool bSet_;
};


std::string  WideCharToMultiByte(std::wstring const& wcs, DWORD codePage = CP_ACP);
std::wstring MultiByteToWideChar(std::string const& mbcs, DWORD codePage = CP_ACP);

// エクスプローラで開く
bool ExplorerOpen(std::wstring const& path);

} // namespace gandharva


// マクロが大量にある Windowsx.h はインクルードしたくないので、
// 必要なものだけここにコピペしておく
#ifndef GET_X_LPARAM
#	define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#	define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif


#endif//_56c4cc3c_85a9_4d36_80ad_c73849de84a7
