#include "FolderZip.h"
#include "FilePath.h"
#include "MemSourceFilter.h"
#include "Application.h"


namespace gandharva
{

FolderZip::FolderZip(std::wstring const& sZipPath)
	: sZipPath_  (sZipPath)
	, pArc_      (std::make_shared<Archive>(sZipPath))
	, itFileList_(pArc_->GetFileList().begin())
{
	// pArc_->IsOK() Ç™ê¨å˜Ç≈Ç‡ÇªÇ§Ç≈Ç»Ç≠ÇƒÇ‡ñ‚ëËÇ»Ç¢
}


bool FolderZip::Find(std::vector<std::wstring> const& filters, FindData* pfd, bool* pbLoading)
{
	auto end = pArc_->GetFileList().end();
	for(; itFileList_ != end; ++itFileList_){
		if(! Path::IsMatchFilters(filters, itFileList_->first)) continue;

		pfd->sFileSpec = itFileList_->first;

		LPTSTR lpExt = ::PathFindExtension(pfd->sFileSpec.c_str());
			if(lpExt[0] == '.') ++lpExt;
		pfd->sExtension = lpExt;

		pfd->uliSize.QuadPart = pArc_->GetSize(itFileList_->second);

		FILETIME ft = pArc_->GetTime(itFileList_->second);
		CopyMemory(&pfd->ftDate, &ft, sizeof(pfd->ftDate));

		// ñﬂÇÈëOÇ… itFileList ÇéüÇ…êiÇﬂÇƒÇ®Ç©Ç»Ç¢Ç∆Ç¢ÇØÇ»Ç¢
		++itFileList_;
		return true;
	}

	return false;
}


void FolderZip::FindClose()
{
	itFileList_ = pArc_->GetFileList().begin();
}


namespace
{
	class ZipSourceRenderer
		: public SourceRenderer
	{
	public:
		ZipSourceRenderer(Archive::P& pArc, std::wstring const& sFileSpec)
			: pArc_(pArc), sFileSpec_(sFileSpec)
		{}

		virtual ~ZipSourceRenderer(){}

		virtual HRESULT Render(IGraphBuilder* pGraph) const {
			auto pFilter = make_comptr<MemSourceFilter>();
			if(! pFilter) return E_FAIL;

			auto hr = pGraph->AddFilter(pFilter.get(), L"MemSource");
			if(FAILED(hr)) return hr;

			pFilter->SetMemory(pArc_->GetData(sFileSpec_));

			comptr<IPin> pPin(pFilter->GetOutputPin());
			return pGraph->Render(pPin.get());
		}

	private:
		Archive::P&  pArc_;
		std::wstring const& sFileSpec_;
	};
}


Player::P FolderZip::CreatePlayer(std::wstring const& sFileSpec, HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound)
{
	return Player::P(new Player(
		ZipSourceRenderer(pArc_, sFileSpec),
		this->sZipPath_, sFileSpec,
		hwndNotify, uMsgNotify, uMsgEndOfSound
	));
}


bool FolderZip::IsSupported(std::wstring const& sFileSpec)
{
	auto& filters = Archive::GetFilters();

	for(auto& f : filters){
		if(Path::IsMatchFilters(f.filters, sFileSpec)) return true;
	}

	return false;
}

} // namespace gandharva
