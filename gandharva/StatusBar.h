#ifndef _099e76b5_7612_4894_b430_f834660a00c3
#define _099e76b5_7612_4894_b430_f834660a00c3

#include "MyWindows.h"
#include <string>

namespace gandharva
{

class StatusBar
{
public:
	StatusBar(HWND hwndParent);

	HWND GetHandle() const {
		return this->hwndThis_;
	}

	void SetTime(long secPlayed);
	void SetTotalTime(long secTotal);
	void SetTitle(std::wstring const& title);

	LRESULT DefOnSize(WPARAM, LPARAM);

private:
	HWND hwndThis_;
	std::wstring sTotal_; // 演奏中のファイルの全体の時間
};

} // namespace gandharva
#endif//_099e76b5_7612_4894_b430_f834660a00c3
