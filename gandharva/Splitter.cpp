#include "Splitter.h"
#include "MyWindows.h"

namespace gandharva
{

CSplitter::CSplitter(HWND hwndParent, HWND hwndPane1, HWND hwndPane2, int xSplitter)
	: hwndParent_(hwndParent),
	  hwndPane1_(hwndPane1), hwndPane2_(hwndPane2),
	  xDragBegin_(-1), xState_(xSplitter)
{
	ZeroMemory(&this->rectTotal_, sizeof(this->rectTotal_));
	this->CreateSplitter();
}


void 
CSplitter::DefOnSize(int xSplitter, const RECT* poptRect)
{
	if(poptRect){
		this->rectTotal_ = *poptRect;
	}

	int height = rectTotal_.bottom - rectTotal_.top;
	int widthTotal = rectTotal_.right - rectTotal_.left;

	if(this->xState_ == PEL_X_USE_DEFAULT){
		// �f�t�H���g�l�ŏ������҂��̂Ƃ�
		xSplitter = 2 * widthTotal / 5;
		this->xState_ = -2;
	}
	else if(this->xState_ >= 0){
		// �����ʒu xState_ �ŏ������҂��̂Ƃ�
		xSplitter = this->xState_;
		this->xState_ = -2;
	}

	if(widthTotal >= PEL_SPLITTER + 50){
		// �e�E�B���h�E�̑傫���͏\��������
		if(xSplitter >= rectTotal_.right - PEL_SPLITTER - 30){
			// �X�v���b�^�̈ʒu���E�ɂ��߂��̎��́A�O�a����
			xSplitter = rectTotal_.right - PEL_SPLITTER - 30;
		}
		if(xSplitter <= rectTotal_.left + 20){
			// �X�v���b�^�̈ʒu�����ɂ��߂��̎��́A�O�a����
			xSplitter = rectTotal_.left + 20;
		}
	}
	else{
		// �e�E�B���h�E������������Ƃ�
		xSplitter = 2 * widthTotal / 5;
	}

	// ���y�C���̕�
	int width1 = xSplitter - rectTotal_.left;
		if(width1 < 0) width1 = 0;

	// �E�y�C���̕�
	int width2 = rectTotal_.right - PEL_SPLITTER - xSplitter;
		if(width2 < 0) width2 = 0;

	// �E�B���h�E�̕`����~�߂�
	CLockWindowUpdate lock(this->hwndParent_);
		// �E�B���h�E�̈ʒu��ς���
		::MoveWindow(hwndPane1_,
			rectTotal_.left, rectTotal_.top, width1, height, TRUE
		);
		::MoveWindow(hwndPane2_,
			xSplitter + PEL_SPLITTER, rectTotal_.top, width2, height, TRUE
		);
		::MoveWindow(this->GetHandle() ,
			xSplitter, rectTotal_.top, PEL_SPLITTER, height, TRUE
		);
}


int
CSplitter::GetLocation() const
{
	RECT rect;
	::GetWindowRect(this->GetHandle(), &rect);

	POINT pt;
	pt.x = rect.left;
	pt.y = rect.top;
	::ScreenToClient(hwndParent_, &pt);

	return pt.x;
}


bool
CSplitter::CreateSplitter()
{
	ATOM atom = RegisterClassWrap(
		CS_HREDRAW | CS_VREDRAW,
		0, 0,
		nullptr,
		::LoadCursor(nullptr, IDC_SIZEWE),
		(HBRUSH)(COLOR_BTNFACE + 1),
		nullptr,
		TEXT("gandharva-Splitter"),
		nullptr
	);

	if(! atom) return false;

	this->CreateWindowWrap(
		WS_EX_WINDOWEDGE,
		(LPCTSTR)atom,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwndParent_,
		nullptr
	);

	// ����this->GetHandle() ���L���ɂȂ�
	if (! this->GetHandle()) return false;

	return true;
}


void
CSplitter::GetCursorParentCoord(POINT* ppt)
{
	::GetCursorPos(ppt);
	::ScreenToClient(hwndParent_, ppt);
}


LRESULT
CSplitter::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_LBUTTONDOWN:
		if(this->xDragBegin_ < 0){
			POINT pt;
			this->GetCursorParentCoord(&pt);

			this->xDragBegin_ = pt.x;
			::SetCapture(hWnd);
		}
		return 0;

	case WM_LBUTTONUP:
		if(this->xDragBegin_ >= 0){
			int xDragBegin = this->xDragBegin_;
			this->xDragBegin_ = -1;
			::ReleaseCapture();

			POINT pt;
			this->GetCursorParentCoord(&pt);

			// �X�v���b�^��͂񂾈ʒu���l�����ĕ␳
			this->DefOnSize(this->GetLocation() + pt.x - xDragBegin, nullptr);
		}
		return 0;
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

} // namespace gandharva
