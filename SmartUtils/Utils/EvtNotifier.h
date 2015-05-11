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

class CEventNotifierEngine
{
public:
	CEventNotifierEngine();
	virtual ~CEventNotifierEngine();

public:
	int32_t AddEvent(EventHandlerPtr_t &pEvtHandler);
	int32_t RemoveEvent(EventHandlerPtr_t &pEvtHandler);

	inline void CheckOnce();
};

} /* namespace NSSmartUtils */

inline void NSSmartUtils::CEventNotifierEngine::CheckOnce()
{
}

#endif /* UTILS_EVTNOTIFIER_H_ */
