#include "FolderDir.h"
#include "FilePath.h"
#include "Application.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


namespace gandharva
{

FolderDir::FolderDir(std::wstring const& sDirectory)
	: sDirectory_(sDirectory), hFind_(INVALID_HANDLE_VALUE)
{}


FolderDir::~FolderDir()
{
	this->FindClose();
}


bool FolderDir::Find(std::vector<std::wstring> const& filters, FindData* pfd, bool* pbLoading)
{
	while(1){
		// ���Ԃ̂����鏈���Ȃ̂ŁA���̃��b�Z�[�W��D�悳����
		Application::PeekAndDispatchMessage();
		if(! *pbLoading){
			// *pbLoading �����Z�b�g���ꂽ��I��
			return false;
		}

		WIN32_FIND_DATA fd;

		if(this->hFind_ != INVALID_HANDLE_VALUE){
			// 2��ڈȍ~�̌Ăяo��

			if(! ::FindNextFile(this->hFind_, &fd)){
				return false;
			}
		}
		else{
			// ����Ăяo��

			this->hFind_ = ::FindFirstFile(Path::Join(this->sDirectory_, L"*").c_str(), &fd);

			if(this->hFind_ == INVALID_HANDLE_VALUE){
				return false;
			}
		}

		if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& Path::IsMatchFilters(filters, fd.cFileName))
		{
			// pfd �֊i�[���Ė߂�
			pfd->sFileSpec =  fd.cFileName;
			LPTSTR lpExt = ::PathFindExtension(fd.cFileName);
				if(lpExt[0] == '.') ++lpExt;
			pfd->sExtension = lpExt;
			pfd->uliSize.LowPart  = fd.nFileSizeLow;
			pfd->uliSize.HighPart = fd.nFileSizeHigh;
			CopyMemory(& pfd->ftDate, & fd.ftLastWriteTime, sizeof(FILETIME));
			return true;
		}
	}
}


void FolderDir::FindClose()
{
	if(this->hFind_ != INVALID_HANDLE_VALUE){
		::FindClose(this->hFind_);
		this->hFind_ = INVALID_HANDLE_VALUE;
	}
}


Player::P FolderDir::CreatePlayer(std::wstring const& sFileSpec, HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound)
{
	auto sFullPath = Path::Join(this->sDirectory_, sFileSpec);

	return Player::P(new Player(
		FileSourceRenderer(sFullPath),
		this->sDirectory_, sFileSpec,
		hwndNotify, uMsgNotify, uMsgEndOfSound
	));
}

} // namespace gandharva
