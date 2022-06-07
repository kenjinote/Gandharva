#include "Rebar.h"
#include <commctrl.h>


namespace gandharva
{

Rebar::Rebar(HWND hwndParent)
{
	hwndThis_ = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		REBARCLASSNAME,
		nullptr,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		RBS_BANDBORDERS | RBS_FIXEDORDER,
		0, 0, 0, 0,
		hwndParent,
		nullptr,
		::GetModuleHandle(nullptr),
		nullptr
	);
}


void
Rebar::InsertBand(HWND hWnd, int cx, int cxMin, int cy, int iInsertAt)
{
	REBARBANDINFO ri = {};
	ri.cbSize = sizeof(ri);
	ri.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
	ri.fStyle = /*RBBS_CHILDEDGE |*/ RBBS_NOGRIPPER | RBBS_NOVERT;
	ri.hwndChild  = hWnd;
	ri.cxMinChild = cxMin;
	ri.cyMaxChild = cy;
	ri.cyMinChild = cy;
	// ri.cyChild    =;
	// ri.cyIntegral =;
	ri.cx         = cx;
	::SendMessage(this->hwndThis_, RB_INSERTBAND, (WPARAM)iInsertAt, (LPARAM)&ri);
}


void
Rebar::DefOnSize(WPARAM wParam, LPARAM lParam)
{
	::SendMessage(this->hwndThis_, WM_SIZE, wParam, lParam);
}

} // namespace gandharva
