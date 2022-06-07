#include "Random.h"

#include "MyWindows.h"


namespace gandharva
{

Random::Random()
{
	FILETIME ft;
		::GetSystemTimeAsFileTime(&ft);
	LARGE_INTEGER li;
		::QueryPerformanceCounter(&li);
	x_ = ft.dwLowDateTime;
	y_ = ft.dwHighDateTime;
	z_ = li.LowPart;
	w_ = li.HighPart;

	// 100 âÒÇ≠ÇÁÇ¢Ç‹ÇÌÇµÇ∆ÇØÇŒÇ¢Ç¢Ç©Åc
	for(int i = 0; i < 100; ++i) (*this)();
}


std::uint32_t Random::operator ()()
{
	/* Xorshift RNG algorithm by George Marsaglia
		http://www.jstatsoft.org/v08/i14/paper
	*/

	enum {
	//	a =  5, b = 14, c =  1
		a = 15, b =  4, c = 21
	//	a = 23, b = 24, c =  3
	//	a =  5, b = 12, c = 29
	};
	std::uint32_t t = (x_ ^ (x_ << a));
	x_ = y_;
	y_ = z_;
	z_ = w_;
	return w_ = (w_ ^ (w_ >> c)) ^ (t ^ (t >> b));
}

} // namespace gandharva
