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

class CUdpWrapper
{
public:
	CUdpWrapper()
	{
	}
	~CUdpWrapper()
	{
	}

};

/*
 * support reliable unicast, multicast and broadcast
 */
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
	int32_t SetData(bptr_t pData, uint32_t DataBytes);
	bptr_t  GetData();
}
;

} /* namespace NSSmartNet */

#endif /* RELIABLEPROTOCAL_RELIABLEPROTOCAL_H_ */
