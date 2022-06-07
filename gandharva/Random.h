#ifndef _2d60dd07_4132_4740_8598_8299c47bc2c2
#define _2d60dd07_4132_4740_8598_8299c47bc2c2

#include <cstdint>

namespace gandharva
{

class Random
{
public:
	Random();
	std::uint32_t operator()();

private:
	std::uint32_t x_;
	std::uint32_t y_;
	std::uint32_t z_;
	std::uint32_t w_;
};

} // namespace gandharva
#endif//_2d60dd07_4132_4740_8598_8299c47bc2c2
