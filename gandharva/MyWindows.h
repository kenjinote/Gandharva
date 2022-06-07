#ifndef _56c4cc3c_85a9_4d36_80ad_c73849de84a7
#define _56c4cc3c_85a9_4d36_80ad_c73849de84a7

// Windows.h ���C���N���[�h���āA���b�p�[�֐����`����B
//
// _WIN32_WINNT �̃o�[�W�����̖�肪����̂ŁA
//  (�Ⴆ�� Rebar �R���g���[���������Ȃ��Ȃ�)
// �X�̃t�@�C���͌����� <Windows.h> �𒼐ڃC���N���[�h���Ă͂����Ȃ��B
// ����ɂ��̃t�@�C�����C���N���[�h���邱�ƁB

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

// �G�N�X�v���[���ŊJ��
bool ExplorerOpen(std::wstring const& path);

} // namespace gandharva


// �}�N������ʂɂ��� Windowsx.h �̓C���N���[�h�������Ȃ��̂ŁA
// �K�v�Ȃ��̂��������ɃR�s�y���Ă���
#ifndef GET_X_LPARAM
#	define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#	define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif


#endif//_56c4cc3c_85a9_4d36_80ad_c73849de84a7
