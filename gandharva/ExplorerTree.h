#ifndef _150fa369_863c_456f_be8a_4dd2114fe0e4
#define _150fa369_863c_456f_be8a_4dd2114fe0e4

#include "MyWindows.h"
#include <commctrl.h>

#include <string>

namespace gandharva
{

class ExplorerTree
{
public:
	ExplorerTree(HWND hwndParent, int iID);

	HWND GetHandle() const { return hwndThis_; }

	// �ŏ��ɃR�����Ăяo���āA�{�����[����\�����Ă������ƁB
	// �Ⴆ�� A:\ �� C:\ ��\���������Ȃ��Ȃ�
	//   fHiddenVolumes = (1 << ('A' - 'A')) | (1 << ('C' - 'A'));
	void CreateRoot(DWORD fHiddenVolumes);

	DWORD GetHiddenVolumes() const { return this->fHiddenVolumes_; }

	// htiFolder �̒��g���c���[�ɓo�^
	// �߂�l�F�T�u�t�H���_�̐�
	DWORD LoadFolderItem(HTREEITEM htiFolder, bool* pbCancelled);

	// lpFolder�̒��g���c���[�ɓo�^���āAlpoptSubFolder ��T���B
	// �߂�l�F�T�u�t�H���_�̐�
	DWORD
	LoadFolderItem(
		HTREEITEM           htiFolder,
		std::wstring const& sFolderPath,
		std::wstring const& soptSubFolderTitle,
		HTREEITEM*          popt_htiSubFolder,
		bool              * pbCancelled
	);

	std::wstring GetItemFullPath(HTREEITEM hItem);

	std::wstring GetText(HTREEITEM hItem);

	HTREEITEM InsertItem(
		HTREEITEM           hParent,
		HTREEITEM           hInsertAfter,
		std::wstring const& sFullPath,
		std::wstring const& sText,
		LPARAM              lParam = 0
	);

	BOOL DeleteItem(HTREEITEM hItem){
		return TreeView_DeleteItem(hwndThis_, hItem);
	}

	void DeleteAllChildren(HTREEITEM hItem);

	LPARAM GetParam(HTREEITEM hItem);
	void SetParam(HTREEITEM hItem, LPARAM lParam);

	HTREEITEM SearchItem(HTREEITEM htiParent, std::wstring const& sItem);

	void Expand(HTREEITEM hItem, WPARAM wTVE);
	// �W�J����Ă邩�A�Ώ��ɂȂ��Ă���Ƃ� true
	bool IsExpanded(HTREEITEM hItem);

	void EnsureVisible(HTREEITEM hItem){
		TreeView_EnsureVisible(hwndThis_, hItem);
	}

	void SetSelection(HTREEITEM hItem){
		TreeView_SelectItem(hwndThis_, hItem);
	}

	// path ���\�������Ƃ���܂Ńc���[���쐬���A�����I��
	bool SetSelection(std::wstring const& path);

	HTREEITEM GetSelection() const {
		return TreeView_GetSelection(hwndThis_);
	}

	// �c���[�A�C�e���̍��� + �}�[�N������
	void SetChildless(HTREEITEM hItem, bool bChildless = true);

	// ���C�ɓ�������[�g�ɒǉ�
	void AddFavorite(std::wstring const& path);

	// ���C�ɓ���Œǉ����ꂽ���̂�����
	BOOL IsFavorite(HTREEITEM hItem){
		return this->GetParam(hItem) & TVItem_Removable;
	}

	// �ŐV�̏��ɍX�V
	void Renew();

	HTREEITEM GetParent(HTREEITEM hItem);

	HTREEITEM GetFirstChild(HTREEITEM hItem){
		return TreeView_GetChild(hwndThis_, hItem);
	}

	HTREEITEM GetNextSibling(HTREEITEM hItem){
		return TreeView_GetNextSibling(hwndThis_, hItem);
	}

	HTREEITEM GetPrevSibling(HTREEITEM hItem){
		return TreeView_GetPrevSibling(hwndThis_, hItem);
	}

	HTREEITEM HitTest(int x, int y, bool bScreen);
	void GetItemRect(HTREEITEM hItem, RECT* prc, bool bScreen);

	LRESULT DefOnNotify(NMHDR* pNMH);
	LRESULT DefOnContextMenu(LPARAM lParam);

private:
	HTREEITEM SetSelectionHelper(std::wstring const& sFullPath);

	LRESULT OnItemExpanding(NMTREEVIEW* pNMTV);
	LRESULT OnItemExpanded(NMTREEVIEW* pNMTV);

	void OnContextMenu_GoParent(HTREEITEM hParent, std::wstring const& sParent);
	void OnContextMenu_Explorer(HTREEITEM hItem);
	void OnContextMenu_RemoveFromFavorite(HTREEITEM hItem);
	void OnContextMenu_AddToFavorite(HTREEITEM hItem);

	HWND	hwndThis_;
	HWND	hwndParent_;
	int		iID_;

	enum ETVState {
		TVS_LoadingSubFolder = 0x01,
		TVS_LoadingCancelled = 0x02
	};
	volatile unsigned fState_;

	enum {
		TVItem_Removable = 0x01
	};

	// �B�����{�����[��
	DWORD fHiddenVolumes_;
};

} // namespace gandharva
#endif//_150fa369_863c_456f_be8a_4dd2114fe0e4
