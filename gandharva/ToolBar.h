#ifndef _efe270bf_91f9_4638_8f9f_5abc8ce19a4f
#define _efe270bf_91f9_4638_8f9f_5abc8ce19a4f

#include "MyWindows.h"
#include <commctrl.h>

namespace gandharva
{

class ToolBar
{
public:
	enum {
		TBID_Play,
		TBID_Pause,
		TBID_Stop,
		TBID_GoPrev,
		TBID_GoNext,
		TBID_Random,
		TBID_Repeat,
		TBID_Volume,
		TBID_Favorite
	};

	ToolBar(HWND hwndParent, int iIDOffset);

	HWND GetHandle() const { return this->hwndThis_; }
	HWND GetToolTip() const {
		return (HWND)::SendMessage(this->hwndThis_, TB_GETTOOLTIPS, 0, 0);
	}

	// スクリーン座標での値を返す
	void GetRect(RECT* pRect);

	// クライアント座標を返す
	void GetItemRect(int iTBID, RECT* pRect);

	bool IsToggleOn(int iTBID);
	void SetToggle(int iTBID, BOOL bOn);

	enum {
		REPEAT_Begin,
		REPEAT_None = REPEAT_Begin,
		REPEAT_List,
		REPEAT_Single,
		REPEAT_End
	};
	int  GetRepeatOption(){ return iRepeatOption_; }
	void SetRepeatOption(int iRepeat);

	LRESULT DefOnNotify(NMHDR* pNMH);

private:
	LRESULT OnToopTip(NMTTDISPINFO* pNMTTDI);

	HWND      hwndThis_;
	HWND      hwndParent_;
	int       iRepeatOption_;
	const int iIDOffset_;
};

} // namespace gandharva
#endif//_efe270bf_91f9_4638_8f9f_5abc8ce19a4f
