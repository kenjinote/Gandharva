#include "ExplorerTree.h"
#include "MyWindows.h"
#include "LanguageUtil.h"
#include "Application.h"
#include "FilePath.h"
#include "resource.h"

#include "FolderZip.h"


namespace gandharva
{

ExplorerTree::ExplorerTree(HWND hwndParent, int iID)
{
	this->hwndParent_ = hwndParent;
	this->iID_ = iID;
	this->hwndThis_ = ::CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_TREEVIEW,
		nullptr,
		WS_VISIBLE | WS_CHILD | TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		0, 0, 0, 0,
		hwndParent,
		(HMENU)iID,
		::GetModuleHandle(nullptr),
		nullptr
	);

	this->fState_ = 0;
	this->fHiddenVolumes_ = 0;
}


DWORD
ExplorerTree::LoadFolderItem(HTREEITEM htiFolder, bool* pbCancelled)
{
	return this->LoadFolderItem(
		htiFolder, this->GetItemFullPath(htiFolder), std::wstring(), nullptr, pbCancelled
	);
}


DWORD
ExplorerTree::LoadFolderItem(
	HTREEITEM           htiFolder,
	std::wstring const& sFolderPath,
	std::wstring const& soptSubFolderTitle,
	HTREEITEM*          popt_htiSubFolder,
	bool              * pbCancelled
){
	*pbCancelled = false;
	this->fState_ |= TVS_LoadingSubFolder;

	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(Path::Join(sFolderPath, L"*").c_str(), &fd);

	DWORD numChildren = 0;
	HTREEITEM hItem = TVI_FIRST, htiSubFolder = nullptr;
	if(hFind != INVALID_HANDLE_VALUE){
		do{
			// サブフォルダをロードする作業は時間がかかることがあるので
			// 他のメッセージが来ていたらその処理を優先する
			Application::PeekAndDispatchMessage();

			if(this->fState_ & TVS_LoadingCancelled){
				// PeekAndDispatchMessage しているうちに
				// ユーザーが他のフォルダを開こうとしたことを感知すると
				// OnNotify 内で TVS_LoadingCancelled がセットされる

				// そのときは、このフォルダを開くことをやめる
				this->DeleteAllChildren(htiFolder);
				*pbCancelled = true;
				htiSubFolder = nullptr;
				numChildren = 0;
				break; // 後処理をして return
			}

			bool bAppend = false;
			bool bChildless = false;
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(fd.cFileName[0] == '.' && (
					fd.cFileName[1] == '\0' || (
						fd.cFileName[1] == '.' && fd.cFileName[2] == '\0'
					)
				)){
					// "." or ".." をスキップ
					;
				}
				else{
					bAppend = true;
				}
			}
			else{
				if(FolderZip::IsSupported(fd.cFileName)){
					bAppend = true;
					bChildless = true;
				}
				else if(::PathMatchSpec(fd.cFileName, TEXT("*.cue"))){
					bAppend = true;
					bChildless = true;
				}
			}

			if(bAppend){
				hItem = this->InsertItem(htiFolder, hItem, Path::Join(sFolderPath, fd.cFileName), fd.cFileName);
				if(bChildless) this->SetChildless(hItem);
				if(! soptSubFolderTitle.empty() &&
					Path::IsEqualFileSpec(soptSubFolderTitle, fd.cFileName))
				{
					htiSubFolder = hItem; 
				}
				++ numChildren;
			}
		}
		while(::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}

	if(! (this->fState_ & TVS_LoadingCancelled)){
		// ロードがキャンセルされたのでなければ
		// サブアイテムをソートする
		::SendMessage(hwndThis_, TVM_SORTCHILDREN, FALSE, (LPARAM)htiFolder);
	}

	this->fState_ &= ~(TVS_LoadingSubFolder | TVS_LoadingCancelled);
	if(popt_htiSubFolder) *popt_htiSubFolder = htiSubFolder;
	return numChildren;
}


std::wstring
ExplorerTree::GetItemFullPath(HTREEITEM hItem)
{
	if(! hItem){
		return std::wstring();
	}

	HTREEITEM htiParent = this->GetParent(hItem);

	if(htiParent){
		auto parentPath = this->GetItemFullPath(htiParent);
		auto thisPath   = this->GetText(hItem);
		return Path::Join(std::move(parentPath), std::move(thisPath));
	}
	else{
		return this->GetText(hItem);
	}
}


std::wstring
ExplorerTree::GetText(HTREEITEM hItem)
{
	// ツリービューアイテムはフォルダ名なので、長さはMAX_PATHを超えない
	WCHAR text[MAX_PATH + 10];
	TVITEM ti;
	ti.mask = TVIF_TEXT;
	ti.hItem = hItem;
	ti.pszText = text;
	ti.cchTextMax = lengthof(text) - 1;

	if(! TreeView_GetItem(hwndThis_, &ti)){
		text[0] = L'\0';
	}

	return std::wstring(ti.pszText);
}


HTREEITEM
ExplorerTree::InsertItem(
	HTREEITEM           hParent,
	HTREEITEM           hInsertAfter,
	std::wstring const& sFullPath,
	std::wstring const& sText,
	LPARAM              lParam
){
	TV_INSERTSTRUCT is = {};

	is.hParent	= hParent;
	is.hInsertAfter	= hInsertAfter;
	is.item.mask	= TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_PARAM;
   	is.item.pszText	= const_cast<LPTSTR>(sText.c_str());
	is.item.cChildren = 1;
	is.item.lParam = lParam;

	// システムから、アイコンの番号を取得
	SHFILEINFO fi;
	::SHGetFileInfo(
		sFullPath.c_str(),
		0,
		&fi,
		sizeof(fi), 
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON
	);

	is.item.iImage = fi.iIcon;

	::SHGetFileInfo(
		sFullPath.c_str(),
		0,
		&fi,
		sizeof(fi), 
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON
	);

	is.item.iSelectedImage=fi.iIcon;

	return TreeView_InsertItem(this->hwndThis_, &is);
}


void
ExplorerTree::DeleteAllChildren(HTREEITEM hItem)
{
	hItem = this->GetFirstChild(hItem);
	while(hItem){
		HTREEITEM htiKilled = hItem;
		hItem = this->GetNextSibling(hItem);
		this->DeleteItem(htiKilled);
	}
}


LPARAM
ExplorerTree::GetParam(HTREEITEM hItem)
{
	TVITEM ti;
	ti.mask = TVIF_PARAM;
	ti.hItem = hItem;

	if(TreeView_GetItem(hwndThis_, &ti)){
		return ti.lParam;
	}
	else{
		return 0;
	}
}


void
ExplorerTree::SetParam(HTREEITEM hItem, LPARAM lParam)
{
	TVITEM ti;
	ti.mask = TVIF_PARAM;
	ti.hItem = hItem;
	ti.lParam = lParam;

	TreeView_SetItem(hwndThis_, &ti);
}


HTREEITEM
ExplorerTree::SearchItem(HTREEITEM htiParent, std::wstring const& sItem)
{
	if(sItem.empty()) return nullptr;

	for(HTREEITEM hItem = this->GetFirstChild(htiParent);
		hItem != nullptr; hItem = this->GetNextSibling(hItem))
	{
		if(Path::IsEqualFileSpec(this->GetText(hItem), sItem)) return hItem;
	}

	return nullptr;
}


void
ExplorerTree::Expand(HTREEITEM hItem, WPARAM wTVE)
{
	::SendMessage(
		hwndThis_,
		TVM_EXPAND,
		wTVE,
		(LPARAM)hItem
	);
}


bool
ExplorerTree::IsExpanded(HTREEITEM hItem)
{
	TVITEM ti;
	ti.hItem	 = hItem;
	ti.mask		 = TVIF_STATE | TVIF_CHILDREN;
	ti.stateMask = TVIS_EXPANDED;

	LRESULT flags = ::SendMessage(
		hwndThis_,
		TVM_GETITEM,
		0,
		(LPARAM)&ti
	);

	return (ti.state & TVIS_EXPANDED) || ti.cChildren == 0;
}


bool
ExplorerTree::SetSelection(std::wstring const& path)
{
	CLockWindowUpdate lock(this->hwndThis_);

	HTREEITEM hItem = this->SetSelectionHelper(Path::GetFullName(path));
	if(hItem){
		this->EnsureVisible(hItem);
		this->SetSelection((HTREEITEM)nullptr); // ITEMEXPANDEDメッセを強制的に生じさせる
		this->SetSelection(hItem);
	}
	// hItem が nullptr のときは、すでにヘルパがSetSelectionを行っている

	return hItem != nullptr;
}


void ExplorerTree::SetChildless(HTREEITEM hItem, bool bChildless)
{
	TVITEM ti;
	ti.hItem	 = hItem;
	ti.mask		 = TVIF_CHILDREN;
	ti.cChildren = bChildless ? 0 : 1;
	TreeView_SetItem(this->hwndThis_, &ti);
}


void
ExplorerTree::AddFavorite(std::wstring const& path)
{
	this->InsertItem(TVI_ROOT, TVI_LAST, path, path, TVItem_Removable);
}


void
ExplorerTree::Renew()
{
	CLockWindowUpdate lock(this->hwndParent_); // 親ウィンドウから止めちゃいましょう

	// 今選択中のアイテムは？
	auto sSelItem = this->GetItemFullPath(this->GetSelection());

	// お気に入りを残してルートを消去
	HTREEITEM hItem = this->GetFirstChild(TVI_ROOT);
	while(hItem){
		HTREEITEM htiKilled = hItem;
		hItem = this->GetNextSibling(hItem);

		if(this->IsFavorite(htiKilled)){
			// お気に入りは子供だけ殺す
			this->Expand(htiKilled, TVE_COLLAPSE | TVE_COLLAPSERESET);
		}
		else{
			// ボリュームルートは本人も殺す
			this->DeleteItem(htiKilled);
		}
	}

	// ボリュームルートを再構築
	this->CreateRoot(this->GetHiddenVolumes());

	// 選択状態を復元
	this->SetSelection(sSelItem);
}


HTREEITEM
ExplorerTree::GetParent(HTREEITEM hItem)
{
	if(hItem == TVI_ROOT) return nullptr;
	return TreeView_GetParent(hwndThis_, hItem);
}


HTREEITEM
ExplorerTree::HitTest(int x, int y, bool bScreen)
{
	TV_HITTESTINFO info;

	info.pt.x = x;
	info.pt.y = y;
	if(bScreen) ::ScreenToClient(hwndThis_, &info.pt);

	TreeView_HitTest(hwndThis_, &info);
	return info.hItem;
}


void
ExplorerTree::GetItemRect(HTREEITEM hItem, RECT* prc, bool bScreen)
{
	TreeView_GetItemRect(hwndThis_, hItem, prc, TRUE);
	if(bScreen){
		RECT rcWnd;
		::GetWindowRect(hwndThis_, &rcWnd);
		prc->left += rcWnd.left;
		prc->right += rcWnd.left;
		prc->top += rcWnd.top;
		prc->bottom += rcWnd.top;
	}
}


LRESULT
ExplorerTree::DefOnNotify(NMHDR *pNMH)
{
	switch(pNMH->code){
	case TVN_ITEMEXPANDING:
		return OnItemExpanding((NMTREEVIEW*)pNMH);
	case TVN_ITEMEXPANDED:
		return OnItemExpanded((NMTREEVIEW*)pNMH);
	}

	return 0;
}


LRESULT
ExplorerTree::DefOnContextMenu(LPARAM lParam)
{
	enum{
		IDMI_ET_GOPARENT = 10,
		IDMI_ET_SHOWPLAYFILE,
		IDMI_ET_EXPLORER,
		IDMI_ET_ADDTOFAVORITE,
		IDMI_ET_REMOVEFROMFAVORITE,
		IDMI_ET_RENEW
	};

	int screenX = GET_X_LPARAM(lParam);
	int screenY = GET_Y_LPARAM(lParam);
	HTREEITEM hItem;

	if(screenX != -1 || screenY != -1){ // if not Shift+F10
		hItem = this->HitTest(screenX, screenY, true);
		if(! hItem) return 0;
	}
	else{
		hItem = this->GetSelection();
		if(! hItem) return 0;

		RECT rc;
		this->GetItemRect(hItem, &rc, true);
		screenX = (rc.right + rc.left) / 2;
		screenY = (rc.bottom + rc.top) / 2;
	}

	this->SetSelection(hItem);

	// 「上へ」用の変数
	HTREEITEM hParent = this->GetParent(hItem);
	std::wstring sParent;
	bool bCanGoParent = true;
	if(! hParent){
		sParent = Path::GetDirName(this->GetItemFullPath(hItem));
		if(sParent.empty()){
			bCanGoParent = false;
		}
	}

	// 今演奏中か、親に問い合わせてみる
	BOOL bPlaying = ::SendMessage(this->hwndParent_, MY_WM_ISPLAYING, 0, 0);

	// メニューを作成
	HMENU hMenu = ::CreatePopupMenu();

	if(bCanGoParent){
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_GOPARENT		, TEXT("上へ"));
	}
	if(bPlaying){
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_SHOWPLAYFILE		, TEXT("演奏中のフォルダへ"));
	}
	if(bCanGoParent || bPlaying){
		::AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
	}

	::AppendMenu(hMenu, MF_STRING, IDMI_ET_EXPLORER			, TEXT("エクスプローラを開く"));
	::AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

	if(this->IsFavorite(hItem)){
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_REMOVEFROMFAVORITE, TEXT("お気に入りから削除"));
	}
	else{
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_ADDTOFAVORITE, TEXT("お気に入りに追加"));
	}
	::AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

	::AppendMenu(hMenu, MF_STRING, IDMI_ET_RENEW, TEXT("最新の情報に更新"));

	int id = ::TrackPopupMenu(
		hMenu, TPM_NONOTIFY | TPM_RETURNCMD,
		screenX, screenY,
		0,
		hwndThis_,
		nullptr
	);
	::DestroyMenu(hMenu);

	switch(id){
	case IDMI_ET_GOPARENT:
		this->OnContextMenu_GoParent(hParent, sParent);
		break;
	case IDMI_ET_SHOWPLAYFILE:
		::SendMessage(this->hwndParent_, MY_WM_SHOWPLAYFILE, 0, 0);
		break;
	case IDMI_ET_EXPLORER:
		this->OnContextMenu_Explorer(hItem);
		break;
	case IDMI_ET_ADDTOFAVORITE:
		this->OnContextMenu_AddToFavorite(hItem);
		break;
	case IDMI_ET_REMOVEFROMFAVORITE:
		this->OnContextMenu_RemoveFromFavorite(hItem);
		break;
	case IDMI_ET_RENEW:
		this->Renew();
		break;
	}

	return 0;
}


void ExplorerTree::CreateRoot(DWORD fHiddenVolumes)
{
	this->fHiddenVolumes_ = fHiddenVolumes;

	HTREEITEM hParent = TVI_ROOT;

	// ボリュームを列挙
	TCHAR szVolume[4] = { 'A', ':', '\\', '\0' };
	HTREEITEM hItem = TVI_FIRST;
	bool bImageListIsSet = false;
	for(TCHAR cDrive = 'A'; cDrive <= 'Z'; ++cDrive){
		szVolume[0] = cDrive;
		if(::GetDriveType(szVolume) != DRIVE_NO_ROOT_DIR){
			// 初回のみ、アイコンを取得するために szVolumeを使う。
			if(! bImageListIsSet){
				HIMAGELIST hIL;
				SHFILEINFO fi;

				//イメージリストをシステムから取得
				hIL = (HIMAGELIST)SHGetFileInfo(
					szVolume,
					0,
					&fi,
					sizeof(SHFILEINFO), 
					SHGFI_SYSICONINDEX | SHGFI_SMALLICON
				);

				if(hIL){
					TreeView_SetImageList(this->hwndThis_, hIL, TVSIL_NORMAL);
				}

				bImageListIsSet = true;
			}

			// ツリーのルートに追加
			if((fHiddenVolumes >> (cDrive - 'A')) & 1){
				; // 隠す
			}
			else{
				hItem = this->InsertItem(TVI_ROOT, hItem, szVolume, szVolume);
			}
		}
	}
}


HTREEITEM 
ExplorerTree::SetSelectionHelper(std::wstring const& sFullPath)
{
	// ルートに見つかれば、それにする
	HTREEITEM htiThis = this->SearchItem(TVI_ROOT, sFullPath);
	if(htiThis) return htiThis;

	// ルートにないときは、親フォルダへ再帰する
	auto sUpperFolder = Path::GetDirName(sFullPath);

	if(sUpperFolder.empty()) return nullptr;

	// 上のフォルダまでツリーに表示
	HTREEITEM hParent = this->SetSelectionHelper(sUpperFolder);
	if(! hParent) return nullptr;

	htiThis = nullptr;
	if(this->IsExpanded(hParent)){
		// すでに親が展開されているときは、その中から探してみる
		htiThis = this->SearchItem(hParent, Path::GetBaseName(sFullPath));
		if(! htiThis){
			// 見つからなかったら、フォルダを再展開させるために、このフォルダを閉じる
			this->Expand(hParent, TVE_COLLAPSE | TVE_COLLAPSERESET);
			// 石女になってるかもしれないので、リセット
			this->SetChildless(hParent, false);
		}
	}
	if(! htiThis){
		// 親フォルダを展開してさがす
		this->Expand(hParent, TVE_EXPAND);
		htiThis = this->SearchItem(hParent, Path::GetBaseName(sFullPath));
	}
	if(! htiThis){
		// 上のフォルダまでがvalidのときは、それを選択。
		this->EnsureVisible(hParent);
		this->SetSelection((HTREEITEM)nullptr); // ITEMEXPANDEDメッセを強制的に生じさせる
		this->SetSelection(hParent);
	}

	return htiThis;
}


LRESULT
ExplorerTree::OnItemExpanding(NMTREEVIEW* pNMTV)
{
	if(this->fState_ & TVS_LoadingSubFolder){
		// 他のフォルダをロードしている途中だったら
		// そのフォルダのロードを中止する
		this->fState_ |= TVS_LoadingCancelled;
		// そしてこのフォルダを開くことはしない
		return 1;
	}

	if(pNMTV->action & TVE_EXPAND){
		// アイテムを展開する前に、サブフォルダをロード
		bool bCancelled;
		DWORD numChildren = this->LoadFolderItem(pNMTV->itemNew.hItem, &bCancelled);

		if(! bCancelled){
			if(numChildren > 0){
				// ロード成功
				return 0;
			}

			// 子供がいないときは、ツリーアイテムの左にある + マークを消す
			this->SetChildless(pNMTV->itemNew.hItem);
		}

		// 失敗
		return 1;
	}
	else if(pNMTV->action & TVE_COLLAPSE){
		// ただ閉じるだけだと、一度展開したことがある、という情報がツリービューに記憶されてしまい
		// 次回から展開しようとしても、ITEMEXPANDINGメッセが飛んでこないことがある
		// なので、ただ閉じるだけじゃなくてリセット命令つきで閉じる
		if(! (pNMTV->action & TVE_COLLAPSERESET)){
			this->Expand(pNMTV->itemNew.hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
		}
	}

	return 0;
}


LRESULT
ExplorerTree::OnItemExpanded(NMTREEVIEW* pNMTV)
{
//	if(pNMTV->action & TVE_COLLAPSE){
//		// アイテムを閉じたときは、子供を全て殺す
//		this->DeleteAllChildren(pNMTV->itemNew.hItem);
//	}

	return 0;
}


void
ExplorerTree::OnContextMenu_GoParent(HTREEITEM hParent, std::wstring const& sParent)
{
	if(hParent){
		this->SetSelection(hParent);
		this->EnsureVisible(hParent);
	}
	else if(! sParent.empty()){
		this->SetSelection(sParent);
		// パスで指定するほうのSetSelectionはEnsureVisibleを呼んでいる
	}
}


void
ExplorerTree::OnContextMenu_Explorer(HTREEITEM hItem)
{
	ExplorerOpen(this->GetItemFullPath(hItem));
}


void
ExplorerTree::OnContextMenu_RemoveFromFavorite(HTREEITEM hItem)
{
	this->DeleteItem(hItem);
}


void
ExplorerTree::OnContextMenu_AddToFavorite(HTREEITEM hItem)
{
	return this->AddFavorite(this->GetItemFullPath(hItem));
}

} // namespace gandharva
