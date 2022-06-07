#include "Player.h"
#pragma comment(lib, "strmiids.lib") // CLSID の在り処

#include "Application.h"


namespace gandharva
{


Player::Player(
	SourceRenderer const& source,
	std::wstring   const& sFolderSpec,
	std::wstring   const& sFileSpec,
	HWND hwndNotify, UINT uMsgNotify, UINT uMsgEndOfSound
)
	: pGraphBuilder_(nullptr), pBasicAudio_(nullptr),
	  pMediaControl_(nullptr), pMediaSeeking_(nullptr), pMediaEvent_(nullptr),
	  state_(EPS_Error), mBVolume_(0),
	  hwndNotify_(hwndNotify),
	  uMsgNotify_(uMsgNotify), uMsgEndOfSound_(uMsgEndOfSound),
	  sFolderSpec_(sFolderSpec), sFileSpec_(sFileSpec)
{
	// FilterGraphを生成
	HRESULT hr = ::CoCreateInstance(
		CLSID_FilterGraph,
		nullptr,
		CLSCTX_INPROC,
		__uuidof(IGraphBuilder),
		(LPVOID *)&pGraphBuilder_
	);

	// 各種インターフェースを取得
	if(SUCCEEDED(hr)){
		hr = pGraphBuilder_.QueryInterface(&pBasicAudio_);
	}
	if(SUCCEEDED(hr)){
		hr = pGraphBuilder_.QueryInterface(&pMediaControl_);
	}
	if(SUCCEEDED(hr)){
		hr = pGraphBuilder_.QueryInterface(&pMediaSeeking_);
	}
	if(SUCCEEDED(hr)){
		hr = pGraphBuilder_.QueryInterface(&pMediaEvent_);
	}

	// 通知先ウィンドウの設定
	if(SUCCEEDED(hr)){
		hr = pMediaEvent_->SetNotifyWindow(
			(OAHWND)this->hwndNotify_, this->uMsgNotify_, (LONG_PTR)this
		);
	}

	// ソースファイルを設定
	if(SUCCEEDED(hr)){
		hr = source.Render(pGraphBuilder_.get());
	}

	if(SUCCEEDED(hr)){
		// 成功。Player を生成した時点ではまだ演奏は止まっている。
		state_ = EPS_Stop;
	}
}


Player::~Player()
{
	this->Destroy();
}


void Player::Destroy()
{
	this->Stop();
	this->state_ = EPS_Error;

	// Player が死ぬ前に、
	// キューにポストされた通知メッセージを取り除かないといけない
	Application::FlushMessageQueue(this->uMsgNotify_, this->uMsgNotify_);

	pMediaEvent_	.reset();
	pMediaControl_	.reset();
	pBasicAudio_	.reset();
	pMediaSeeking_	.reset();
	pGraphBuilder_	.reset();
}


bool Player::Play()
{
	if(state_ == EPS_Play) return true;

	else if(state_ == EPS_Stop){
		if(SUCCEEDED(pMediaControl_->Run())){
			state_ = EPS_Play;
			return true;
		}
	}

	return false;
}


bool Player::Stop()
{
	if(this->state_ == EPS_Stop) return true;

	else if(state_ == EPS_Play){
		if(SUCCEEDED(pMediaControl_->Stop())){
			state_ = EPS_Stop;
			return true;
		}
	}

	return false;
}


bool Player::SetVolume(long centi_dB)
{
	if(state_ == EPS_Error) return false;

	if(centi_dB < -10000) centi_dB = -10000;
	else if(centi_dB > 0) centi_dB = 0;

	this->mBVolume_ = centi_dB;

	return ! pBasicAudio_ ||
		SUCCEEDED(pBasicAudio_->put_Volume(centi_dB));
}


long Player::SetPosition(long sec)
{
	if(this->state_ != EPS_Error){
		LONGLONG _100nsTime = (LONGLONG)sec * (LONGLONG)10000000;
		this->pMediaSeeking_->SetPositions(
			&_100nsTime	, AM_SEEKING_AbsolutePositioning,
			nullptr		, AM_SEEKING_NoPositioning
		);
		return this->GetPosition();
	}

	return 0;
}


long Player::GetPosition()
{
	if(this->state_ != EPS_Error){
		LONGLONG _100nsTime = 0;
		this->pMediaSeeking_->GetCurrentPosition(&_100nsTime);
		return (long)(_100nsTime / (LONGLONG)10000000);
	}
	else{
		return 0;
	}
}


long Player::GetDuration()
{
	if(this->state_ != EPS_Error){
		LONGLONG _100nsTime;
		this->pMediaSeeking_->GetDuration(&_100nsTime);
		return (long)(_100nsTime / (LONGLONG)10000000);
	}
	else{
		return 0;
	}
}


LRESULT
Player::DefOnNotify()
{
	long	 code;
	LONG_PTR lParam1;
	LONG_PTR lParam2;
	while(SUCCEEDED(pMediaEvent_->GetEvent(&code, &lParam1, &lParam2, 0))){
		switch(code){
		case EC_COMPLETE:
			::PostMessage(this->hwndNotify_, this->uMsgEndOfSound_, 0, 0);
			break;
		}
		pMediaEvent_->FreeEventParams(code, lParam1, lParam2);
	}

	return 0;
}


//const GUID CLSID_FilterGraph = {
//	0xe436ebb3, 0x524f, 0x11ce, {0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}
//};

} // namespace gandharva
