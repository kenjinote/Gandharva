#ifndef _4cbf25bb_b15a_4d56_962e_6eefbaafac44
#define _4cbf25bb_b15a_4d56_962e_6eefbaafac44

#define ID_APPICON			10
#define ID_TOOLBARBMP		100

// ToolBar ���̃{�^�� ID �̃I�t�Z�b�g
#define IDTB_OFFSET			1000

// Player �� hwndNotify �֓�����
#define MY_WM_DSHOWNOTIFY	(WM_APP + 0)
// Player �� hwndNotify �֓�����
#define MY_WM_ENDOFSOUND	(WM_APP + 1)

// VolumeBar ���e�E�B���h�E�֑��� lParam = mBVolume
#define MY_WM_VOLUME		(WM_APP + 2)

// CMainWindow �֑���ƁA���t���̃t�@�C����\������
#define MY_WM_SHOWPLAYFILE	(WM_APP + 3)

// CMainWindow �֑���ƁA���t�����ǂ����� BOOL �ŕԂ�
#define MY_WM_ISPLAYING		(WM_APP + 4)

#endif//_4cbf25bb_b15a_4d56_962e_6eefbaafac44
