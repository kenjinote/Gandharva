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
			// �T�u�t�H���_�����[�h�����Ƃ͎��Ԃ������邱�Ƃ�����̂�
			// ���̃��b�Z�[�W�����Ă����炻�̏�����D�悷��
			Application::PeekAndDispatchMessage();

			if(this->fState_ & TVS_LoadingCancelled){
				// PeekAndDispatchMessage ���Ă��邤����
				// ���[�U�[�����̃t�H���_���J�����Ƃ������Ƃ����m�����
				// OnNotify ���� TVS_LoadingCancelled ���Z�b�g�����

				// ���̂Ƃ��́A���̃t�H���_���J�����Ƃ���߂�
				this->DeleteAllChildren(htiFolder);
				*pbCancelled = true;
				htiSubFolder = nullptr;
				numChildren = 0;
				break; // �㏈�������� return
			}

			bool bAppend = false;
			bool bChildless = false;
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(fd.cFileName[0] == '.' && (
					fd.cFileName[1] == '\0' || (
						fd.cFileName[1] == '.' && fd.cFileName[2] == '\0'
					)
				)){
					// "." or ".." ���X�L�b�v
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
		// ���[�h���L�����Z�����ꂽ�̂łȂ����
		// �T�u�A�C�e�����\�[�g����
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
	// �c���[�r���[�A�C�e���̓t�H���_���Ȃ̂ŁA������MAX_PATH�𒴂��Ȃ�
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

	// �V�X�e������A�A�C�R���̔ԍ����擾
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
		this->SetSelection((HTREEITEM)nullptr); // ITEMEXPANDED���b�Z�������I�ɐ���������
		this->SetSelection(hItem);
	}
	// hItem �� nullptr �̂Ƃ��́A���łɃw���p��SetSelection���s���Ă���

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
	CLockWindowUpdate lock(this->hwndParent_); // �e�E�B���h�E����~�߂��Ⴂ�܂��傤

	// ���I�𒆂̃A�C�e���́H
	auto sSelItem = this->GetItemFullPath(this->GetSelection());

	// ���C�ɓ�����c���ă��[�g������
	HTREEITEM hItem = this->GetFirstChild(TVI_ROOT);
	while(hItem){
		HTREEITEM htiKilled = hItem;
		hItem = this->GetNextSibling(hItem);

		if(this->IsFavorite(htiKilled)){
			// ���C�ɓ���͎q�������E��
			this->Expand(htiKilled, TVE_COLLAPSE | TVE_COLLAPSERESET);
		}
		else{
			// �{�����[�����[�g�͖{�l���E��
			this->DeleteItem(htiKilled);
		}
	}

	// �{�����[�����[�g���č\�z
	this->CreateRoot(this->GetHiddenVolumes());

	// �I����Ԃ𕜌�
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

	// �u��ցv�p�̕ϐ�
	HTREEITEM hParent = this->GetParent(hItem);
	std::wstring sParent;
	bool bCanGoParent = true;
	if(! hParent){
		sParent = Path::GetDirName(this->GetItemFullPath(hItem));
		if(sParent.empty()){
			bCanGoParent = false;
		}
	}

	// �����t�����A�e�ɖ₢���킹�Ă݂�
	BOOL bPlaying = ::SendMessage(this->hwndParent_, MY_WM_ISPLAYING, 0, 0);

	// ���j���[���쐬
	HMENU hMenu = ::CreatePopupMenu();

	if(bCanGoParent){
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_GOPARENT		, TEXT("���"));
	}
	if(bPlaying){
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_SHOWPLAYFILE		, TEXT("���t���̃t�H���_��"));
	}
	if(bCanGoParent || bPlaying){
		::AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
	}

	::AppendMenu(hMenu, MF_STRING, IDMI_ET_EXPLORER			, TEXT("�G�N�X�v���[�����J��"));
	::AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

	if(this->IsFavorite(hItem)){
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_REMOVEFROMFAVORITE, TEXT("���C�ɓ��肩��폜"));
	}
	else{
		::AppendMenu(hMenu, MF_STRING, IDMI_ET_ADDTOFAVORITE, TEXT("���C�ɓ���ɒǉ�"));
	}
	::AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

	::AppendMenu(hMenu, MF_STRING, IDMI_ET_RENEW, TEXT("�ŐV�̏��ɍX�V"));

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

	// �{�����[�����
	TCHAR szVolume[4] = { 'A', ':', '\\', '\0' };
	HTREEITEM hItem = TVI_FIRST;
	bool bImageListIsSet = false;
	for(TCHAR cDrive = 'A'; cDrive <= 'Z'; ++cDrive){
		szVolume[0] = cDrive;
		if(::GetDriveType(szVolume) != DRIVE_NO_ROOT_DIR){
			// ����̂݁A�A�C�R�����擾���邽�߂� szVolume���g���B
			if(! bImageListIsSet){
				HIMAGELIST hIL;
				SHFILEINFO fi;

				//�C���[�W���X�g���V�X�e������擾
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

			// �c���[�̃��[�g�ɒǉ�
			if((fHiddenVolumes >> (cDrive - 'A')) & 1){
				; // �B��
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
	// ���[�g�Ɍ�����΁A����ɂ���
	HTREEITEM htiThis = this->SearchItem(TVI_ROOT, sFullPath);
	if(htiThis) return htiThis;

	// ���[�g�ɂȂ��Ƃ��́A�e�t�H���_�֍ċA����
	auto sUpperFolder = Path::GetDirName(sFullPath);

	if(sUpperFolder.empty()) return nullptr;

	// ��̃t�H���_�܂Ńc���[�ɕ\��
	HTREEITEM hParent = this->SetSelectionHelper(sUpperFolder);
	if(! hParent) return nullptr;

	htiThis = nullptr;
	if(this->IsExpanded(hParent)){
		// ���łɐe���W�J����Ă���Ƃ��́A���̒�����T���Ă݂�
		htiThis = this->SearchItem(hParent, Path::GetBaseName(sFullPath));
		if(! htiThis){
			// ������Ȃ�������A�t�H���_���ēW�J�����邽�߂ɁA���̃t�H���_�����
			this->Expand(hParent, TVE_COLLAPSE | TVE_COLLAPSERESET);
			// �Ώ��ɂȂ��Ă邩������Ȃ��̂ŁA���Z�b�g
			this->SetChildless(hParent, false);
		}
	}
	if(! htiThis){
		// �e�t�H���_��W�J���Ă�����
		this->Expand(hParent, TVE_EXPAND);
		htiThis = this->SearchItem(hParent, Path::GetBaseName(sFullPath));
	}
	if(! htiThis){
		// ��̃t�H���_�܂ł�valid�̂Ƃ��́A�����I���B
		this->EnsureVisible(hParent);
		this->SetSelection((HTREEITEM)nullptr); // ITEMEXPANDED���b�Z�������I�ɐ���������
		this->SetSelection(hParent);
	}

	return htiThis;
}


LRESULT
ExplorerTree::OnItemExpanding(NMTREEVIEW* pNMTV)
{
	if(this->fState_ & TVS_LoadingSubFolder){
		// ���̃t�H���_�����[�h���Ă���r����������
		// ���̃t�H���_�̃��[�h�𒆎~����
		this->fState_ |= TVS_LoadingCancelled;
		// �����Ă��̃t�H���_���J�����Ƃ͂��Ȃ�
		return 1;
	}

	if(pNMTV->action & TVE_EXPAND){
		// �A�C�e����W�J����O�ɁA�T�u�t�H���_�����[�h
		bool bCancelled;
		DWORD numChildren = this->LoadFolderItem(pNMTV->itemNew.hItem, &bCancelled);

		if(! bCancelled){
			if(numChildren > 0){
				// ���[�h����
				return 0;
			}

			// �q�������Ȃ��Ƃ��́A�c���[�A�C�e���̍��ɂ��� + �}�[�N������
			this->SetChildless(pNMTV->itemNew.hItem);
		}

		// ���s
		return 1;
	}
	else if(pNMTV->action & TVE_COLLAPSE){
		// �������邾�����ƁA��x�W�J�������Ƃ�����A�Ƃ�����񂪃c���[�r���[�ɋL������Ă��܂�
		// ���񂩂�W�J���悤�Ƃ��Ă��AITEMEXPANDING���b�Z�����ł��Ȃ����Ƃ�����
		// �Ȃ̂ŁA�������邾������Ȃ��ă��Z�b�g���߂��ŕ���
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
//		// �A�C�e��������Ƃ��́A�q����S�ĎE��
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
		// �p�X�Ŏw�肷��ق���SetSelection��EnsureVisible���Ă�ł���
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
