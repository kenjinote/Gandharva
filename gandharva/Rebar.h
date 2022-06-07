#ifndef _baec6531_cb06_45ea_9526_851cbcd6a37a
#define _baec6531_cb06_45ea_9526_851cbcd6a37a

#include "MyWindows.h"

namespace gandharva
{

class Rebar
{
public:
	Rebar(HWND hwndParent);

	HWND GetHandle() const { return this->hwndThis_; }

	void InsertBand(HWND hWnd, int cx, int cxMin, int cy, int iInsertAt = -1);

	void DefOnSize(WPARAM wParam, LPARAM lParam);

private:
	HWND hwndThis_;
};

} // namespace gandharva
#endif//_baec6531_cb06_45ea_9526_851cbcd6a37a
