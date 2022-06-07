#include "ToolBar.h"
#include "MyWindows.h"
#include "resource.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")


namespace gandharva
{

namespace {
	enum {
		gNUM_TBICONS   = 11,
		gNUM_TBBUTTONS = 11,
		gIX_ICON_REPEAT = 6 // the index of icon "REPEAT"
	};
}


ToolBar::ToolBar(HWND hwndParent, int iIDOffset)
	: iIDOffset_(iIDOffset), iRepeatOption_(REPEAT_None)
{
	this->hwndParent_ = hwndParent;

	// �c�[���o�[�쐬
	this->hwndThis_ = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		TOOLBARCLASSNAME,
		nullptr,
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS |
		CCS_NOPARENTALIGN /*| CCS_NODIVIDER*/,
		0, 0, 0, 0,
		hwndParent,
		nullptr,
		::GetModuleHandle(nullptr),
		nullptr
	);

	// ����͂���
	::SendMessage(hwndThis_, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

//	// �g���X�^�C��
//	::SendMessage(hwndThis_, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

	// �{�^���p�̃r�b�g�}�b�v��ݒ�
	HBITMAP hBMP = ::LoadBitmap(::GetModuleHandle(nullptr), MAKEINTRESOURCE(ID_TOOLBARBMP));
	HIMAGELIST hIL = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, gNUM_TBICONS);
	ImageList_AddMasked(hIL, hBMP, RGB(0, 255, 0));
	::SendMessage(hwndThis_, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
	::SendMessage(hwndThis_, TB_SETIMAGELIST, gNUM_TBICONS, (LPARAM)hIL);

	// �{�^�����쐬
	const TBBUTTON aTBB[] = {
		{0, iIDOffset + TBID_Play	, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0},
		{1, iIDOffset + TBID_Pause	, TBSTATE_ENABLED, BTNS_CHECK , 0, 0},
		{2, iIDOffset + TBID_Stop	, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0},
		{3, iIDOffset + TBID_GoPrev	, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0},
		{4, iIDOffset + TBID_GoNext	, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0},
		{	8, 0					, 0				 , BTNS_SEP	  , 0, 0},
		{5, iIDOffset + TBID_Random	, TBSTATE_ENABLED, BTNS_CHECK, 0, 0},
		{6, iIDOffset + TBID_Repeat	, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0},
		{9, iIDOffset + TBID_Volume	, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0},
		{	8, 0					, 0				 , BTNS_SEP	  , 0, 0},
		{10, iIDOffset + TBID_Favorite,TBSTATE_ENABLED, BTNS_BUTTON, 0, 0}
	};
	::SendMessage(hwndThis_, TB_ADDBUTTONS, gNUM_TBBUTTONS, (LPARAM)&aTBB);

	// �T�C�Y�𒲐�
	::SendMessage(hwndThis_, TB_AUTOSIZE, 0, 0);
}


void
ToolBar::GetRect(RECT *pRect)
{
	// �Ƃ肠�����S�̂��擾
	::GetWindowRect(this->hwndThis_, pRect);

	// �Ō�̃A�C�e���̍��W���擾
	RECT rtLast;
	::SendMessage(this->hwndThis_, TB_GETITEMRECT, gNUM_TBBUTTONS - 1, (LPARAM)&rtLast);

	// �X�N���[�����W�ɕϊ�
	POINT pt = {rtLast.right, rtLast.bottom};
	::ClientToScreen(this->hwndThis_, &pt);

	// �{�^�������݂���͈͂�ݒ�
	pRect->right = pt.x;
}


void
ToolBar::GetItemRect(int iTBID, RECT* pRect)
{
	::SendMessage(
		this->hwndThis_,
		TB_GETRECT,
		this->iIDOffset_ + iTBID,
		(LPARAM)pRect
	);
}


bool
ToolBar::IsToggleOn(int iTBID)
{
	return ::SendMessage(
		this->hwndThis_,
		TB_ISBUTTONCHECKED,
		this->iIDOffset_ + iTBID,
		0
	) != FALSE;
}


void
ToolBar::SetToggle(int iTBID, BOOL bOn)
{
	::SendMessage(
		this->hwndThis_,
		TB_CHECKBUTTON,
		this->iIDOffset_ + iTBID,
		MAKELONG(bOn, 0)
	);
}


void
ToolBar::SetRepeatOption(int iRepeat)
{
	this->iRepeatOption_ = iRepeat;

	CLockWindowUpdate lock(this->hwndThis_);

	::SendMessage(
		this->hwndThis_,
		TB_CHANGEBITMAP,
		this->iIDOffset_ + TBID_Repeat,
		MAKELPARAM(gIX_ICON_REPEAT + iRepeat, 0)
	);

	// Vista �΍�F
	// �������ł̓A�C�R����������Ə㏑������Ȃ��̂�
	// ��x hot item �ɂ��āA�摜���X�V����
	::SendMessage(
		this->hwndThis_,
		TB_SETHOTITEM,
		this->iIDOffset_ + TBID_Repeat,
		0
	);

	::SendMessage(
		this->hwndThis_,
		TB_SETHOTITEM,
		(WPARAM)(-1),
		0
	);

}


LRESULT
ToolBar::DefOnNotify(NMHDR* pNMH)
{
	switch(pNMH->code){
	case TTN_NEEDTEXT:
		return this->OnToopTip((NMTTDISPINFO*)pNMH);
	}

	return 0;
}


LRESULT
ToolBar::OnToopTip(NMTTDISPINFO* pNMTTDI)
{
	pNMTTDI->hinst = nullptr;
	pNMTTDI->uFlags = 0;

	LPCTSTR lpText = 0;

	switch(pNMTTDI->hdr.idFrom - this->iIDOffset_){
	case TBID_Play:
		lpText = TEXT("�Đ�");
		break;
	case TBID_Pause:
		lpText = this->IsToggleOn(TBID_Pause) ? TEXT("�ĊJ") : TEXT("�ꎞ��~");
		break;
	case TBID_Stop:
		lpText = TEXT("��~");
		break;
	case TBID_GoPrev:
		lpText = TEXT("�O�̋�");
		break;
	case TBID_GoNext:
		lpText = TEXT("���̋�");
		break;
	case TBID_Random:
		lpText = TEXT("�����_��");
		break;
	case TBID_Repeat:
		switch(iRepeatOption_){
		case REPEAT_None:
			lpText = TEXT("���s�[�g�Ȃ�");
			break;
		case REPEAT_List:
			lpText = TEXT("���s�[�g�i���X�g�j");
			break;
		case REPEAT_Single:
			lpText = TEXT("���s�[�g�i�P�ȁj");
			break;
		}
		break;
	case TBID_Volume:
		lpText = TEXT("����");
		break;
	case TBID_Favorite:
		lpText = TEXT("���C�ɓ���");
		break;
	}

	if(lpText) ::lstrcpy(pNMTTDI->szText, lpText);
	return 0;
}

} // namespace gandharva
