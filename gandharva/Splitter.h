#ifndef _45ec4e22_8a48_414d_9eca_b2126d8cda31
#define _45ec4e22_8a48_414d_9eca_b2126d8cda31

#include "Window.h"

namespace gandharva
{

class CSplitter
	: public Window
{
public:
	enum { PEL_X_USE_DEFAULT = -1 };
	CSplitter(HWND hwndParent, HWND hwndPane1, HWND hwndPane2, int xSplitter);
	virtual ~CSplitter(){}

	void SetInitialLocation(int xSplitter){
		this->xState_ = xSplitter;
	}

	void DefOnSize(int xSplitter, const RECT* poptRect);
	int GetLocation() const;

private:
	bool CreateSplitter();
	void GetCursorParentCoord(POINT* ppt);

	virtual LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
	
	HWND hwndParent_;
	HWND hwndPane1_;
	HWND hwndPane2_;

	int xDragBegin_;

	// -2 �� �������I���
	// -1 �� �������҂��i�f�t�H���g�̕�������g���j
	// ����ȊO ���������҂��ixState_ ���ŏ��̕�����j
	int xState_;

	RECT rectTotal_;

	enum {
		PEL_SPLITTER = 4
	};
};

} // namespace gandharva
#endif//_45ec4e22_8a48_414d_9eca_b2126d8cda31
