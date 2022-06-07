#include "MainWindow.h"
#include "FolderDir.h"
#include "FolderZip.h"
#include "FolderCue.h"
#include "Profile.h"
#include "FilePath.h"
#include "resource.h"

#include <shellapi.h>


namespace gandharva
{

MainWindow::MainWindow()
	: timer_(1)
{
	HICON hIcon = ::LoadIcon(::GetModuleHandle(nullptr), MAKEINTRESOURCE(ID_APPICON));
	ATOM atom = RegisterClassWrap(
		0,
		0, 0,
		hIcon,
		::LoadCursor(nullptr, IDC_ARROW),
		(HBRUSH)::GetStockObject(NULL_BRUSH),
		nullptr,
		TEXT("gandharva-MainWindow"),
		hIcon
	);

	this->CreateWindowWrap(
		WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
		(LPTSTR)atom,
		TEXT("gandharva"),
		WS_OVERLAPPEDWINDOW,
		0, 0, 0, 0,
		nullptr,
		nullptr
	);
}


void
MainWindow::RestorePrevState()
{
	auto& config = Profile::Get<Profile::Config>();
	auto& state  = Profile::Get<Profile::State >();

	{ // 隠すボリュームは隠しつつ、ツリービューにボリュームを作成
		// "Hidden=ACD" → fHidden = 32'b0000_..._0000_1101
		auto sHidden = config.Get(TEXT("Hidden"), nullptr);
		DWORD fHidden = 0;
		for(auto c: sHidden){
			if('A' <= c && c <= 'Z'){
				c -= 'A';
			}
			else if('a' <= c && c <= 'z'){
				c -= 'a';
			}
			else continue;

			fHidden |= ((DWORD)1 << c);
		}

		pExplorerTree_->CreateRoot(fHidden);
	}

	for(int i = 0; ; ++i){
		// お気に入りルートを復元
		TCHAR szKey[32];
			wsprintf(szKey, TEXT("Favorite[%02d]"), i);
		auto path = config.Get(szKey, nullptr);
		if(path.empty()) break;

		pExplorerTree_->AddFavorite(path);
	}

	{ // スプリッタの位置を設定
		pSplitter_->SetInitialLocation(
			state.GetInt(TEXT("MainWnd.Splitter"), CSplitter::PEL_X_USE_DEFAULT)
		);
	}

	{ // ツールバーの状態を復元
		BOOL bRandom = state.GetInt(TEXT("Random"), 0);
			pToolBar_->SetToggle(ToolBar::TBID_Random, bRandom);
			pFileList_->SetRandom(bRandom != FALSE);
		int iRepeat = state.GetInt(TEXT("Repeat"), ToolBar::REPEAT_None);
			if(ToolBar::REPEAT_Begin <= iRepeat && iRepeat < ToolBar::REPEAT_End){
				pToolBar_->SetRepeatOption(iRepeat);
			}
		pVolumeBar_->SetVolume(state.GetInt(TEXT("Volume"), 0));
	}

	{ // リストビューのコラムの幅を復元
		FileListView::SColumnWidth cw;
		const FileListView::SColumnWidth cwDefault = {
			{200, 50, 50, 50}
		};

		for(int i = FileListView::ICOL_Begin;
			i < FileListView::ICOL_End; ++i)
		{
			TCHAR szKey[32];
				wsprintf(szKey, TEXT("FileList.Column[%d]"), i);
			cw.acx[i] = state.GetInt(szKey, cwDefault.acx[i]);
		}

		pFileList_->SetColumnWidth(cw);
	}

	{ // ウィンドウ位置復元
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		wp.flags = 0;
		wp.showCmd				   = state.GetInt(TEXT("MainWnd.ShowCmd"), SW_SHOWNORMAL);
		wp.ptMinPosition.x		   = state.GetInt(TEXT("MainWnd.MinPos.x"), 0);
		wp.ptMinPosition.y		   = state.GetInt(TEXT("MainWnd.MinPos.y"), 0);
		wp.ptMaxPosition.x		   = state.GetInt(TEXT("MainWnd.MaxPos.x"), 0);
		wp.ptMaxPosition.y		   = state.GetInt(TEXT("MainWnd.MaxPos.y"), 0);
		wp.rcNormalPosition.left   = state.GetInt(TEXT("MainWnd.Pos.left"), 0);
		wp.rcNormalPosition.top    = state.GetInt(TEXT("MainWnd.Pos.top"), 0);
		wp.rcNormalPosition.right  = state.GetInt(TEXT("MainWnd.Pos.right"), 640);
		wp.rcNormalPosition.bottom = state.GetInt(TEXT("MainWnd.Pos.bottom"), 480);
		::SetWindowPlacement(this->GetHandle(), &wp);
	}

	{ // フォルダを開く
		int argc = 0;
		LPTSTR* argv = ::CommandLineToArgvW(::GetCommandLine(), &argc);
		if(argc > 1){
			// コマンドラインで指定されているときは、それを演奏
			if(this->SelectFile(argv[1])){
				this->OnPlay();
			}
		}
		else{
			// コマンドラインがないときは、前回のを選択
			this->SelectFile(
				state.Get(TEXT("LastFolder"), nullptr),
				state.Get(TEXT("LastFile"), nullptr)
			);
		}

		::LocalFree((HLOCAL)argv);
	}
}


LRESULT
MainWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case MY_WM_DSHOWNOTIFY:
		return ((Player*)lParam)->DefOnNotify();
	case WM_CREATE:
		return this->OnCreate(hWnd);
	case WM_DESTROY:
		return this->OnDestroy();
	case WM_CLOSE:
		::DestroyWindow(hWnd);
		return 0;
	case WM_SIZE:
		return this->OnSize(wParam, lParam);
	case WM_TIMER:
		return this->OnTimer();
	case WM_HSCROLL:
		return this->OnHScroll(wParam, lParam);
	case WM_NOTIFY:
		return this->OnNotify(lParam);
	case WM_COMMAND:
		return this->OnCommand(wParam, lParam);
	case WM_CONTEXTMENU:
		return this->OnContextMenu(wParam, lParam);
	case WM_DROPFILES:
		return this->OnDropFiles(wParam);
	case MY_WM_ENDOFSOUND:
		return this->OnEndOfSound();
	case MY_WM_VOLUME:
		return this->OnVolumeChanged(lParam);
	case MY_WM_SHOWPLAYFILE:
		return this->OnShowPlayingFile();
	case MY_WM_ISPLAYING:
		return this->OnIsPlaying();
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


LRESULT
MainWindow::OnCreate(HWND hWnd)
{
	// メインの作業域
	this->pExplorerTree_.reset(new ExplorerTree(hWnd, 100));
	this->pFileList_	.reset(new FileListView(hWnd, 200));
	this->pSplitter_	.reset(new CSplitter(
		hWnd,
		pExplorerTree_->GetHandle(), pFileList_->GetHandle(),
		CSplitter::PEL_X_USE_DEFAULT
	));

	// ツールバー
	this->pRebar_.reset(new Rebar(hWnd));

	this->pToolBar_.reset(new ToolBar(hWnd, IDTB_OFFSET));
	this->pSeekBar_.reset(new SeekBar(hWnd, 400));

	this->pVolumeBar_.reset(new VolumeBar(hWnd, MY_WM_VOLUME));

	RECT rectToolBar;
		pToolBar_->GetRect(&rectToolBar);
	int width = rectToolBar.right - rectToolBar.left;
	int height = rectToolBar.bottom - rectToolBar.top;

	pRebar_->InsertBand(pToolBar_->GetHandle(), width, width, height);
	pRebar_->InsertBand(
		pSeekBar_->GetHandle(),
		10000 /* ツールバーのバンドが広がらないように押さえつけるため、大きな値を入れておく */,
		0,
		height
	);

	// ステータスバー
	this->pStatusBar_.reset(new StatusBar(hWnd));

	return 0;
}


LRESULT
MainWindow::OnDestroy()
{
	// begin 現在の状態を保存
	auto& config = Profile::Get<Profile::Config>();
	auto& state  = Profile::Get<Profile::State >();


	{ // ルートに追加されたパス
		int i = 0;
		for(HTREEITEM hItem = pExplorerTree_->GetFirstChild(TVI_ROOT);
			hItem; hItem = pExplorerTree_->GetNextSibling(hItem))
		{
			if(pExplorerTree_->IsFavorite(hItem)){
				TCHAR szKey[32];
                int keyLen = wsprintf(szKey, TEXT("Favorite[%02d]"), i++);
                std::wstring key(szKey, szKey + keyLen);

				auto path = pExplorerTree_->GetText(hItem);

                if(! Path::IsEqualFileSpec(config.Get(key), path)){
                    config.Set(key, path);
                }
			}
		}
	}

	{ // 現在演奏中or選択中のファイル
		std::wstring sFolderSpec;
		std::wstring sFileSpec  ;

		if(this->pPlayer_){
			sFolderSpec = pPlayer_->GetFolderSpec();
			sFileSpec   = pPlayer_->GetFileSpec();
		}
		else{
			sFolderSpec = pExplorerTree_->GetItemFullPath(
				pExplorerTree_->GetSelection()
			);

			if(this->pFolder_){
				int iItem = pFileList_->GetSelectedItem();
				if(iItem >= 0){
					sFileSpec = pFileList_->GetItemText(
						iItem, FileListView::ICOL_Title
					);
				}
			}
		}

		state.Set(TEXT("LastFolder"), sFolderSpec);
		state.Set(TEXT("LastFile"  ), sFileSpec);
	}

	{// ウィンドウの大きさ
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		::GetWindowPlacement(this->GetHandle(), &wp);

		state.SetInt(TEXT("MainWnd.ShowCmd"), wp.showCmd);
		state.SetInt(TEXT("MainWnd.MinPos.x"), wp.ptMinPosition.x);
		state.SetInt(TEXT("MainWnd.MinPos.y"), wp.ptMinPosition.y);
		state.SetInt(TEXT("MainWnd.MaxPos.x"), wp.ptMaxPosition.x);
		state.SetInt(TEXT("MainWnd.MaxPos.y"), wp.ptMaxPosition.y);
		state.SetInt(TEXT("MainWnd.Pos.left"), wp.rcNormalPosition.left);
		state.SetInt(TEXT("MainWnd.Pos.top"), wp.rcNormalPosition.top);
		state.SetInt(TEXT("MainWnd.Pos.right"), wp.rcNormalPosition.right);
		state.SetInt(TEXT("MainWnd.Pos.bottom"), wp.rcNormalPosition.bottom);
	}

	{ // スプリッタの位置
		state.SetInt(TEXT("MainWnd.Splitter"), pSplitter_->GetLocation());
	}

	{ // リストビューのコラムの幅
		FileListView::SColumnWidth cw;
		pFileList_->GetColumnWidth(&cw);

		for(int i = FileListView::ICOL_Begin;
			i < FileListView::ICOL_End; ++i)
		{
			TCHAR szKey[32];
				wsprintf(szKey, TEXT("FileList.Column[%d]"), i);
			state.SetInt(szKey, cw.acx[i]);
		}
	}

	{ // ツールバーの状態
		state.SetInt(TEXT("Random"),
			pToolBar_->IsToggleOn(ToolBar::TBID_Random) ? 1 : 0
		);
		state.SetInt(TEXT("Repeat"), pToolBar_->GetRepeatOption());
		state.SetInt(TEXT("Volume"), pVolumeBar_->GetVolume());
	}

	// end 現在の状態を保存

	::PostQuitMessage(0);
	return 0;
}


LRESULT
MainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	int heiRebar = 0;
	if(pRebar_){
		pRebar_->DefOnSize(wParam, lParam);

		RECT rect;
			::GetWindowRect(pRebar_->GetHandle(), &rect);
		heiRebar = rect.bottom - rect.top;
	}

	int heiStatus = 0;
	if(pStatusBar_){
		pStatusBar_->DefOnSize(wParam, lParam);
		RECT rect;
			::GetWindowRect(pStatusBar_->GetHandle(), &rect);
		heiStatus = rect.bottom - rect.top;
	}

	if(wParam != SIZE_MINIMIZED && pRebar_){
		RECT rect;
			::GetClientRect(this->GetHandle(), &rect);
			rect.top += heiRebar;
			rect.bottom -= heiStatus;
		pSplitter_->DefOnSize(pSplitter_->GetLocation(), &rect);
	}

	return 0;
}


LRESULT
MainWindow::OnTimer()
{
	long secPlayed = 0;
	if(pPlayer_){
		secPlayed = pPlayer_->GetPosition();
	}
	if(pSeekBar_){
		pSeekBar_->SetPosition(secPlayed);
	}
	if(pStatusBar_){
		pStatusBar_->SetTime(secPlayed);
	}
	return 0;
}


LRESULT
MainWindow::OnHScroll(WPARAM wParam, LPARAM lParam)
{
	if(pSeekBar_ && pSeekBar_->GetHandle() == (HWND)lParam){
		return this->OnSeekBarScroll(LOWORD(wParam));
	}
	return 0;
}


LRESULT
MainWindow::OnSeekBarScroll(DWORD dwNotification)
{
	switch(dwNotification){
	case TB_THUMBPOSITION:
	case TB_ENDTRACK:
		if(pPlayer_) pPlayer_->SetPosition(pSeekBar_->GetPosition());
		this->timer_.Set(this->GetHandle(), 1000);
		return 0;
	default:
		this->timer_.Unset(this->GetHandle());
		return 0;
	}

	return 0;
}


LRESULT
MainWindow::OnNotify(LPARAM lParam)
{
	NMHDR* pNMH = (NMHDR*)lParam;
	if(this->pExplorerTree_ &&
		this->pExplorerTree_->GetHandle() == pNMH->hwndFrom)
	{
		switch(pNMH->code){
		case TVN_SELCHANGED:
			return this->OnTreeSelChanged((NMTREEVIEW*)pNMH);
		}

		return this->pExplorerTree_->DefOnNotify(pNMH);
	}
	if(this->pFileList_ &&
		this->pFileList_->GetHandle() == pNMH->hwndFrom)
	{
		switch(pNMH->code){
		case NM_DBLCLK:
			return this->OnListDBLClick((NMITEMACTIVATE*)pNMH);
		}

		return this->pFileList_->DefOnNotify(pNMH);
	}
	if(this->pSeekBar_ &&
		this->pSeekBar_->GetToolTip() == pNMH->hwndFrom)
	{
		return this->pSeekBar_->DefOnNotify(pNMH);
	}
	if(this->pToolBar_ && (
		this->pToolBar_->GetHandle() == pNMH->hwndFrom ||
		this->pToolBar_->GetToolTip() == pNMH->hwndFrom))
	{
		return this->pToolBar_->DefOnNotify(pNMH);
	}

	return 0;
}


LRESULT
MainWindow::OnTreeSelChanged(NMTREEVIEW* pNMTV)
{
	this->LoadContents(pNMTV->itemNew.hItem);
	return 0;
}


LRESULT MainWindow::OnListDBLClick(NMITEMACTIVATE* pNMIA)
{
	pFileList_->SetSelectedItem(pNMIA->iItem);
	return this->OnPlay();
}


LRESULT MainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam) - IDTB_OFFSET){
	case ToolBar::TBID_Play:
		return this->OnPlay();
	case ToolBar::TBID_Pause:
		return this->OnTogglePause();
	case ToolBar::TBID_Stop:
		return this->OnStop();
	case ToolBar::TBID_GoPrev:
		return this->OnGoPrev();
	case ToolBar::TBID_GoNext:
		return this->OnGoNext();
	case ToolBar::TBID_Random:
		return this->OnToggleRandom();
	case ToolBar::TBID_Repeat:
		return this->OnRepeatOption();
	case ToolBar::TBID_Volume:
		return this->OnVolumeButton();
	case ToolBar::TBID_Favorite:
		return this->OnFavoriteButton();
	}

	return 0;
}


LRESULT MainWindow::OnPlay()
{
	pToolBar_->SetToggle(ToolBar::TBID_Pause, FALSE);

	int iItem = pFileList_->GetSelectedItem();
	if(iItem < 0 && pFileList_->GetItemCount() > 0){
		pFileList_->SetSelectedItem(iItem = 0);
	}

	if(iItem >= 0){
		this->BeginPlay(iItem);
	}
	return 0;
}


LRESULT MainWindow::OnTogglePause()
{
	if(pToolBar_->IsToggleOn(ToolBar::TBID_Pause)){
		if(pPlayer_ && pPlayer_->Stop()){
			;
		}
		else{
			pToolBar_->SetToggle(ToolBar::TBID_Pause, FALSE);
		}
	}
	else if(! pToolBar_->IsToggleOn(ToolBar::TBID_Pause)){
		if(pPlayer_ && pPlayer_->Play()){
			;
		}
		else{
			pToolBar_->SetToggle(ToolBar::TBID_Pause, TRUE);
		}
	}
	else{
		pToolBar_->SetToggle(ToolBar::TBID_Pause, FALSE);
	}

	return 0;
}


LRESULT MainWindow::OnStop()
{
	if(this->EndPlay()){
		pToolBar_->SetToggle(ToolBar::TBID_Pause, FALSE);
	}
	return 0;
}


LRESULT MainWindow::OnGoPrev()
{
	int iItem = pFileList_->GetSelectedItem();
	int cItems = pFileList_->GetItemCount();
	if(iItem < 0){
		// 何も選択されていないなら
		// 最後のアイテムを選択できるように、そのひとつ先のをポイント
		iItem = cItems;
	}
	else if(iItem == 0 &&
		pToolBar_->GetRepeatOption() == ToolBar::REPEAT_List)
	{
		// もうこれ以上前に戻れないとき
		// リピートがONならループ
		iItem = cItems;
	}

	if(--iItem >= 0){
		// 前のアイテムがあるなら、それを選択
		;
	}
	else{
		// リストの終わりに達した
		this->OnStop();
		return 0;
	}

	// 選択したものを再生
	pFileList_->SetSelectedItem(iItem);
	return this->OnPlay();
}


LRESULT MainWindow::OnGoNext()
{
	int iItem = pFileList_->GetSelectedItem();
	int cItems = pFileList_->GetItemCount();
	if(iItem < 0){
		// 何も選択されていないなら
		// 最初のアイテムが選択されるように -1 を入れておく
		iItem = -1;
	}
	else if(iItem >= cItems - 1 &&
		pToolBar_->GetRepeatOption() == ToolBar::REPEAT_List)
	{
		// もうこれ以上先に進めないとき
		// リピートがONならループ
		iItem = -1;

		// ランダム再生が On のときは、再シャッフル
		if(pToolBar_->IsToggleOn(ToolBar::TBID_Random)){
			pFileList_->SetRandom(true);
		}
	}

	if(++iItem < cItems){
		// 次のアイテムがあるなら、それを選択
		;
	}
	else{
		// リストの終わりに達した
		this->OnStop();
		return 0;
	}

	// 選択したものを再生
	pFileList_->SetSelectedItem(iItem);
	return this->OnPlay();
}


LRESULT
MainWindow::OnToggleRandom()
{
	pFileList_->SetRandom(
		pToolBar_->IsToggleOn(ToolBar::TBID_Random)
	);
	return 0;
}


LRESULT MainWindow::OnRepeatOption()
{
	enum {
		iIDOffset = 10
	};

	int iRepeat = pToolBar_->GetRepeatOption();

	HMENU hMenu = ::CreatePopupMenu();
	::AppendMenu(hMenu,
		(iRepeat == ToolBar::REPEAT_None) ? MF_STRING | MF_CHECKED : MF_STRING,
		ToolBar::REPEAT_None + iIDOffset,
		TEXT("リピートなし")
	);
	::AppendMenu(hMenu,
		(iRepeat == ToolBar::REPEAT_List) ? MF_STRING | MF_CHECKED : MF_STRING,
		ToolBar::REPEAT_List + iIDOffset,
		TEXT("リピート（リスト）")
	);
	::AppendMenu(hMenu,
		(iRepeat == ToolBar::REPEAT_Single) ? MF_STRING | MF_CHECKED : MF_STRING,
		ToolBar::REPEAT_Single + iIDOffset,
		TEXT("リピート（単曲）")
	);

	RECT rect;
		pToolBar_->GetItemRect(ToolBar::TBID_Repeat, &rect);
	POINT pt;
		pt.x = rect.left;
		pt.y = rect.bottom;
		::ClientToScreen(pToolBar_->GetHandle(), &pt);

	int id = ::TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
		pt.x, pt.y,
		0,
		this->GetHandle(),
		nullptr
	);

	if(id >= iIDOffset){
		pToolBar_->SetRepeatOption(id - iIDOffset);
	}

	::DestroyMenu(hMenu);
	return 0;
}


LRESULT
MainWindow::OnVolumeButton()
{
	RECT rect;
		pToolBar_->GetRect(&rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	POINT pt;
		pt.x = rect.left;
		pt.y = rect.bottom;

	pVolumeBar_->Show(pt.x, pt.y, width, height);
	return 0;
}


LRESULT
MainWindow::OnFavoriteButton()
{
	HMENU hMenu = ::CreatePopupMenu();

	// お気に入りフォルダをメニューに登録
	int cItems = 0;
	for(HTREEITEM hItem = pExplorerTree_->GetFirstChild(TVI_ROOT);
		hItem; hItem = pExplorerTree_->GetNextSibling(hItem))
	{
		if(pExplorerTree_->IsFavorite(hItem)){
			::AppendMenu(
				hMenu, MF_STRING, ++cItems, pExplorerTree_->GetText(hItem).c_str()
			);
		}
	}

	int id = 0;
	if(cItems > 0){
		// メニューを表示する箇所は、お気に入りボタンの下
		RECT rect;
			pToolBar_->GetItemRect(ToolBar::TBID_Favorite, &rect);
		POINT pt;
			pt.x = rect.left;
			pt.y = rect.bottom;
			::ClientToScreen(pToolBar_->GetHandle(), &pt);

		id = ::TrackPopupMenu(
			hMenu,
			TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN,
			pt.x, pt.y,
			0,
			this->GetHandle(),
			nullptr
		);
	}

	if(0 < id && id <= cItems){
		int cchPath = ::GetMenuString(hMenu, id, nullptr, 0, MF_BYCOMMAND);
		std::wstring path(cchPath, L'\0');
		cchPath = ::GetMenuString(hMenu, id, const_cast<LPWSTR>(path.data()), cchPath+1, MF_BYCOMMAND);
		path.resize(cchPath);

		pExplorerTree_->SetSelection(path);
		// SetSelection(path) をしても、path が選択されているとは限らない
		// e.g. path が存在しないとき
		// ので、実際に選択されたアイテムを別に取得する
		HTREEITEM hItem = pExplorerTree_->GetSelection();
		// そのフォルダを展開
		if(hItem) pExplorerTree_->Expand(hItem, TVE_EXPAND);
	}

	::DestroyMenu(hMenu);
	return 0;
}


LRESULT
MainWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	if(pExplorerTree_ &&
		pExplorerTree_->GetHandle() == (HWND)wParam)
	{
		return pExplorerTree_->DefOnContextMenu(lParam);
	}
	return 0;
}


LRESULT
MainWindow::OnDropFiles(WPARAM wParam)
{
	HDROP hDrop = (HDROP)wParam;
	TCHAR szPath[MAX_PATH + 10];
	UINT cchCopied = ::DragQueryFile(hDrop, 0, szPath, MAX_PATH + 10);
	if(cchCopied < MAX_PATH + 10){
		if(this->SelectFile(szPath)){
			this->OnPlay();
		}
	}

	::DragFinish(hDrop);
	return 0;
}


LRESULT
MainWindow::OnEndOfSound()
{
	if(pToolBar_->GetRepeatOption() == ToolBar::REPEAT_Single){
		// 単曲リピートのときは、同じ曲を繰り返す
		if(pPlayer_){
			pPlayer_->Stop();
			pPlayer_->SetPosition(0);
			pPlayer_->Play();
		}

		return 0;
	}
	else{
		// それ以外なら、次の曲へいく
		// （リストリピートのときもコレでおｋ）
		return this->OnGoNext();
	}
}


LRESULT
MainWindow::OnVolumeChanged(LPARAM lParam)
{
	// lParam はそのまま音量になっている
	if(pPlayer_) pPlayer_->SetVolume((long)lParam);
	return 0;
}


LRESULT
MainWindow::OnShowPlayingFile()
{
	if(pPlayer_){
		this->SelectFile(
			pPlayer_->GetFolderSpec(),
			pPlayer_->GetFileSpec()
		);
	}
	return 0;
}


LRESULT MainWindow::OnIsPlaying()
{
	return (BOOL)(bool)pPlayer_;
}


bool
MainWindow::BeginPlay(int iLVItem)
{
	bool bResult = false;
	std::wstring sFileSpec;

	if(iLVItem >= 0){
		sFileSpec = pFileList_->GetItemText(
			iLVItem, FileListView::ICOL_Title
		);
		if(this->pFolder_){
			pPlayer_ = pFolder_->CreatePlayer(
				sFileSpec, this->GetHandle(), MY_WM_DSHOWNOTIFY, MY_WM_ENDOFSOUND
			);
			if(pPlayer_){
				pPlayer_->SetVolume(pVolumeBar_->GetVolume());
				bResult = pPlayer_->Play();
			}
		}
	}

	if(bResult){
		long secDuration = pPlayer_->GetDuration();
		pSeekBar_->Enable(TRUE);
		pSeekBar_->SetRange(0, secDuration);
		pStatusBar_->SetTotalTime(secDuration);
		pStatusBar_->SetTitle(sFileSpec);

		this->timer_.Set(this->GetHandle(), 1000);
	}
	else{
		this->EndPlay();
	}

	return bResult;
}


bool
MainWindow::EndPlay()
{
	pPlayer_.reset();

	this->timer_.Unset(this->GetHandle());

	pSeekBar_->SetRange(0, 0);
	pSeekBar_->Enable(FALSE);
	pStatusBar_->SetTotalTime(-1);
	pStatusBar_->SetTitle(std::wstring());

	return true;
}


void MainWindow::LoadContents(HTREEITEM hti)
{
	auto path = pExplorerTree_->GetItemFullPath(hti);

	this->pFolder_.reset();

	if(::GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY){
		this->pFolder_ = std::make_shared<FolderDir>(path);
	}
	else if(FolderZip::IsSupported(path)){
		this->pFolder_ = std::make_shared<FolderZip>(path);
	}
	else if(PathMatchSpec(path.c_str(), TEXT("*.cue"))){
		this->pFolder_ = std::make_shared<FolderCue>(path);
	}

	// ロード
	if(pFileList_ && pFolder_){
		pFileList_->LoadFolderContents(this->pFolder_);
	}

	// 演奏中ならそれを選択
	if(pPlayer_){
		auto sPlayingFolder = pPlayer_->GetFolderSpec();
		if(Path::IsEqualFileSpec(path, sPlayingFolder)){
			pFileList_->SetSelectedItem(pPlayer_->GetFileSpec());
		}
	}
}


bool MainWindow::SelectFile(std::wstring const& path)
{
	// path は最初 ディレクトリだと思って考える
	auto dir = path;

	bool bSelectFile = false;

	DWORD dwAttr = ::GetFileAttributes(dir.c_str());
	if(dwAttr == (DWORD)(-1) ||
		!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		// ディレクトリだと思ったけどそうじゃなかった

		if(FolderZip::IsSupported(dir)){
			; // zip が有効のときは、zipもやっぱりフォルダと同じ扱いをする
		}
		else if(PathMatchSpec(dir.c_str(), TEXT("*.cue"))){
			; // cue が有効のときは、cueもやっぱりフォルダと同じ扱いをする
		}
		else{
			// フォルダでないときは、ディレクトリ部分を抜き出す
			dir = Path::GetDirName(path);
			bSelectFile = true;
		}
	}

	// ツリーを開く
	if(! pExplorerTree_->SetSelection(dir.empty() ? L"." : dir)){
		bSelectFile = false;
	}

	if(bSelectFile){
		// ファイルを選択
		bSelectFile = pFileList_->SetSelectedItem(Path::GetBaseName(path));
	}

	return bSelectFile;
}


bool MainWindow::SelectFile(std::wstring const& folderSpec, std::wstring const& fileSpec)
{
	if(folderSpec.empty()) return false;
	if(! pExplorerTree_->SetSelection(folderSpec)) return false;

	if(fileSpec.empty()) return false;
	if(! pFileList_->SetSelectedItem(fileSpec)) return false;

	return true;
}

} // namespace gandharva
