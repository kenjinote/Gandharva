#include "FolderCue.h"

#include "LanguageUtil.h"
#include "MyWindows.h"
#include "FilePath.h"


namespace gandharva
{

namespace {
	class PlayerOfCue
		: public Player
	{
	public:
		PlayerOfCue(
			std::wstring const& sFullPath,
			std::wstring const& sFolderSpec, std::wstring const& sFileSpec,
			const LONGLONG& _100nsBegin_, const LONGLONG& _100nsEnd,
			HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound
		);

		virtual ~PlayerOfCue(){}

		virtual long SetPosition(long sec);
		virtual long GetPosition();
		virtual long GetDuration();

	private:
		LONGLONG _100nsBegin_;
		LONGLONG _100nsEnd_;
	};

	bool CueTimeStrTo100ns(LPCTSTR lpTime, LONGLONG* p100ns);

	// 返り値：コマンドが一致したか？ (×コマンドが正しいか、ではない)
	bool CueGetParameter(LPTSTR itLine, LPCTSTR lpCommand, LPTSTR* plpValue, bool* pbError);
	bool CueGetParameter(LPTSTR itLine, LPCTSTR lpCommand, int* piValue, LPTSTR* plpValue, bool* pbError);
}


FolderCue::FolderCue(std::wstring const& sCuePath)
	: cTracks_(0), ixNextTrack_(0), sCuePath_(sCuePath)
{
	// cue 読み込み
	HANDLE hFile = ::CreateFile(
		sCuePath_.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr
	);
	if(hFile == INVALID_HANDLE_VALUE) return;

	auto fileHandleCleaner = Cleaner([&]{ ::CloseHandle(hFile); });

	DWORD cbFile = ::GetFileSize(hFile, nullptr);
	if(cbFile >= 100 * 1024 * 1024) return;

	std::string mbcsFile(cbFile, '\0');
	DWORD dwRead = 0;
	::ReadFile(hFile, const_cast<char*>(mbcsFile.data()), cbFile, &dwRead, nullptr);
	if(cbFile != dwRead) return;

	fileHandleCleaner.Invoke();

	std::wstring sFile = MultiByteToWideChar(mbcsFile);

	// 解析
	if(! sFile.empty()){
		LPWSTR itLine = const_cast<LPWSTR>(sFile.data());
		LPWSTR lpTrackFile = 0;
		LPWSTR lpTrackFilePrev = 0;
		LPWSTR lpPerformerLocal = 0;
		LPWSTR lpTitleLocal = 0;
		LPWSTR lpPerformerGlobal = 0;
		LPWSTR lpTitleGlobal = 0;
		bool bGlobal = true;

		while(1){
			// 行端を探す
			LPTSTR pEndLine;
			bool bNextLineExist = false;
			for(; *itLine != 0; ++itLine){
				if(*itLine != ' ' && *itLine != '\t') break;
			}
			if(*itLine == 0) break;

			for(pEndLine = itLine; *pEndLine != 0; ++pEndLine){
				if(*pEndLine == '\r' || *pEndLine == '\n'){
					*pEndLine = 0;
					bNextLineExist = true;
					break;
				}
			}

			// begin コマンド解釈
			bool bError;
			LPTSTR lpValue;
			int iValue;
			if(CueGetParameter(itLine, TEXT("FILE"), &lpValue, &bError)){
				if(bError) break;
				lpTrackFilePrev = lpTrackFile;
				lpTrackFile = lpValue;
			}
			else if(CueGetParameter(itLine, TEXT("TITLE"), &lpValue, &bError)){
				if(bError) break;
				if(bGlobal){
					lpTitleGlobal = lpValue;
				}
				else{
					lpTitleLocal = lpValue;
				}
			}
			else if(CueGetParameter(itLine, TEXT("PERFORMER"), &lpValue, &bError)){
				if(bError) break;
				if(bGlobal){
					lpPerformerGlobal = lpValue;
				}
				else{
					lpPerformerLocal = lpValue;
				}
			}
			else if(CueGetParameter(itLine, TEXT("TRACK"), &iValue, &lpValue, &bError)){
				if(bError ||
					! bGlobal || 
					! lpTrackFile ||
					::StrCmpNI(lpValue, TEXT("AUDIO"), 5) != 0
				){
					break;
				}

				// ローカルスコープに入る
				bGlobal = false;
			}
			else if(CueGetParameter(itLine, TEXT("INDEX"), &iValue, &lpValue, &bError)){
				if(bError ||
					bGlobal ||
					iValue < 0 ||
					iValue > 2
				){
					break;
				}

				LONGLONG _100ns;
				if(! CueTimeStrTo100ns(lpValue, &_100ns)){
					break;
				}

				// bGlobal == false になっているならば、必ず lpTrackFile != 0

				if(iValue == 0){
					if(lpTrackFilePrev == lpTrackFile){
						// INDEX 00 ... は前のトラックの終了時刻に設定
						if(cTracks_ > 0){
							aTracks_[cTracks_ - 1]._100nsEnd = _100ns;
						}
					}
				}
				else /* if(iValue == 1) */ {
					// もし前のトラックの終了時刻がまだ設定されていなければ
					// この時刻を終了時刻に設定
					if(lpTrackFilePrev == lpTrackFile){
						if(cTracks_ > 0){
							if(aTracks_[cTracks_ - 1]._100nsEnd == -1){
								aTracks_[cTracks_ - 1]._100nsEnd = _100ns;
							}
						}
					}

					// INDEX 01 ... をトラックの開始時刻に設定して
					// トラックを追加
					STrack& track = aTracks_[cTracks_++];

					track._100nsBegin = _100ns;
					track._100nsEnd = (LONGLONG)(-1);

					if(::PathIsRelative(lpTrackFile)){
						track.sFile = Path::Join(Path::GetDirName(sCuePath), lpTrackFile);
					}
					else{
						track.sFile = lpTrackFile;
					}

					WCHAR szTracks[64];
					int lenSzTracks = ::wnsprintf(szTracks, lengthof(szTracks), L"%02d", cTracks_);

					track.sTitle = std::wstring(szTracks, szTracks + lenSzTracks);
					if(lpTitleLocal || lpTitleGlobal){
						track.sTitle.push_back(L' ');
						track.sTitle += lpTitleLocal ? lpTitleLocal : lpTitleGlobal;
					}
					if(lpPerformerLocal || lpPerformerGlobal){
						track.sTitle += L" - ";
						track.sTitle += lpPerformerLocal ? lpPerformerLocal :  lpPerformerGlobal;
					}

					// ローカルスコープを抜ける
					lpTrackFilePrev = lpTrackFile;
					lpPerformerLocal = 0;
					lpTitleLocal = 0;
					bGlobal = true;

					// もしトラック数が99を超えたら終了
					if(cTracks_ >= 99) break;
				}
			}
			// 上記以外のコマンドは読み飛ばす
			// end コマンド解釈

			// 次の行へ位置づけ
			if(! bNextLineExist) break;
			itLine = pEndLine + 1;
		}
	}

	return;
}


bool FolderCue::Find(std::vector<std::wstring> const& filters, FindData* pfd, bool* pbLoading)
{
	if(this->ixNextTrack_ < this->cTracks_){
		pfd->uliSize.LowPart = (DWORD)(-1);
		pfd->uliSize.HighPart = (DWORD)(-1);
		pfd->ftDate.dwLowDateTime = 0;
		pfd->ftDate.dwHighDateTime = 0;
		pfd->sFileSpec = this->aTracks_[ixNextTrack_++].sTitle;
		pfd->sExtension = TEXT("cue");
		return true;
	}
	else{
		return false;
	}
}


void FolderCue::FindClose()
{
	this->ixNextTrack_ = 0;
}



Player::P FolderCue::CreatePlayer(std::wstring const& sFileSpec, HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound)
{
	for(DWORD i = 0; i < this->cTracks_; ++i){
		STrack& track = this->aTracks_[i];
		if(sFileSpec == track.sTitle){
			// make_shared は variadic template のない VC2012 ではまともに使えません(涙)
			return Player::P(new PlayerOfCue(
				track.sFile,
				this->sCuePath_, sFileSpec,
				track._100nsBegin, track._100nsEnd,
				hwndNotify, uMsgNotify, uMsgEndOfSound
			));
		}
	}

	return nullptr;
}


namespace {
	PlayerOfCue::PlayerOfCue(
		std::wstring const& sFullPath,
		std::wstring const& sFolderSpec, std::wstring const& sFileSpec,
		const LONGLONG& _100nsBegin, const LONGLONG& _100nsEnd,
		HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound
	)
		: Player(FileSourceRenderer(sFullPath),
			sFolderSpec, sFileSpec,
			hwndNotify, uMsgNotify, uMsgEndOfSound)
	{
		this->_100nsBegin_ = _100nsBegin;
		this->_100nsEnd_ = _100nsEnd;

		if(this->state_ != EPS_Error){
			// 終了時刻が不明だったときは
			// ここで取得しておく
			if(_100nsEnd == -1){
				this->pMediaSeeking_->GetDuration(&this->_100nsEnd_);
			}

			// 始点と終点を設定
			this->pMediaSeeking_->SetPositions(
				&this->_100nsBegin_, AM_SEEKING_AbsolutePositioning,
				&this->_100nsEnd_,
				(_100nsEnd != -1) ?
				AM_SEEKING_AbsolutePositioning : AM_SEEKING_NoPositioning
			);
		}
	}


	long PlayerOfCue::SetPosition(long sec)
	{
		if(this->state_ != EPS_Error){
			LONGLONG _100nsTime = (LONGLONG)sec * (LONGLONG)10000000;
			_100nsTime += this->_100nsBegin_;

			this->pMediaSeeking_->SetPositions(
				&_100nsTime	, AM_SEEKING_AbsolutePositioning,
				nullptr		, AM_SEEKING_NoPositioning
			);
			return this->GetPosition();
		}

		return 0;
	}


	long PlayerOfCue::GetPosition()
	{
		if(this->state_ != EPS_Error){
			LONGLONG _100nsTime = this->_100nsBegin_;
				this->pMediaSeeking_->GetCurrentPosition(&_100nsTime);
				_100nsTime -= this->_100nsBegin_;
			return long(_100nsTime / 10000000);
		}
		else{
			return 0;
		}
	}


	long PlayerOfCue::GetDuration()
	{
		if(this->state_ != EPS_Error){
			LONGLONG _100nsTime = this->_100nsEnd_ - this->_100nsBegin_;
			return long(_100nsTime / 10000000);
		}
		else{
			return 0;
		}
	}


	bool CueTimeStrTo100ns(LPCTSTR lpTime, LONGLONG* p100ns)
	{
		LPCTSTR lpMinute = lpTime;
		LPCTSTR lpSecond = 0;
			for(LPCTSTR it = lpMinute; *it != 0; ++it){
				if(*it == ':'){
					lpSecond = it + 1;
					break;
				}
			}
		LPCTSTR lpFrame = 0;
		if(lpSecond){
			for(LPCTSTR it = lpSecond; *it != 0; ++it){
				if(*it == ':'){
					lpFrame = it + 1;
					break;
				}
			}
		}

		if(! lpFrame) return false;

		int iM = ::StrToInt(lpMinute);
		int iS = ::StrToInt(lpSecond);
		int iF = ::StrToInt(lpFrame);

		// 1 sec = 75 frames
		iF += (iM * 60 + iS) * 75;

		// iF frames = (iF * 10,000,000 + 37) / 75 [100ns]
		*p100ns = ((LONGLONG)iF * (LONGLONG)10000000 + 37) / 75;

		return true;
	}


	bool CueGetParameter(LPTSTR itLine, LPCTSTR lpCommand, LPTSTR* plpValue, bool* pbError)
	{
		// eg: TITLE "..."

		int lenCommand = ::lstrlen(lpCommand);
		*pbError = true;

		if(::StrCmpNI(itLine, lpCommand, lenCommand) == 0){
			LPTSTR p1stQuot = ::StrChr(itLine + lenCommand, '"');
			LPTSTR p2ndQuot = p1stQuot ? ::StrChr(p1stQuot + 1, '"') : 0;
			if(p2ndQuot){
				*plpValue = p1stQuot + 1;
				*p2ndQuot = 0;
				*pbError = false;
			}

			return true;
		}

		return false;
	}


	bool CueGetParameter(LPTSTR itLine, LPCTSTR lpCommand, int* piValue, LPTSTR* plpValue, bool* pbError)
	{
		// eg: INDEX 01 00:00:00

		int lenCommand = ::lstrlen(lpCommand);
		*pbError = true;

		if(::StrCmpNI(itLine, lpCommand, lenCommand) != 0) return false;

		// まずは空白をスキップ
		TCHAR* it;
		for(it = itLine + 5; *it != 0; ++it){
			if(*it != ' ' && *it != '\t') break;
		}
		if(*it < '0' || '9' < *it) return true; // コマンドは一致するけどシンタックスはエラー（以下同様）

		*piValue = ::StrToInt(it);

		// 数字をスキップ
		for(it = it + 1; *it != 0; ++it){
			if(*it < '0' || '9' < *it) break;
		}

		// また空白をスキップ
		for(; *it != 0; ++it){
			if(*it != ' ' && *it != '\t') break;
		}
		
		*plpValue = it;
		*pbError = false;
		return true;
	}

}

} // namespace gandharva
