#ifndef _4877e502_1a62_4b4e_962b_e041a007545e
#define _4877e502_1a62_4b4e_962b_e041a007545e

#include "MyWindows.h"
#include "Window.h"
#include "ExplorerTree.h"
#include "FileListView.h"
#include "Splitter.h"
#include "Folder.h"
#include "Rebar.h"
#include "ToolBar.h"
#include "SeekBar.h"
#include "VolumeBar.h"
#include "StatusBar.h"


#include <memory>
#include <string>

namespace gandharva
{

class MainWindow : public Window
{
public:
	MainWindow();
	virtual ~MainWindow(){}

	void RestorePrevState();

private:
	virtual LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnCreate(HWND);
	LRESULT OnDestroy();
	LRESULT OnSize(WPARAM, LPARAM);
	LRESULT OnTimer();
	LRESULT OnHScroll(WPARAM, LPARAM);
		LRESULT OnSeekBarScroll(DWORD dwNotification);
	LRESULT OnNotify(LPARAM);
		LRESULT OnTreeSelChanged(NMTREEVIEW* pNMTV);
		LRESULT OnListDBLClick(NMITEMACTIVATE* pNMIA);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
		LRESULT OnPlay();
		LRESULT OnTogglePause();
		LRESULT OnStop();
		LRESULT OnGoPrev();
		LRESULT OnGoNext();
		LRESULT OnToggleRandom();
		LRESULT OnRepeatOption();
		LRESULT OnVolumeButton();
		LRESULT OnFavoriteButton();
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnDropFiles(WPARAM wParam);

	LRESULT OnEndOfSound();
	LRESULT OnVolumeChanged(LPARAM lParam);
	LRESULT OnShowPlayingFile();
	LRESULT OnIsPlaying();

	bool BeginPlay(int iLVItem);
	bool EndPlay();

	void LoadContents(HTREEITEM hti);

	bool SelectFile(std::wstring const& path);
	bool SelectFile(std::wstring const& folderSpec, std::wstring const& fileSpec);

	std::unique_ptr<ExplorerTree>	pExplorerTree_;
	std::unique_ptr<FileListView>	pFileList_;
	std::unique_ptr<CSplitter>		pSplitter_;

	std::unique_ptr<Rebar>			pRebar_;
	std::unique_ptr<ToolBar>		pToolBar_;
	std::unique_ptr<SeekBar>		pSeekBar_;
	std::unique_ptr<VolumeBar>		pVolumeBar_;
	std::unique_ptr<StatusBar>		pStatusBar_;

	Folder::P						pFolder_;
	Player::P						pPlayer_;

	CTimer							timer_;
};

} // namespace gandharva
#endif//_4877e502_1a62_4b4e_962b_e041a007545e
