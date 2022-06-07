#ifndef _be4a5c0b_b722_4d24_9076_8ebc1b95019b
#define _be4a5c0b_b722_4d24_9076_8ebc1b95019b

#include "MyWindows.h"

namespace gandharva
{

class TrackBar
{
public:
	virtual ~TrackBar(){}

	TrackBar(HWND hwndParent, int iID, DWORD dwStyle);

	HWND GetHandle() const { return this->hwndThis_; }

	void SetRange(long min, long max);
	void SetPosition(long x);
	long GetPosition();

private:
	HWND hwndThis_;
};

} // namespace gandharva
#endif//_be4a5c0b_b722_4d24_9076_8ebc1b95019b
