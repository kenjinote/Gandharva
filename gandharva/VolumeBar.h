#ifndef _c3a1056d_e564_438c_82fa_2bc118e93cee
#define _c3a1056d_e564_438c_82fa_2bc118e93cee

#include "TrackBar.h"
#include "Window.h"
#include <memory>

namespace gandharva
{

class VolumeBar
	: public Window
{
public:
	VolumeBar(HWND hwndParent, UINT uMsgVolume);
	virtual ~VolumeBar(){}

	void Show(int x, int y, int width, int height);

	long GetVolume();
	void SetVolume(long mBVolume);

private:
	virtual LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnActivate(HWND hWnd, WPARAM wParam);
	LRESULT OnSize(HWND hWnd);
	LRESULT OnHScroll(WPARAM, LPARAM);

	std::unique_ptr<TrackBar> pTrackBar_;
	HWND                      hwndParent_;
	const UINT                uMsgVolume_;
	long                      mBVolume_;
};

} // namespace gandharva

#endif//_c3a1056d_e564_438c_82fa_2bc118e93cee
