//============================================================================
// Name        : ReliableProtocal.h
// Author      : TheRockLiuHY
// Date        : May 5, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartNet
//============================================================================

#ifndef RELIABLE_CAST_RELIABLE_CAST_H_
#define RELIABLE_CAST_RELIABLE_CAST_H_

#include <base/base.h>

namespace ns_smart_net
{

/*
 * support unreliable/reliable unicast, multicast
 */
class net_endpoint
{
	DISABLE_COPY(net_endpoint)
	DISABLE_MOVE(net_endpoint)

public:
	enum cast_type
	{
		ECT_NONE = -1, ECT_UNICAST = 0, ECT_MULTICAST = 1, ECT_BROADCAST = 2
	};

	enum endpoint_type
	{
		EEPT_NONE = -1, EEPT_SENDER = 0, EEPT_RECVER = 1
	};

	enum reliable_or_not
	{
		ERON_NONE = -1, ERON_RELIABLE = 0, ERON_NO_RELIABLE = 1
	};

public:
	net_endpoint(ECastType ect, EEPType eept, EReliableOrNot eron);
	~net_endpoint();

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

#endif /* RELIABLE_CAST_RELIABLE_CAST_H_ */
