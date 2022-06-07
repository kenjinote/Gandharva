#ifndef _2f68bc2e_bf42_4aa2_8042_cdd8b2d6acd2
#define _2f68bc2e_bf42_4aa2_8042_cdd8b2d6acd2

#include "Folder.h"
#include "Archive.h"


namespace gandharva
{

class FolderZip
	: public Folder
{
public:
	FolderZip(std::wstring const& sZipPath);

	virtual ~FolderZip(){}

	virtual bool Find(std::vector<std::wstring> const& filters, FindData* pfd, bool* pbLoading);
	virtual void FindClose();

	virtual std::wstring const& GetPath() { return this->sZipPath_; }

	virtual Player::P CreatePlayer(std::wstring const& sFileSpec, HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound);

	static bool IsSupported(std::wstring const& sFileSpec);

private:
	std::wstring                      sZipPath_;
	Archive::P                        pArc_    ;
	Archive::FileList::const_iterator itFileList_;
};

} // namespace gandharva
#endif//_2f68bc2e_bf42_4aa2_8042_cdd8b2d6acd2
