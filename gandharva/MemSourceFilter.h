#ifndef _d4a65750_9684_4a62_9d1d_baa246e9741f
#define _d4a65750_9684_4a62_9d1d_baa246e9741f

// 本来は <DShow.h> をインクルードすれば済むけれど
// Windows のバージョンの設定をしないといけないので
// 先に MyWindows.h をインクルードする
#include "MyWindows.h"

#include <DShow.h>
#include <vector>
#include <queue>
#include <utility>

namespace gandharva
{

class MemSourceFilter;

 
class MemOutputPin
	: public IPin
{
public:
	friend class MemSourceFilter;

private:
	MemOutputPin(MemSourceFilter* pFilter)
		:	pFilter_(pFilter), 
			pMediaType_(nullptr), pPinTo_(nullptr),
			numMyMediaTypes_(1),
			bAsyncReaderQueried_(false)
	{
		// this instance is owned by the filter; so don't "pFilter_->AddRef()"

		for(auto& mt: myMediaTypes_){
			mt.majortype            = MEDIATYPE_Stream;
			mt.subtype              = GUID_NULL;
			mt.bFixedSizeSamples    = TRUE ;
			mt.bTemporalCompression = FALSE;
			mt.lSampleSize          = 1;
			mt.formattype           = GUID_NULL;
			mt.pUnk                 = nullptr;
			mt.cbFormat             = 0;
			mt.pbFormat             = nullptr;
		}
	}

	virtual ~MemOutputPin()
	{
		/* do nothing */
	}

	MemSourceFilter* pFilter_;

//
// IUnknown
//
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

//
// IPin
//
public:
	virtual HRESULT STDMETHODCALLTYPE Connect( 
		/* [in] */ IPin *pReceivePin,
		/* [annotation][in] */ 
		_In_opt_  const AM_MEDIA_TYPE *pmt);
        
	virtual HRESULT STDMETHODCALLTYPE ReceiveConnection( 
		/* [in] */ IPin *pConnector,
		/* [in] */ const AM_MEDIA_TYPE *pmt);
        
	virtual HRESULT STDMETHODCALLTYPE Disconnect( void);
        
	virtual HRESULT STDMETHODCALLTYPE ConnectedTo( 
		/* [annotation][out] */ 
		_Out_  IPin **pPin);
        
	virtual HRESULT STDMETHODCALLTYPE ConnectionMediaType( 
		/* [annotation][out] */ 
		_Out_  AM_MEDIA_TYPE *pmt);
        
	virtual HRESULT STDMETHODCALLTYPE QueryPinInfo( 
		/* [annotation][out] */ 
		_Out_  PIN_INFO *pInfo);
        
	virtual HRESULT STDMETHODCALLTYPE QueryDirection( 
		/* [annotation][out] */ 
		_Out_  PIN_DIRECTION *pPinDir);
        
	virtual HRESULT STDMETHODCALLTYPE QueryId( 
		/* [annotation][out] */ 
		_Out_  LPWSTR *Id);
        
	virtual HRESULT STDMETHODCALLTYPE QueryAccept( 
		/* [in] */ const AM_MEDIA_TYPE *pmt);
        
	virtual HRESULT STDMETHODCALLTYPE EnumMediaTypes( 
		/* [annotation][out] */ 
		_Out_  IEnumMediaTypes **ppEnum);
        
	virtual HRESULT STDMETHODCALLTYPE QueryInternalConnections( 
		/* [annotation][out] */ 
		IPin **apPin,
		/* [out][in] */ ULONG *nPin);
        
	virtual HRESULT STDMETHODCALLTYPE EndOfStream( void);
        
	virtual HRESULT STDMETHODCALLTYPE BeginFlush( void);
        
	virtual HRESULT STDMETHODCALLTYPE EndFlush( void);
        
	virtual HRESULT STDMETHODCALLTYPE NewSegment( 
		/* [in] */ REFERENCE_TIME tStart,
		/* [in] */ REFERENCE_TIME tStop,
		/* [in] */ double dRate);

private:
	AM_MEDIA_TYPE* pMediaType_; // 接続情報
	IPin         * pPinTo_    ; // 接続情報

public:
	AM_MEDIA_TYPE const* GetMyMediaType(ULONG index) const {
		return &myMediaTypes_[index];
	}
	ULONG GetNumMyMediaType() const { return numMyMediaTypes_; }

	void SetMyMediaType(std::vector<unsigned char> const& mem);

private:
	AM_MEDIA_TYPE myMediaTypes_[2]; // このピン固有の情報
	ULONG         numMyMediaTypes_; // このピン固有の情報

	bool bAsyncReaderQueried_;
};


class MemAMAsyncReaderTimestampScaling
	: public IAMAsyncReaderTimestampScaling
{
public:
	friend class MemSourceFilter;

private:
	MemAMAsyncReaderTimestampScaling(MemSourceFilter* pFilter)
		: pFilter_(pFilter), fRaw_(FALSE)
	{
		// this instance is owned by the filter; so don't "pFilter_->AddRef()"
	}

	virtual ~MemAMAsyncReaderTimestampScaling()
	{
		/* do nothing */
	}

	MemSourceFilter* pFilter_;
	BOOL             fRaw_   ;

//
// IUnknown
//
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

//
// IAMAsyncReaderTimestampScaling
//
public:
	virtual HRESULT STDMETHODCALLTYPE GetTimestampMode( 
		/* [annotation] */ 
		_Out_  BOOL *pfRaw);
        
	virtual HRESULT STDMETHODCALLTYPE SetTimestampMode( 
		BOOL fRaw);

};


class MemAsyncReader
	: public IAsyncReader
{
public:
	friend class MemSourceFilter;

private:
	MemAsyncReader(MemSourceFilter* pFilter)
		: pFilter_(pFilter)
		, memory_()
		, hSemNumQueue_(nullptr)
		, hEvFlushing_ (nullptr)
	{
		// this instance is owned by the filter; so don't "pFilter_->AddRef()"

		hSemNumQueue_ = ::CreateSemaphore(nullptr, 0, LONG_MAX, nullptr);
		hEvFlushing_  = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);

		::InitializeCriticalSection(&csQueue_);
		::InitializeCriticalSection(&csEndFlush_);
	}

	virtual ~MemAsyncReader()
	{
		if(hSemNumQueue_ != nullptr){
			::CloseHandle(hSemNumQueue_);
		}
		if(hEvFlushing_ != nullptr){
			::CloseHandle(hEvFlushing_);
		}
		::DeleteCriticalSection(&csQueue_);
		::DeleteCriticalSection(&csEndFlush_);
	}

	MemSourceFilter* pFilter_;
	std::vector<unsigned char> memory_;

//
// IUnknown
//
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

//
// IAsyncReader
//
public:
	virtual HRESULT STDMETHODCALLTYPE RequestAllocator( 
		/* [in] */ IMemAllocator *pPreferred,
		/* [annotation][in] */ 
		_In_  ALLOCATOR_PROPERTIES *pProps,
		/* [annotation][out] */ 
		_Out_  IMemAllocator **ppActual);
        
	virtual HRESULT STDMETHODCALLTYPE Request( 
		/* [in] */ IMediaSample *pSample,
		/* [in] */ DWORD_PTR dwUser);
        
	virtual HRESULT STDMETHODCALLTYPE WaitForNext( 
		/* [in] */ DWORD dwTimeout,
		/* [annotation][out] */ 
		_Out_opt_  IMediaSample **ppSample,
		/* [annotation][out] */ 
		_Out_  DWORD_PTR *pdwUser);
        
	virtual HRESULT STDMETHODCALLTYPE SyncReadAligned( 
		/* [in] */ IMediaSample *pSample);
        
	virtual HRESULT STDMETHODCALLTYPE SyncRead( 
		/* [in] */ LONGLONG llPosition,
		/* [in] */ LONG lLength,
		/* [annotation][size_is][out] */ 
		BYTE *pBuffer);
        
	virtual HRESULT STDMETHODCALLTYPE Length( 
		/* [annotation][out] */ 
		_Out_  LONGLONG *pTotal,
		/* [annotation][out] */ 
		_Out_  LONGLONG *pAvailable);
        
	virtual HRESULT STDMETHODCALLTYPE BeginFlush( void);
        
	virtual HRESULT STDMETHODCALLTYPE EndFlush( void);

private:
	HANDLE  hSemNumQueue_;
	HANDLE  hEvFlushing_ ;
	CRITICAL_SECTION csQueue_;
	CRITICAL_SECTION csEndFlush_;
	std::queue<std::pair<IMediaSample*, DWORD_PTR> > queue_;
};


class MemSourceFilter
	: public IBaseFilter
{
public:
	MemSourceFilter()
		: outputPin_(this)
		, amAsyncReaderTimestampScaling_(this)
		, asyncReader_(this)
		, refCount_(1)
		, filterState_(State_Stopped)
		, pClock_(nullptr)
	{
		filterInfo_.achName[0] = L'\0';
		filterInfo_.pGraph     = nullptr;

		InitializeCriticalSection(&csRefCount_);
	}

	virtual ~MemSourceFilter(){
		DeleteCriticalSection(&csRefCount_);
	}

	MemOutputPin* GetOutputPin() {
		outputPin_.AddRef();
		return &outputPin_;
	}

	MemAMAsyncReaderTimestampScaling* GetAMAsyncReaderTimestampScaling() {
		amAsyncReaderTimestampScaling_.AddRef();
		return &amAsyncReaderTimestampScaling_;
	}

	MemAsyncReader* GetAsyncReader() {
		asyncReader_.AddRef();
		return &asyncReader_;
	}

	FILTER_STATE GetState() { return this->filterState_; }

	BOOL IsTimestampScalingRaw() {
		return amAsyncReaderTimestampScaling_.fRaw_;
	}

	void SetMemory(std::vector<unsigned char> const& mem)
	{
		outputPin_.SetMyMediaType(mem);
		asyncReader_.memory_ = mem;
	}
	void SetMemory(std::vector<unsigned char>&& mem)
	{
		outputPin_.SetMyMediaType(mem);
		asyncReader_.memory_ = std::move(mem);
	}

private:
	MemOutputPin                     outputPin_;
	MemAMAsyncReaderTimestampScaling amAsyncReaderTimestampScaling_;
	MemAsyncReader                   asyncReader_;

//
// IUnknown
//
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

private:
	ULONG            refCount_;
	CRITICAL_SECTION csRefCount_;

//
// IPersist
//
public:
    virtual HRESULT STDMETHODCALLTYPE GetClassID( 
        /* [out] */ __RPC__out CLSID *pClassID);

//
// IMediaFilter
//
public:
	virtual HRESULT STDMETHODCALLTYPE Stop( void);
        
	virtual HRESULT STDMETHODCALLTYPE Pause( void);
        
	virtual HRESULT STDMETHODCALLTYPE Run( 
		REFERENCE_TIME tStart);
        
	virtual HRESULT STDMETHODCALLTYPE GetState( 
		/* [in] */ DWORD dwMilliSecsTimeout,
		/* [annotation][out] */ 
		__out  FILTER_STATE *State);
        
	virtual HRESULT STDMETHODCALLTYPE SetSyncSource( 
		/* [annotation][in] */ 
		__in_opt  IReferenceClock *pClock);
        
	virtual HRESULT STDMETHODCALLTYPE GetSyncSource( 
		/* [annotation][out] */ 
		__deref_out_opt  IReferenceClock **pClock);

private:
	FILTER_STATE      filterState_;
	IReferenceClock * pClock_;

//
// IBaseFilter
//
public:
	virtual HRESULT STDMETHODCALLTYPE EnumPins( 
		/* [annotation][out] */ 
		__out  IEnumPins **ppEnum);
        
	virtual HRESULT STDMETHODCALLTYPE FindPin( 
		/* [string][in] */ LPCWSTR Id,
		/* [annotation][out] */ 
		__out  IPin **ppPin);
        
	virtual HRESULT STDMETHODCALLTYPE QueryFilterInfo( 
		/* [annotation][out] */ 
		__out  FILTER_INFO *pInfo);
        
	virtual HRESULT STDMETHODCALLTYPE JoinFilterGraph( 
		/* [annotation][in] */ 
		__in_opt  IFilterGraph *pGraph,
		/* [annotation][string][in] */ 
		__in_opt  LPCWSTR pName);
        
	virtual HRESULT STDMETHODCALLTYPE QueryVendorInfo( 
		/* [annotation][string][out] */ 
		__out  LPWSTR *pVendorInfo);

private:
	FILTER_INFO filterInfo_;
};

} // namespace gandharva
#endif//_d4a65750_9684_4a62_9d1d_baa246e9741f
