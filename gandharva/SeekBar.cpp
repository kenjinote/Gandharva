#include "SeekBar.h"
#include "LanguageUtil.h"


namespace gandharva
{

SeekBar::SeekBar(HWND hwndParent, int iID)
	: TrackBar(hwndParent, iID,
		WS_CHILD | WS_VISIBLE | WS_DISABLED |
		TBS_NOTICKS | TBS_BOTH | TBS_HORZ | TBS_TOOLTIPS |
		CCS_NODIVIDER
	  )
{}


LRESULT
SeekBar::DefOnNotify(NMHDR* pNMH)
{
	switch(pNMH->code){
	case TTN_NEEDTEXT:
		return this->OnToolTip((NMTTDISPINFO*)pNMH);
	}

	return 0;
}


LRESULT
SeekBar::OnToolTip(NMTTDISPINFO* pNMTT)
{
	pNMTT->hinst = nullptr;
	pNMTT->uFlags = 0;

	std::wstring str = TimeToString(this->GetPosition());

	::lstrcpynW(pNMTT->szText, str.c_str(), lengthof(pNMTT->szText));
	pNMTT->szText[ lengthof(pNMTT->szText) ] = L'\0';

	return 0;
}

} // namespace gandharva
