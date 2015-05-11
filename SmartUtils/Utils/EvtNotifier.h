/*
 * EvtNotifier.h
 *
 *  Created on: May 11, 2015
 *      Author: rock
 */

#ifndef UTILS_EVTNOTIFIER_H_
#define UTILS_EVTNOTIFIER_H_

#include <memory>
#include "../Common/Common.h"

namespace NSSmartUtils
{

class IEventHandler
{

};

typedef std::shared_ptr<NSSmartUtils::IEventHandler> EventHandlerPtr_t;

class CTimerHandler
{

};

class CSignalHandler
{

};

class CEventHandler
{

};

class CNetHandler
{

};

class CEvtNotifierEngine
{
public:
	CEvtNotifierEngine();
	virtual ~CEvtNotifierEngine();

public:
	int32_t AddEvent(EventHandlerPtr_t &pEvtHandler);
	int32_t RemoveEvent(EventHandlerPtr_t &pEvtHandler);

	void HandleOnce();
};

} /* namespace NSSmartUtils */

#endif /* UTILS_EVTNOTIFIER_H_ */
