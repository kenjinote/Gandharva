#include "StatusBar.h"
#include "LanguageUtil.h"
#include <commctrl.h>

namespace gandharva
{

StatusBar::StatusBar(HWND hwndParent)
{
	this->hwndThis_ = ::CreateWindowEx(
		0,
		STATUSCLASSNAME,
		nullptr,
		WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
		0, 0, 0, 0,
		hwndParent,
		nullptr,
		::GetModuleHandle(nullptr),
		nullptr
	);

	const int widths[] = {0, 0};
	::SendMessage(this->hwndThis_, SB_SETPARTS, 2, (LPARAM)widths);
}


LRESULT
StatusBar::DefOnSize(WPARAM wParam, LPARAM lParam)
{
	int widths[2];
	widths[0] = (int)LOWORD(lParam) - 200;
	if(widths[0] < 0) widths[0] = 0;
	widths[1] = -1;

	::SendMessage(this->hwndThis_, SB_SETPARTS, 2, (LPARAM)widths);
	return ::SendMessage(this->hwndThis_, WM_SIZE, wParam, lParam);
}


void
StatusBar::SetTime(long secPlayed)
{
	std::wstring str;

	if(! this->sTotal_.empty()){
		str = TimeToString(secPlayed) + L" / " + this->sTotal_;
	}

	::SendMessage(this->hwndThis_, SB_SETTEXT, 1, (LPARAM)str.c_str());
}


void
StatusBar::SetTotalTime(long secTotal)
{
	if(secTotal < 0){
		this->sTotal_.clear();
	}
	else{
		this->sTotal_ = TimeToString(secTotal);
	}

	this->SetTime(0);
}


void
StatusBar::SetTitle(std::wstring const& title)
{
	::SendMessage(this->hwndThis_, SB_SETTEXT, 0, (LPARAM)title.c_str());
}

} // namespace gandharva
