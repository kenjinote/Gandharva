#ifndef _c4f013b7_e8a7_40fc_9e24_61ca5a30bd8a
#define _c4f013b7_e8a7_40fc_9e24_61ca5a30bd8a

#include "MyWindows.h"
#include <dshow.h>

#include <string>
#include <memory>

#include "ComPtr.h"


// <dshow.h> が <windowsx.h> を読み込んでしまうので
// 他に影響を与えるマクロを消しておく
#undef GetFirstChild
#undef GetNextSibling


namespace gandharva
{

class SourceRenderer
{
public:
	virtual ~SourceRenderer(){}
	virtual HRESULT Render(IGraphBuilder* pGraph) const = 0;
};


// ファイルを演奏するための SourceRenderer
// sFilePath を参照で保持するだけなので注意
class FileSourceRenderer
	: public SourceRenderer
{
public:
	FileSourceRenderer(std::wstring const& sFilePath)
		: sFilePath_(sFilePath)
	{}

	virtual HRESULT Render(IGraphBuilder* pGraph) const {
		return pGraph->RenderFile(sFilePath_.c_str(), nullptr);
	}

private:
	std::wstring const& sFilePath_;
};


class Player
{
public:
	typedef std::shared_ptr<Player>  P;

	// sFolderSpec : ツリービューアイテムのID文字列
	// sFileSpec   : ファイルのID文字列
	Player(
		SourceRenderer const& source,
		std::wstring   const& sFolderSpec,
		std::wstring   const& sFileSpec,
		HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound
	);
	virtual ~Player();

	// 以下いろんな演奏メソッドがあるが
	//   this->state_ == EPS_Error （DirectX 初期化失敗）
	// のときに呼び出されても大丈夫なようにオーバーロードすること
	virtual bool Play();
	virtual bool Stop();

	enum EPlayState {
		EPS_Error,
		EPS_Stop,
		EPS_Play,
		EPS_Pause
	};
	virtual int GetState() const { return this->state_; }

	virtual bool SetVolume(long centi_dB);
	virtual long GetVolume() const { return mBVolume_; }
	virtual long SetPosition(long sec);
	virtual long GetPosition();
	virtual long GetDuration();

	virtual LRESULT DefOnNotify();

	std::wstring const& GetFolderSpec() const { return sFolderSpec_; }
	std::wstring const& GetFileSpec() const { return sFileSpec_; }

protected:
	void Destroy();

	comptr<IGraphBuilder>	pGraphBuilder_;
	comptr<IBasicAudio  >	pBasicAudio_;
	comptr<IMediaControl>	pMediaControl_;
	comptr<IMediaSeeking>	pMediaSeeking_;
	comptr<IMediaEventEx>	pMediaEvent_;

	EPlayState state_;
	long mBVolume_;

	const HWND hwndNotify_;
	const UINT uMsgNotify_;
	const UINT uMsgEndOfSound_;

private:
	std::wstring sFolderSpec_;
	std::wstring sFileSpec_;
};

} // namespace gandharva
#endif//_c4f013b7_e8a7_40fc_9e24_61ca5a30bd8a
