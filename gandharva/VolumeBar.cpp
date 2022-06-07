#include "VolumeBar.h"
#include <commctrl.h>

namespace gandharva
{

VolumeBar::VolumeBar(HWND hwndParent, UINT uMsgVolume)
	: pTrackBar_(nullptr),
	  hwndParent_(hwndParent), uMsgVolume_(uMsgVolume), mBVolume_(0)
{
	ATOM atom = RegisterClassWrap(
		CS_HREDRAW | CS_VREDRAW /*| CS_DROPSHADOW*/,
		0, 0,
		nullptr,
		::LoadCursor(nullptr, IDC_ARROW),
		(HBRUSH)(COLOR_BTNFACE + 1),
		nullptr,
		TEXT("gandharva-Volume"),
		nullptr
	);

	this->CreateWindowWrap(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
		(LPTSTR)atom, nullptr,
		WS_POPUP | WS_CLIPCHILDREN,
		0, 0, 0, 0,
		hwndParent,
		nullptr
	);
}


void
VolumeBar::Show(int x, int y, int width, int height)
{
	this->SetVolume(this->mBVolume_);
	::SetWindowPos(
		this->GetHandle(),
		HWND_TOPMOST,
		x, y, width, height,
		SWP_SHOWWINDOW
	);		
}


long VolumeBar::GetVolume()
{
	return mBVolume_;
}


void VolumeBar::SetVolume(long mBVolume)
{
	// �{�����[���� -10000 �~���x�� ���� 0 �܂ł͈̔�
	if(0 < mBVolume) mBVolume = 0;
	else if(mBVolume < -10000) mBVolume = -10000;

	// -10000 �~���x���t�߂́A�������Ƃ���͂ǂ����������Ȃ��B
	// ���[�U�[�́A�t�����ʂ̕ӂ� (0 �t��) �Ŕ������������͂��Ȃ̂�
	// �g���b�N�o�[�̂܂݂́A�t�����ʂ�����̂ق������ݕ����������Ȃ�悤�ɂ���B

	// [-10000, 0] -> [0, 100]
	long pos = ((mBVolume + 10000)*(mBVolume + 10000) + 500000) / 1000000;

	pTrackBar_->SetPosition(pos);
	mBVolume_ = mBVolume;
}

LRESULT
VolumeBar::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CREATE:
		return this->OnCreate(hWnd);
	case WM_ACTIVATE:
		return this->OnActivate(hWnd, wParam);
	case WM_SIZE:
		return this->OnSize(hWnd);
	case WM_HSCROLL:
		return this->OnHScroll(wParam, lParam);
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


LRESULT
VolumeBar::OnCreate(HWND hWnd)
{
	pTrackBar_.reset(new TrackBar(hWnd, 0,
		WS_CHILD | WS_VISIBLE |
		TBS_NOTICKS | TBS_BOTTOM | TBS_HORZ | TBS_TOOLTIPS |
		CCS_NODIVIDER
	));

	pTrackBar_->SetRange(0, 100);
	return 0;
}


LRESULT
VolumeBar::OnActivate(HWND hWnd, WPARAM wParam)
{
	if(LOWORD(wParam) == WA_INACTIVE){
		::ShowWindow(hWnd, SW_HIDE);
	}
	return 0;
}


LRESULT
VolumeBar::OnSize(HWND hWnd)
{
	RECT rect;
	::GetClientRect(this->GetHandle(), &rect);
	::MoveWindow(pTrackBar_->GetHandle(),
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		TRUE
	);

	return 0;
}


LRESULT
VolumeBar::OnHScroll(WPARAM wParam, LPARAM lParam)
{
	auto Sqrt = [](unsigned long x) -> unsigned long
	{
		if(x == 0) return x;
		unsigned long s = 1;
		unsigned long t = x;

		// �Ƃ肠�����̋ߎ��l
		while(s < t){
			s *= 2;
			t /= 2;
		}

		// �j���[�g���ߎ�
		do {
			t = s;
			s = (x / s + s) / 2;
		}
		while(s < t);

		return t;
	};


	if(pTrackBar_ &&
		pTrackBar_->GetHandle() == (HWND)lParam)
	{
		// �܂� pos �Ɖ��� mBVolume �̊֌W�ɂ��Ă�
		// SetVolume �̒��߂��Q�Ƃ̂���

		long pos = pTrackBar_->GetPosition();
		if(pos < 0) pos = 0;
		if(100 < pos) pos = 100;

		// [0, 100] -> [-10000, 0]
		mBVolume_ = (long)Sqrt(1000000 * pos) - 10000;

		::SendMessage(
			this->hwndParent_,
			this->uMsgVolume_,
			0, mBVolume_
		);
		return 0;
	}

	return 0;
}

} // namespace gandharva
