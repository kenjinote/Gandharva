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
		// GetFullPath, CreatePlayer に渡す、ファイルのID
		std::wstring sFileSpec;
		// ファイルの種類（表示用）
		std::wstring sExtension;
		// サイズ（表示用 in bytes）; -1 で無効な値を表す 
		ULARGE_INTEGER uliSize;
		// 更新日時（表示用 in UTC）; 0 で無効な値を表す
		FILETIME ftDate;
	};

	// filters に該当するファイルを一つ探す
	// *pbLoading が false になったら、直ちに false を返す
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
