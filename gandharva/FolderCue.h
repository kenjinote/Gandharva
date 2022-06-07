#ifndef _2e543d8d_f66e_4030_8dbb_4b8ea5281a9f
#define _2e543d8d_f66e_4030_8dbb_4b8ea5281a9f


#include "Folder.h"


namespace gandharva
{

class FolderCue
	: public Folder
{
public:
	FolderCue(std::wstring const& sCuePath);
	virtual ~FolderCue(){}

	// filters ÇÕñ≥éãÇ≥ÇÍÇÈ
	// *pbLoading Ç™ false Ç…Ç»Ç¡ÇΩÇÁÅAíºÇøÇ… false Çï‘Ç∑
	virtual bool Find(std::vector<std::wstring> const& filters, FindData* pfd, bool* pbLoading);
	virtual void FindClose();

	virtual std::wstring const& GetPath(){ return sCuePath_; }

	virtual Player::P CreatePlayer(std::wstring const& sFileSpec, HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound);

private:
	struct STrack {
		LONGLONG     _100nsBegin;
		LONGLONG     _100nsEnd;
		std::wstring sFile ;
		std::wstring sTitle;
	};

	STrack aTracks_[99];
	DWORD  cTracks_;
	DWORD  ixNextTrack_;

	std::wstring sCuePath_;
};

} // namespace gandharva
#endif//_2e543d8d_f66e_4030_8dbb_4b8ea5281a9f
