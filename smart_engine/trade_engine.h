//============================================================================
// Name        : tradeengine.h
// Author      : TheRockLiuHY
// Date        : May 21, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartEngine
//============================================================================

#ifndef TRADE_ENGINE_H_
#define TRADE_ENGINE_H_

#include <base/base.h>

namespace smart_engine
{
	enum trade_direction
	{
		TD_NONE = -1,
		TD_BID = 0,
		TD_OFFER = 1
	};

	class trade_kernel
	{
	public:
		int32_t handle(uint32_t trade_id, trade_direction td, uint64_t trade_price, uint32_t trade_cnt);

	};

	class trade_engine
	{
	public:
		trade_engine();
		virtual ~trade_engine();
	};

} /* namespace smart_engine */

#endif /* TRADE_ENGINE_H_ */
