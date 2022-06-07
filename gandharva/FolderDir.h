#ifndef _fa2418e3_2766_4f32_b1e5_f692aec00ee2
#define _fa2418e3_2766_4f32_b1e5_f692aec00ee2

#include "Folder.h"

namespace gandharva
{

class FolderDir
	: public Folder
{
public:
	FolderDir(std::wstring const& sDirectory);
	virtual ~FolderDir();

	virtual bool Find(std::vector<std::wstring> const& filters, FindData* pfd, bool* pbLoading);
	virtual void FindClose();

	virtual std::wstring const& GetPath(){ return sDirectory_; }

	virtual Player::P CreatePlayer(std::wstring const& sFileSpec, HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound);

private:
	std::wstring sDirectory_;
	HANDLE       hFind_;
};

} // namespace gandharva
#endif//_fa2418e3_2766_4f32_b1e5_f692aec00ee2
