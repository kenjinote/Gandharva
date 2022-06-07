#include "Archive.h"
#include "ComPtr.h"
#include "LanguageUtil.h"
#include "FilePath.h"
#include "MyWindows.h"
#include "Profile.h"

#include <initguid.h>

#include "7z/CPP/7zip/Archive/IArchive.h"


namespace gandharva
{

class PropVariant
{
public:
	PropVariant(): bFresh_(true) { PropVariantInit(&var_); }
	~PropVariant(){ this->Clear(); }

	PROPVARIANT* set() {
		this->Clear();
		this->bFresh_ = false;
		return &var_;
	}

	PROPVARIANT const* operator->() const { return &var_; }
	PROPVARIANT      * operator->()       { return &var_; }

	void Clear(){
		if(! bFresh_){
			PropVariantClear(&var_);
			bFresh_ = true;
		}
	}

	template <class T>
	T ToInt(){
		switch(var_.vt){
		case VT_I1:
			return (T)var_.cVal;
		case VT_UI1:
			return (T)var_.bVal;
		case VT_I2:
			return (T)var_.iVal;
		case VT_UI2:
			return (T)var_.uiVal;
		case VT_I4:
			return (T)var_.lVal;
		case VT_UI4:
			return (T)var_.ulVal;
		case VT_I8:
			return (T)var_.hVal.QuadPart;
		case VT_UI8:
			return (T)var_.uhVal.QuadPart;
		case VT_INT:
			return (T)var_.intVal;
		case VT_UINT:
			return (T)var_.uintVal;
		}

		return 0;
	}

private:
	PropVariant(PropVariant const&);
	void operator=(PropVariant const&);

	PROPVARIANT var_;
	bool        bFresh_;
};


class FileInStream
	: public IInStream
{
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if(! ppvObject) return E_POINTER;
#define ADDBRANCH(type__) \
		if(riid == IID_ ## type__){ \
			(*(type__**)ppvObject = this)->AddRef(); \
			return S_OK; \
		}

		ADDBRANCH(IUnknown);
		ADDBRANCH(ISequentialInStream);
		ADDBRANCH(IInStream);

#undef ADDBRANCH
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void){
		return ++refCount_;
	}

	virtual ULONG STDMETHODCALLTYPE Release( void){
		auto newCount = --refCount_;
		if(newCount == 0) delete this;
		return newCount;
	}

	STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize)
	{
		if(::ReadFile(hFile_, data, size, (DWORD*)processedSize, nullptr)){
			return S_OK;
		}
		else{
			return E_FAIL;
		}
	}

	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
	{
		if(SetFilePointerEx(
			hFile_, (LARGE_INTEGER&)offset, (LARGE_INTEGER*)newPosition, seekOrigin
		)){
			return S_OK;
		}
		else{
			return E_FAIL;
		}
	}

	explicit FileInStream(std::wstring const& path)
		: refCount_(1)
		, hFile_(::CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr))
	{}

	~FileInStream(){
		if(hFile_ != INVALID_HANDLE_VALUE) ::CloseHandle(hFile_);
	}

	bool IsOK() const { return hFile_ != INVALID_HANDLE_VALUE; }

private:
	ULONG  refCount_;
	HANDLE hFile_;
};


class CArchiveOpenCallback
	: public IArchiveOpenCallback
	//, public ICryptoGetTextPassword
{
public:
	CArchiveOpenCallback(): refCount_(1) {}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if(! ppvObject) return E_POINTER;
#define ADDBRANCH(type__) \
		if(riid == IID_ ## type__){ \
			(*(type__**)ppvObject = this)->AddRef(); \
			return S_OK; \
		}

		ADDBRANCH(IUnknown);
		ADDBRANCH(IArchiveOpenCallback);

#undef ADDBRANCH
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void){
		return ++refCount_;
	}

	virtual ULONG STDMETHODCALLTYPE Release( void){
		auto newCount = --refCount_;
		if(newCount == 0) delete this;
		return newCount;
	}

private:
	ULONG refCount_;

public:
	STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes){ return S_OK; }
	STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes){ return S_OK; }
};


class CMemSequentialOutStream
	: public ISequentialOutStream
{
public:
public:
	explicit CMemSequentialOutStream(std::vector<unsigned char>& mem)
		: refCount_(1), mem_(mem) {}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if(! ppvObject) return E_POINTER;
#define ADDBRANCH(type__) \
		if(riid == IID_ ## type__){ \
			(*(type__**)ppvObject = this)->AddRef(); \
			return S_OK; \
		}

		ADDBRANCH(IUnknown);
		ADDBRANCH(ISequentialOutStream);

#undef ADDBRANCH
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void){
		return ++refCount_;
	}

	virtual ULONG STDMETHODCALLTYPE Release( void){
		auto newCount = --refCount_;
		if(newCount == 0) delete this;
		return newCount;
	}

private:
	ULONG refCount_;

public:
	STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize){
		auto pByte = (const unsigned char*)data;
		try{
			mem_.insert(mem_.end(), pByte, pByte + size);
			return S_OK;
		}
		catch(...){
			return E_OUTOFMEMORY;
		}
	}

private:
	std::vector<unsigned char>& mem_; // reference to memory
};


class CMemArchiveExtractCallback
	: public IArchiveExtractCallback
	//, public ICryptoGetTextPassword
{
public:
	explicit CMemArchiveExtractCallback(std::vector<unsigned char>& mem)
		: refCount_(1), mem_(mem) {}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if(! ppvObject) return E_POINTER;
#define ADDBRANCH(type__) \
		if(riid == IID_ ## type__){ \
			(*(type__**)ppvObject = this)->AddRef(); \
			return S_OK; \
		}

		ADDBRANCH(IUnknown);
		ADDBRANCH(IProgress);
		ADDBRANCH(IArchiveExtractCallback);

#undef ADDBRANCH
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void){
		return ++refCount_;
	}

	virtual ULONG STDMETHODCALLTYPE Release( void){
		auto newCount = --refCount_;
		if(newCount == 0) delete this;
		return newCount;
	}

private:
	ULONG refCount_;

public:
	STDMETHOD(SetTotal)(UInt64 total){ return S_OK; }
	STDMETHOD(SetCompleted)(const UInt64 *completeValue){ return S_OK; }

	STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream,  Int32 askExtractMode){
		if(askExtractMode != NArchive::NExtract::NAskMode::kExtract){ return S_OK; }

		auto pStream = make_comptr<CMemSequentialOutStream>(mem_);
		if(! pStream){ return E_OUTOFMEMORY; }

		*outStream = pStream.get(); pStream.detach();
		return S_OK;
	}
	STDMETHOD(PrepareOperation)(Int32 askExtractMode){ return S_OK; }
	STDMETHOD(SetOperationResult)(Int32 resultEOperationResult){ return S_OK; }

private:
	std::vector<unsigned char>& mem_; // reference to memory
};



HMODULE                  g_h7z = nullptr;

HRESULT (STDAPICALLTYPE *CreateObject)(const GUID *clsid, const GUID *iid, void **outObject) = nullptr;
HRESULT (STDAPICALLTYPE *GetHandlerProperty2)(UInt32 formatIndex, PROPID propID, PROPVARIANT *value) = nullptr;
HRESULT (STDAPICALLTYPE *GetNumberOfFormats)(UINT32 *numFormats) = nullptr;

std::vector<Archive::FilterInfo>* g_pFilterInfo = nullptr;


namespace {
	Archive::FileList LoadFileList(IInArchive* pArc)
	{
		UInt32 numItems = 0;
		pArc->GetNumberOfItems(&numItems);

		WCHAR      szNoName[256];
		auto const sizeNoName = sizeof(szNoName)/sizeof(szNoName[0]);
		int        nonameId   = 1;

		Archive::FileList list;

		for(UInt32 iItem = 0; iItem < numItems; ++iItem){
			PropVariant isDir, name;

			auto hr = pArc->GetProperty(iItem, kpidIsDir, isDir.set());
			if(hr != S_OK) continue;
			if(isDir->boolVal) continue;

			hr = pArc->GetProperty(iItem, kpidPath, name.set());

			LPWSTR pszName;
			if(hr == S_OK && name->vt == VT_BSTR){
				pszName = name->bstrVal;
			}
			else{
				pszName = szNoName;

				wnsprintfW(szNoName, sizeNoName, L"<名前なし %d>", nonameId);
				szNoName[sizeNoName-1] = L'\0';
			}

			list.emplace(name->bstrVal, iItem);
		}

		return list;
	}
}


Archive::Archive(std::wstring const& path)
	: pArchive_(nullptr)
{
	if(! Init()) return;


	auto istream = make_comptr<FileInStream>(path);
	auto pOpenCallback = make_comptr<CArchiveOpenCallback>();
	if(! istream || ! istream->IsOK() || ! pOpenCallback){
		return;
	}

	std::uint32_t const nFormats = g_pFilterInfo->size();

	for(std::size_t iFormat = 0; iFormat < nFormats; ++iFormat){
		auto& format = (*g_pFilterInfo)[iFormat];

		if(! Path::IsMatchFilters(format.filters, path)) continue;

		PropVariant value;
		auto hr = GetHandlerProperty2(iFormat, NArchive::kClassID, value.set());

		comptr<IInArchive> pArc;
		// GUID は BSTR でやって来る
		if(hr == S_OK) hr = CreateObject((CLSID*)value->bstrVal, &IID_IInArchive, &pArc);

		if(hr == S_OK){
			istream->Seek(0, STREAM_SEEK_SET, nullptr);
			hr = pArc->Open(istream.get(), nullptr, pOpenCallback.get());
		}

		auto bOpened = bool(hr == S_OK);

		if(bOpened){
			this->fileList_ = LoadFileList(pArc.get());
			this->pArchive_ = pArc.get(); pArc.detach();
			return;
		}

		if(bOpened) pArc->Close();
	}
}


Archive::~Archive()
{
	if(this->pArchive_) ((IInArchive*)pArchive_)->Release();
}


std::vector<unsigned char>
Archive::GetData(std::wstring const& item) const
{
	auto it = this->fileList_.find(item);
	if(it == this->fileList_.end()) return std::vector<unsigned char>();

	return this->GetData(it->second);
}


std::vector<unsigned char>
Archive::GetData(std::uint32_t iItem) const
{
	auto pArc = (IInArchive*)pArchive_;
	std::vector<unsigned char> mem;

	auto pExtractCallback = make_comptr<CMemArchiveExtractCallback>(mem);
	if(! pExtractCallback) return mem;

	auto size = this->GetSize(iItem);

	try{
		mem.reserve((std::size_t)(size));
	}
	catch(...){
		return std::vector<unsigned char>();
	}

	UInt32 indices[] = { iItem };
	auto hr = pArc->Extract(indices, 1, FALSE, pExtractCallback.get());

	if(hr != S_OK){
		return std::vector<unsigned char>();
	}

	return mem;
}


std::uint64_t
Archive::GetSize(std::wstring const& item) const
{
	auto it = this->fileList_.find(item);
	if(it == this->fileList_.end()) return 0;

	return this->GetSize(it->second);
}


std::uint64_t
Archive::GetSize(std::uint32_t iItem) const
{
	auto pArc = (IInArchive*)pArchive_;

	PropVariant size;
	auto hr = pArc->GetProperty(iItem, kpidSize, size.set());
	if(hr != S_OK) return 0;

	return size.ToInt<std::uint64_t>();
}


FILETIME 
Archive::GetTime(std::wstring const& item) const
{
	auto it = this->fileList_.find(item);
	if(it == this->fileList_.end()){
		FILETIME ft = {};
		return ft;
	}

	return this->GetTime(it->second);
}


FILETIME
Archive::GetTime(std::uint32_t iItem) const
{
	auto pArc = (IInArchive*)pArchive_;

	PropVariant size;
	auto hr = pArc->GetProperty(iItem, kpidMTime, size.set());
	if(hr != S_OK){
		FILETIME ft = {};
		return ft;
	}

	return size->filetime;
}



std::vector<Archive::FilterInfo>& Archive::GetFilters()
{
	Init();

	// Init が成功しようがしまいがこのポインタは有効
	return *g_pFilterInfo;
}


bool Archive::Init()
{
	static bool s_bTried = false;

	if(g_h7z) return true;
	if(s_bTried) return false;

	// フィルタ情報だけはとりあえず初期化しておく
	g_pFilterInfo = new std::vector<Archive::FilterInfo>();

	s_bTried = true;

	// 設定ファイル
	auto& config = Profile::Get<Profile::Config>();

	//
	// 7z.dll をロード
	//

	g_h7z = LoadLibrary(config.Get(L"7zDLL", L"7z.dll").c_str());
	if(! g_h7z) return false;

	CreateObject        = (decltype(CreateObject       ))GetProcAddress(g_h7z, "CreateObject");
	GetHandlerProperty2 = (decltype(GetHandlerProperty2))GetProcAddress(g_h7z, "GetHandlerProperty2");
	GetNumberOfFormats  = (decltype(GetNumberOfFormats ))GetProcAddress(g_h7z, "GetNumberOfFormats");

	if(! CreateObject
	|| ! GetHandlerProperty2
	|| ! GetNumberOfFormats
	){
		CreateObject        = nullptr;
		GetHandlerProperty2 = nullptr;
		GetNumberOfFormats  = nullptr;

		::FreeLibrary(g_h7z);
		g_h7z = nullptr;

		::MessageBox(nullptr, TEXT("7z.dll が予期していたものではありません。無視します。"), TEXT("gandharva"), MB_OK | MB_ICONWARNING);

		return false;
	}

	//
	// サポートされている拡張子を取得
	//

	UINT32 numFormats = 0;
	GetNumberOfFormats(&numFormats);

	g_pFilterInfo->resize(numFormats);

	WCHAR const* const szAsterisk  = L"*.";
	WCHAR const* const endAsterisk = szAsterisk + 2;

	for(UInt32 i = 0; i < numFormats; ++i){
		auto& info = (*g_pFilterInfo)[i];

		PropVariant value;
		auto hr = GetHandlerProperty2(i, NArchive::kName, value.set());

		if(hr == S_OK) info.name = value->bstrVal;

		std::wstring sFilter = config.Get(L"Archive[" + info.name + L"]", L"DEFAULT");
		if(sFilter.empty()) continue;

		if(sFilter == L"DEFAULT"){
			hr = GetHandlerProperty2(i, NArchive::kExtension, value.set());

			std::wstring  ext;
			if(hr == S_OK) ext = value->bstrVal;

			info.filters = Split(ext, L' ');

			// { "zip", "docx"} |-> { "*.zip", "*.docx" }
			for(auto& e: info.filters){
				e.insert(e.begin(), szAsterisk, endAsterisk);
			}
		}
		else{
			info.filters = Split(sFilter, L';');
		}
	}

	return true;
}



} // namespace gandarva
