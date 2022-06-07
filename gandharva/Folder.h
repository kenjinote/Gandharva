#ifndef _02ee4383_1e54_4ab5_8ed7_d6aa3a6d74bd
#define _02ee4383_1e54_4ab5_8ed7_d6aa3a6d74bd

#include "MyWindows.h"
#include "Player.h"

#include <vector>
#include <string>
#include <memory>

namespace gandharva
{

class Folder
{
public:
	typedef std::shared_ptr<Folder>  P;
	
	Folder(){}
	virtual ~Folder(){}

	struct FindData
	{
		// GetFullPath, CreatePlayer �ɓn���A�t�@�C����ID
		std::wstring sFileSpec;
		// �t�@�C���̎�ށi�\���p�j
		std::wstring sExtension;
		// �T�C�Y�i�\���p in bytes�j; -1 �Ŗ����Ȓl��\�� 
		ULARGE_INTEGER uliSize;
		// �X�V�����i�\���p in UTC�j; 0 �Ŗ����Ȓl��\��
		FILETIME ftDate;
	};

	// filters �ɊY������t�@�C������T��
	// *pbLoading �� false �ɂȂ�����A������ false ��Ԃ�
	virtual bool Find(std::vector<std::wstring> const& filters, FindData* pfd, bool* pbLoading) = 0;
	virtual void FindClose() = 0;

	virtual std::wstring const& GetPath() = 0;

	virtual Player::P CreatePlayer(std::wstring const& sFileSpec, HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound) = 0;

private:
	Folder(Folder const&);
	void operator=(Folder const&);
};

} // namespace gandharva
#endif//_02ee4383_1e54_4ab5_8ed7_d6aa3a6d74bd
