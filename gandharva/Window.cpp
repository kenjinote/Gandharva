#include "Window.h"


namespace gandharva
{

ATOM
Window::RegisterClassWrap(
	UINT		style,
	int			cbClsExtra,
	int			cbWndExtra,
	HICON		hIcon,
	HCURSOR		hCursor,
	HBRUSH		hbrBackground,
	LPCWSTR		lpszMenuName,
	LPCWSTR		lpszClassName,
	HICON		hIconSm
){
	WNDCLASSEX wc;
	wc.cbSize		 = sizeof(wc);
	wc.style		 = style;
	wc.lpfnWndProc	 = s_WndProc;
	wc.cbClsExtra	 = cbClsExtra;
	wc.cbWndExtra	 = cbWndExtra;
	wc.hInstance	 = ::GetModuleHandle(nullptr);
	wc.hIcon		 = hIcon;
	wc.hCursor		 = hCursor;
	wc.hbrBackground = hbrBackground;
	wc.lpszMenuName	 = lpszMenuName;
	wc.lpszClassName = lpszClassName;
	wc.hIconSm		 = hIconSm;

	return ::RegisterClassEx(&wc);
}


HWND
Window::CreateWindowWrap(
	DWORD	dwExStyle,
	LPCWSTR	lpClassName,
	LPCWSTR	lpWindowName,
	DWORD	dwStyle,
	int		x,
	int		y,
	int		nWidth,
	int		nHeight,
	HWND	hWndParent,
	HMENU	hMenu
){
	return ::CreateWindowEx(
		dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight,
		hWndParent, hMenu,
		::GetModuleHandle(nullptr), this
	);
}


LRESULT CALLBACK
Window::s_WndProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
){
	Window* pWindow = (Window*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if(! pWindow && uMsg == WM_NCCREATE){
		pWindow = (Window*)((CREATESTRUCT*)lParam)->lpCreateParams;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWindow);
		pWindow->hWnd_ = hWnd;
	}

	if(pWindow) return pWindow->WndProc(hWnd, uMsg, wParam, lParam);
	else        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

} // namespace gandharva
