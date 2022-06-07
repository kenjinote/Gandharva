#ifndef _4c82d541_2c58_433d_b91b_4cac673cb9fb
#define _4c82d541_2c58_433d_b91b_4cac673cb9fb

#include "MyWindows.h"

namespace gandharva
{

namespace Application
{
	// メッセージループをまわす
	UINT Run();

	// 長いループがあるとき、その最初にこれを置くといい
	void PeekAndDispatchMessage();

	// 指定されたメッセージをキューから取り除く
	// (きちんと dispatch も行う)
	void FlushMessageQueue(UINT msgMin, UINT msgMax);
}

} // namespace gandharva
#endif//_4c82d541_2c58_433d_b91b_4cac673cb9fb
