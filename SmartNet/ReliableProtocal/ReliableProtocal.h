//============================================================================
// Name        : ReliableProtocal.h
// Author      : TheRockLiuHY
// Date        : May 5, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartNet
//============================================================================

#ifndef RELIABLEPROTOCAL_RELIABLEPROTOCAL_H_
#define RELIABLEPROTOCAL_RELIABLEPROTOCAL_H_

#include <Common/Common.h>

namespace NSSmartNet
{

/*
 * support reliable unicast, multicast and broadcast
 */
template <typename DATA_REF>
class CReliableNetEndpoint
{
	DISABLE_COPY(CReliableNetEndpoint)
	DISABLE_MOVE(CReliableNetEndpoint)

public:
	enum ECastType
	{
		ECT_NONE = -1, ECT_UNICAST = 0, ECT_MULTICAST = 1, ECT_BROADCAST = 2
	};

	enum EEPType
	{
		EEPT_NONE = -1, EEPT_SENDER = 0, EEPT_RECVER = 1
	};

public:
	CReliableNetEndpoint(ECastType ect, EEPType eept);
	~CReliableNetEndpoint();

public:
	int32_t SetData(DATA_REF &dr);
	bptr_t  GetData();

private:
	NSSmartUtils::RingBufPtr_t pInternalSendRB_;
	NSSmartUtils::RingBufPtr_t pInternalRecvRB_;
	NSSmartUtils::MemPoolPtr_t pMemPool_;
}
;

} /* namespace NSSmartNet */

template<typename DATA_REF, typename MEM_POOL>
inline NSSmartNet::CReliableNetEndpoint<DATA_REF, MEM_POOL>::CReliableNetEndpoint(
		ECastType ect, EEPType eept)
{
}

template<typename DATA_REF, typename MEM_POOL>
inline NSSmartNet::CReliableNetEndpoint<DATA_REF, MEM_POOL>::~CReliableNetEndpoint()
{
}

template<typename DATA_REF, typename MEM_POOL>
inline int32_t NSSmartNet::CReliableNetEndpoint<DATA_REF, MEM_POOL>::SetData(
		DATA_REF& dr)
{
}

template<typename DATA_REF, typename MEM_POOL>
inline bptr_t NSSmartNet::CReliableNetEndpoint<DATA_REF, MEM_POOL>::GetData()
{
}

#endif /* RELIABLEPROTOCAL_RELIABLEPROTOCAL_H_ */
