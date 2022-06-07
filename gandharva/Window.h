#ifndef _348ae194_d584_462e_9fa7_bc1f296f666b
#define _348ae194_d584_462e_9fa7_bc1f296f666b

#include "MyWindows.h"

namespace gandharva
{

class Window
{
public:
	Window(): hWnd_(nullptr) {}
	virtual ~Window(){}

	HWND GetHandle() const { return hWnd_; }

protected:
	virtual LRESULT WndProc(HWND, UINT, WPARAM, LPARAM) = 0;

	static ATOM RegisterClassWrap(
		UINT		style,
		int			cbClsExtra,
		int			cbWndExtra,
		HICON		hIcon,
		HCURSOR		hCursor,
		HBRUSH		hbrBackground,
		LPCWSTR		lpszMenuName,
		LPCWSTR		lpszClassName,
		HICON		hIconSm
	);

	HWND CreateWindowWrap(
		DWORD	dwExStyle,
		LPCWSTR	lpClassName,
		LPCWSTR	lpWindowName,
		DWORD	dwStyle,
		int		X,
		int		Y,
		int		nWidth,
		int		nHeight,
		HWND	hWndParent,
		HMENU	hMenu
	);

private:
	static LRESULT CALLBACK s_WndProc(
		HWND, UINT, WPARAM, LPARAM
	);

	HWND hWnd_;
};

} // namespace gandharva
#endif//_348ae194_d584_462e_9fa7_bc1f296f666b
