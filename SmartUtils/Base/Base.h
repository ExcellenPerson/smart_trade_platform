//============================================================================
// Name        : Platform.h
// Author      : TheRockLiuHY
// Date        : Apr 29, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartUtils
//============================================================================

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#if !defined(__linux__) || !defined(__GNUC__) || (__GNUC__ < 4)
#error only support linux and using gnu compiler 4.x...
#endif

#if (__cplusplus <= 199711L)
#define __SUPPORT_CPP_11__ (0)
#else
#define __SUPPORT_CPP_11__ (1)
#endif

///common header files...
#include <cstdint>

#include "Defines.h"
#include "Types.h"
#include "Config.h"

namespace NSSmartUtils
{
	enum EErrCode
	{
		EEC_SUC = 0, EEC_ERR = -1, EEC_REDO_ERR = -2
	};

}
#endif /* COMMON_COMMON_H_ */

