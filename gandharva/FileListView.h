#ifndef _3c84e25e_e586_4456_8ca1_0f0b6188925f
#define _3c84e25e_e586_4456_8ca1_0f0b6188925f

#include "MyWindows.h"
#include "Folder.h"
#include "Random.h"
#include <commctrl.h>

namespace gandharva
{

class FileListView
{
public:
	FileListView(HWND hwndParent, int iID);
	HWND GetHandle() const { return this->hwndThis_; }

	enum {
		ICOL_Begin,
		ICOL_Title = ICOL_Begin,
		ICOL_Size,
		ICOL_Extension,
		ICOL_Date,
		ICOL_End
	};

	struct SColumnWidth
	{
		int acx[ICOL_End - ICOL_Begin];
	};
	void SetColumnWidth(const SColumnWidth& cw);
	void GetColumnWidth(SColumnWidth* pcw);

	struct SSortInfo
	{
		HWND hwndLV;
		int iSubItem;
		bool bAscend;
	};

	void LoadFolderContents(Folder::P const& pFolder);
	void SetRandom(bool bRandom);

	std::wstring GetItemText(int iItem, int iSubItem);

	int GetSelectedItem();
	// à»â∫ÇÃÇQÇ¬ÇÕÅAEnsureVisible Çä‹ÇﬁÅB
	void SetSelectedItem(int iItem);
	bool SetSelectedItem(std::wstring const& title);

	void EnsureVisible(int iItem);

	int GetItemCount();

	LRESULT DefOnNotify(NMHDR* pNMH);

private:
	void CreateColumn();

	LRESULT OnColumnClick(NMITEMACTIVATE* pNMIA);
	void SortItems();

	std::vector<std::wstring> filters_;

	bool      bLoadingFolder_;

	HWND	  hwndThis_;
	HWND	  hwndParent_;
	int		  iID_;

	SSortInfo sortInfo_;
	bool      bRandom_;

	Random    rand_;
};

} // namespace gandharva
#endif//_3c84e25e_e586_4456_8ca1_0f0b6188925f
