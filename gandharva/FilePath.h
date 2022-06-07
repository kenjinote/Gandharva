#ifndef _e629e23e_a1ae_474b_981b_20b974d43a74
#define _e629e23e_a1ae_474b_981b_20b974d43a74

#include "MyWindows.h"

#include <string>
#include <vector>
#include <memory>


namespace gandharva
{

namespace Path
{

// dir と file を繋げる
std::wstring Join(std::wstring const& dir, std::wstring const& file);

// 区切り記号 ('\\') なしのファイル名同士が等しいかどうか
inline
bool IsEqualFileSpec(std::wstring const& filespec1, std::wstring const& filespec2)
{
	return 0 == ::lstrcmpi(filespec1.c_str(), filespec2.c_str());
}

// フルパスを取得
std::wstring GetFullName(std::wstring const& relativePath);

std::wstring GetBaseName(std::wstring const& path);
std::wstring GetDirName (std::wstring const& path);

// 拡張子を消去
std::wstring RemoveExtension(std::wstring const& path);

// 実行ファイルの場所
std::wstring GetExePath();

// フィルタ (*.wav みたいなの) と比較
bool IsMatchFilters(std::vector<std::wstring> const& filters, std::wstring const& path);

} // namespace Path
} // namespace gandharva
#endif//_e629e23e_a1ae_474b_981b_20b974d43a74


