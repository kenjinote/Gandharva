#ifndef _4cbf25bb_b15a_4d56_962e_6eefbaafac44
#define _4cbf25bb_b15a_4d56_962e_6eefbaafac44

#define ID_APPICON			10
#define ID_TOOLBARBMP		100

// ToolBar 内のボタン ID のオフセット
#define IDTB_OFFSET			1000

// Player が hwndNotify へ投げる
#define MY_WM_DSHOWNOTIFY	(WM_APP + 0)
// Player が hwndNotify へ投げる
#define MY_WM_ENDOFSOUND	(WM_APP + 1)

// VolumeBar が親ウィンドウへ送る lParam = mBVolume
#define MY_WM_VOLUME		(WM_APP + 2)

// CMainWindow へ送ると、演奏中のファイルを表示する
#define MY_WM_SHOWPLAYFILE	(WM_APP + 3)

// CMainWindow へ送ると、演奏中かどうかが BOOL で返る
#define MY_WM_ISPLAYING		(WM_APP + 4)

#endif//_4cbf25bb_b15a_4d56_962e_6eefbaafac44
