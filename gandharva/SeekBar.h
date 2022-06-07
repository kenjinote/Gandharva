#ifndef _2508ec11_8ca2_4394_af5c_7d9a8fcd4ac2
#define _2508ec11_8ca2_4394_af5c_7d9a8fcd4ac2

#include "TrackBar.h"
#include <commctrl.h>


namespace gandharva
{

class SeekBar
	: public TrackBar
{
public:
	SeekBar(HWND hwndParent, int iID);
	virtual ~SeekBar(){}

	HWND GetToolTip() const {
		return (HWND)::SendMessage(this->GetHandle(), TBM_GETTOOLTIPS, 0, 0);
	}

	void Enable(BOOL bEnable){
		::EnableWindow(this->GetHandle(), bEnable);
	}

	LRESULT DefOnNotify(NMHDR* pNMH);

private:
	LRESULT OnToolTip(NMTTDISPINFO* pNMTT);
};

} // namespace gandharva
#endif//_2508ec11_8ca2_4394_af5c_7d9a8fcd4ac2
