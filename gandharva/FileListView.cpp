#include "FileListView.h"
#include "Application.h"
#include "MyWindows.h"
#include "FilePath.h"
#include "Profile.h"
#include "LanguageUtil.h"


namespace gandharva
{


FileListView::FileListView(HWND hwndParent, int iID)
	: filters_       ()
	, bLoadingFolder_(false)
	, hwndThis_      (nullptr)
	, hwndParent_    (hwndParent)
	, iID_           (iID)
	, bRandom_       (false)
{
	auto& config = Profile::Get<Profile::Config>();
	auto& state  = Profile::Get<Profile::State >();

	this->sortInfo_.hwndLV   = nullptr;
	this->sortInfo_.iSubItem = ICOL_Title;
	this->sortInfo_.bAscend  = true;

	this->filters_ = Split(config.Get(L"Playable", L"*.mp?;*.wma;*.asf;*.wav;*.og?;*.aac"), L';');

	this->hwndThis_ = ::CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		nullptr,
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		0, 0, 0, 0,
		hwndParent,
		(HMENU)iID,
		::GetModuleHandle(nullptr),
		nullptr
	);

	ListView_SetExtendedListViewStyle(this->hwndThis_,
		LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP
	);

	this->CreateColumn();
}


void
FileListView::SetColumnWidth(const SColumnWidth& cw)
{
	for(int icol = ICOL_Begin; icol < ICOL_End; ++icol){
		ListView_SetColumnWidth(this->hwndThis_, icol, cw.acx[icol]);
	}
}


void
FileListView::GetColumnWidth(SColumnWidth* pcw)
{
	for(int icol = ICOL_Begin; icol < ICOL_End; ++icol){
		pcw->acx[icol] = ListView_GetColumnWidth(this->hwndThis_, icol);
	}
}


std::wstring
FileListView::GetItemText(int iItem, int iSubItem)
{
	WCHAR szText[MAX_PATH + 10] = {};
	ListView_GetItemText(this->hwndThis_, iItem, iSubItem, szText, MAX_PATH + 9);
	return std::wstring(szText);
}


int
FileListView::GetSelectedItem()
{
	int iItem = ListView_GetSelectionMark(this->hwndThis_);
	// セレクションマークは、実際に選択されているアイテムとずれることがあるので
	// 本当にそれが選択されているアイテムなのか検証する
	if(iItem >= 0 &&
		(ListView_GetItemState(this->hwndThis_, iItem, LVIS_SELECTED) & LVIS_SELECTED))
	{
		return iItem;
	}

	// セレクションマークがずれていたときは
	// 選択されているアイテムを虱潰しに調べる
	int cItems = this->GetItemCount();
	for(iItem = 0; iItem < cItems; ++iItem){
		if(ListView_GetItemState(this->hwndThis_, iItem, LVIS_SELECTED) & LVIS_SELECTED){
			return iItem;
		}
	}

	// 結局何も選択されてませんですた
	return -1;
}


void
FileListView::SetSelectedItem(int iItem)
{
	if(iItem >= 0){
		ListView_SetItemState(this->hwndThis_, iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_SetSelectionMark(this->hwndThis_, iItem);
		this->EnsureVisible(iItem);
	}
}


bool
FileListView::SetSelectedItem(std::wstring const& title)
{
	int cItems = this->GetItemCount();
	for(int iItem = 0; iItem < cItems; ++iItem){
		if(Path::IsEqualFileSpec(title, this->GetItemText(iItem, ICOL_Title))){
			this->SetSelectedItem(iItem);
			return true;
		}
	}

	return false;
}


void FileListView::EnsureVisible(int iItem)
{
	if(iItem >= 0){
		ListView_EnsureVisible(this->hwndThis_, iItem, FALSE);
	}
}


int
FileListView::GetItemCount()
{
	return ListView_GetItemCount(this->hwndThis_);
}


namespace
{
	void FileSizeToStr(ULARGE_INTEGER& uli, LPTSTR lp)
	{
		if(uli.HighPart > 0){
			::lstrcpy(lp, TEXT(">4GB"));
		}
		else{
			TCHAR asz[4][4];
			int ptr = 0;
			DWORD dw = uli.LowPart;
			do{
				DWORD dwHi  = dw / 1000;
				DWORD dwLow = dw % 1000;

				wsprintf(asz[ptr++],
					dwHi > 0 ? TEXT("%03u") : TEXT("%u"),
					dwLow
				);

				dw = dwHi;
			}
			while(dw > 0);

			while(--ptr > 0){
				int len = ::lstrlen(asz[ptr]);
				CopyMemory(lp, asz[ptr], len * sizeof(TCHAR));
				lp[len] = ',';
				lp += len + 1;
			}

			lstrcpy(lp, asz[0]);
		}
	}


	int FileSizeCompareStr(LPCTSTR lp1, LPCTSTR lp2)
	{
		if('0' <= lp1[0] && lp1[0] <= '9' &&
			'0' <= lp2[0] && lp2[0] <= '9' )
		{
			// どちらも数値のとき
			int len1 = ::lstrlen(lp1);
			int len2 = ::lstrlen(lp2);
			if(len1 == len2){
				// 長さが一緒のとき
				return ::lstrcmp(lp1, lp2);
			}
			else{
				// 短いほうが、数値が小さい
				return len1 - len2;
			}
		}
		else{
			// 数値以外はどうでもいい
			return lp1[0] - lp2[0];
		}
	}
}


void
FileListView::LoadFolderContents(Folder::P const& pFolder)
{
	this->bLoadingFolder_ = true;

	CLockWindowUpdate lock(this->hwndThis_);

	ListView_DeleteAllItems(this->hwndThis_);

	Folder::FindData fd;
	int iItem = 0;
	while(pFolder->Find(filters_, &fd, &this->bLoadingFolder_)){
		// サブアイテムをリストへ追加
		TCHAR szSize[32];
		if(fd.uliSize.LowPart != (DWORD)(-1) || fd.uliSize.HighPart != (DWORD)(-1)){
			FileSizeToStr(fd.uliSize, szSize);
		}
		else{
			// 無効
			szSize[0] = 0;
		}

		BOOL bDateValid = FALSE;
		FILETIME ftLocal;
		if(fd.ftDate.dwHighDateTime != 0 || fd.ftDate.dwLowDateTime != 0){
			bDateValid = ::FileTimeToLocalFileTime(&fd.ftDate, &ftLocal);
		}
		SYSTEMTIME st;
		if(bDateValid){
			bDateValid = ::FileTimeToSystemTime(&ftLocal, &st);
		}
		TCHAR szDate[32];
		if(bDateValid){
			wsprintf(szDate, TEXT("%04u/%02u/%02u %02u:%02u:%02u"),
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond
			);
		}
		else{
			// 無効
			szDate[0] = 0;
		}

		LPCTSTR alp[ICOL_End];
			alp[ICOL_Title]     = fd.sFileSpec.c_str();
			alp[ICOL_Size]      = szSize;
			alp[ICOL_Extension] = fd.sExtension.c_str();
			alp[ICOL_Date]      = szDate;

		for(int icol = ICOL_Begin; icol < ICOL_End; ++icol){
			LVITEM li;
			li.mask = LVIF_TEXT;
			li.cchTextMax = -1;
			li.iItem = iItem;
			li.iSubItem = icol;
			li.pszText = const_cast<LPTSTR>(alp[icol]);
			if(icol > ICOL_Begin){
				ListView_SetItem(this->hwndThis_, &li);
			}
			else{
				li.mask = LVIF_TEXT | LVIF_PARAM;
				li.lParam = iItem; // ランダムシャッフル用
				ListView_InsertItem(this->hwndThis_, &li);
			}
		}

		++iItem;
	}

	if(this->bLoadingFolder_){
		this->SortItems();
		this->bLoadingFolder_ = false;
	}
}


void
FileListView::SetRandom(bool bRandom)
{
	this->bRandom_ = bRandom;
	this->SortItems();
}


LRESULT
FileListView::DefOnNotify(NMHDR* pNMH)
{
	switch(pNMH->code){
	case LVN_COLUMNCLICK:
		return this->OnColumnClick((NMITEMACTIVATE*)pNMH);
	}

	return 0;
}


void
FileListView::CreateColumn()
{
	const LPCTSTR alpText[] = {
		TEXT("ファイル名"),
		TEXT("サイズ"),
		TEXT("種類"),
		TEXT("更新日時")
	};

	for(int icol = ICOL_Begin; icol < ICOL_End; ++icol){
		LVCOLUMN lc;
		lc.mask = LVCF_TEXT | LVCF_FMT;
		lc.pszText = const_cast<LPTSTR>(alpText[icol]);
		lc.cchTextMax = -1;
		lc.fmt = (icol != ICOL_Size) ? LVCFMT_LEFT : LVCFMT_RIGHT;
		ListView_InsertColumn(this->hwndThis_, icol, &lc);
	}
}


LRESULT
FileListView::OnColumnClick(NMITEMACTIVATE* pNMIA)
{
	if(this->sortInfo_.iSubItem == pNMIA->iSubItem){
		this->sortInfo_.bAscend = ! this->sortInfo_.bAscend;
	}
	else{
		this->sortInfo_.iSubItem = pNMIA->iSubItem;
	}

	this->SortItems();

	return 0;
}


namespace
{
	int CALLBACK
	CompareLVItems(
		LPARAM lpm_iItem1, LPARAM lpm_iItem2, LPARAM lpm_pSortInfo
	){
		FileListView::SSortInfo* const pSortInfo =
			(FileListView::SSortInfo*)lpm_pSortInfo;

		TCHAR sz1[MAX_PATH + 10];
		TCHAR sz2[MAX_PATH + 10];
		ListView_GetItemText(pSortInfo->hwndLV, lpm_iItem1, pSortInfo->iSubItem, sz1, MAX_PATH + 9);
		ListView_GetItemText(pSortInfo->hwndLV, lpm_iItem2, pSortInfo->iSubItem, sz2, MAX_PATH + 9);

		int result;
		if(pSortInfo->iSubItem != FileListView::ICOL_Size){
			result = ::lstrcmpi(sz1, sz2);
		}
		else{
			result = FileSizeCompareStr(sz1, sz2);
		}

		return pSortInfo->bAscend ? result : -result;
	}

	int CALLBACK
	RandCompareLVItems(
		LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort
	){
		return
			(lParam1 < lParam2) ? -1 :
			(lParam1 > lParam2) ? 1  :
			/* left == right*/ 0 ;
	}
}

void
FileListView::SortItems()
{
	if(this->bRandom_){
		int cItems = this->GetItemCount();
		if(cItems > 0){
			LVITEM item = {};
			item.mask = LVIF_PARAM;

			for(int i = 0; i < cItems; ++i){
				item.iItem  = i;
				item.lParam = rand_();

				ListView_SetItem(this->GetHandle(), &item);
			}

			ListView_SortItems(this->hwndThis_, RandCompareLVItems, 0);
		}
	}
	else{
		this->sortInfo_.hwndLV = this->hwndThis_;
		ListView_SortItemsEx(this->hwndThis_, CompareLVItems, &this->sortInfo_);
	}
}

} // namespace gandharva
