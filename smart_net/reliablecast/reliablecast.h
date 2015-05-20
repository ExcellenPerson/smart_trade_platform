//============================================================================
// Name        : ReliableProtocal.h
// Author      : TheRockLiuHY
// Date        : May 5, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartNet
//============================================================================

#ifndef RELIABLECAST_RELIABLECAST_H_
#define RELIABLECAST_RELIABLECAST_H_

#include <base/base.h>

namespace ns_smart_net
{

/*
 * support unreliable/reliable unicast, multicast
 */
class CNetEndpoint
{
	DISABLE_COPY(CNetEndpoint)
	DISABLE_MOVE(CNetEndpoint)

public:
	enum ECastType
	{
		ECT_NONE = -1, ECT_UNICAST = 0, ECT_MULTICAST = 1, ECT_BROADCAST = 2
	};

	enum EEPType
	{
		EEPT_NONE = -1, EEPT_SENDER = 0, EEPT_RECVER = 1
	};

	enum EReliableOrNot
	{
		ERON_NONE = -1, ERON_RELIABLE = 0, ERON_NO_RELIABLE = 1
	};

public:
	CNetEndpoint(ECastType ect, EEPType eept, EReliableOrNot eron);
	~CNetEndpoint();

public:
	int32_t SetData(DATA_REF &dr);
	bptr_t GetData();

private:
	ns_smart_utils::RingBufPtr_t pInternalSendRB_;
	ns_smart_utils::RingBufPtr_t pInternalRecvRB_;
	ns_smart_utils::MemPoolPtr_t pMemPool_;
}
;

} /* namespace NSSmartNet */

#endif /* RELIABLECAST_RELIABLECAST_H_ */
