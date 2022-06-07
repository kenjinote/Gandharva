#include "TrackBar.h"
#include <commctrl.h>

namespace gandharva
{

TrackBar::TrackBar(HWND hwndParent, int iID, DWORD dwStyle)
{
	this->hwndThis_ = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		TRACKBAR_CLASS,
		nullptr,
		dwStyle,
		0, 0, 0, 0,
		hwndParent,
		(HMENU)iID,
		::GetModuleHandle(nullptr),
		nullptr
	);
}


void
TrackBar::SetRange(long min, long max)
{
	::SendMessage(this->hwndThis_, TBM_SETRANGEMIN, FALSE, min);
	::SendMessage(this->hwndThis_, TBM_SETRANGEMAX, TRUE, max);
//	::SendMessage(this->hwndThis_, TBM_SETPAGESIZE, 0, (max - min) / 10);
	::SendMessage(this->hwndThis_, TBM_SETPAGESIZE, 0, 1);
}


void
TrackBar::SetPosition(long x)
{
	::SendMessage(this->hwndThis_, TBM_SETPOS, TRUE, x);
}


long
TrackBar::GetPosition()
{
	return (long)::SendMessage(this->hwndThis_, TBM_GETPOS, 0, 0);
}

} // namespace gandharva
