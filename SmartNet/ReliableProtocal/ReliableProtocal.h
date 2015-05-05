//============================================================================
// Name        : ReliableProtocal.h
// Author      : TheRockLiuHY
// Date        : May 5, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartNet
//============================================================================

#ifndef RELIABLEPROTOCAL_RELIABLEPROTOCAL_H_
#define RELIABLEPROTOCAL_RELIABLEPROTOCAL_H_

#include <Common/Defines.h>

namespace NSSmartNet
{

class CUdpWrapper
{
public:
	CUdpWrapper(){}
	~CUdpWrapper(){}


};

/*
 *
 */
class CReliableProtocal
{
public:
	CReliableProtocal();
	virtual ~CReliableProtocal();
};

} /* namespace NSSmartNet */

#endif /* RELIABLEPROTOCAL_RELIABLEPROTOCAL_H_ */
