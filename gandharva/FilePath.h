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

// dir �� file ���q����
std::wstring Join(std::wstring const& dir, std::wstring const& file);

// ��؂�L�� ('\\') �Ȃ��̃t�@�C�������m�����������ǂ���
inline
bool IsEqualFileSpec(std::wstring const& filespec1, std::wstring const& filespec2)
{
	return 0 == ::lstrcmpi(filespec1.c_str(), filespec2.c_str());
}

// �t���p�X���擾
std::wstring GetFullName(std::wstring const& relativePath);

std::wstring GetBaseName(std::wstring const& path);
std::wstring GetDirName (std::wstring const& path);

// �g���q������
std::wstring RemoveExtension(std::wstring const& path);

// ���s�t�@�C���̏ꏊ
std::wstring GetExePath();

// �t�B���^ (*.wav �݂����Ȃ�) �Ɣ�r
bool IsMatchFilters(std::vector<std::wstring> const& filters, std::wstring const& path);

} // namespace Path
} // namespace gandharva
#endif//_e629e23e_a1ae_474b_981b_20b974d43a74


