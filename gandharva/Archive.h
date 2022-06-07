#ifndef _c6b631bc_e279_4926_ac73_a678156ef20c
#define _c6b631bc_e279_4926_ac73_a678156ef20c

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <map>

#include "MyWindows.h"


namespace gandharva
{

class Archive
{
public:
	typedef std::shared_ptr<Archive>  P;

	// ファイル名 |-> アーカイブ内のインデクス
	typedef std::map<std::wstring, std::uint32_t>  FileList;

	struct FilterInfo
	{
		std::wstring               name;
		std::vector<std::wstring>  filters;
	};

	explicit Archive(std::wstring const& path);
	~Archive();

	bool IsOK() const { return pArchive_ != nullptr; }

	FileList const& GetFileList(){ return fileList_; }

	std::vector<unsigned char>
	GetData(std::wstring const& item) const;

	std::vector<unsigned char>
	GetData(std::uint32_t iItem) const;

	std::uint64_t
	GetSize(std::wstring const& item) const;

	std::uint64_t
	GetSize(std::uint32_t iItem) const;

	FILETIME 
	GetTime(std::wstring const& item) const;

	FILETIME
	GetTime(std::uint32_t iItem) const;


	static std::vector<FilterInfo>& GetFilters();

private:
	Archive(Archive const&);
	void operator=(Archive const&);

	void*       pArchive_;
	FileList    fileList_;

	static bool Init();
};

}

#endif//_c6b631bc_e279_4926_ac73_a678156ef20c
