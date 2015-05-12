/*
 * EvtNotifier.h
 *
 *  Created on: May 11, 2015
 *      Author: rock
 */

#ifndef UTILS_EVENTNOTIFIER_H_
#define UTILS_EVENTNOTIFIER_H_

#include <memory>
#include <unordered_set>
#include <vector>
#include <mutex>
#include "../Common/Common.h"

namespace NSSmartUtils
{

class IEventHandler
{
public:
	IEventHandler();
	virtual ~IEventHandler();

public:
	virtual void OnAdded(bool Suc) = 0;
	virtual void OnRemoved(bool Suc) = 0;
	virtual void OnCanRead() = 0;
	virtual void OnCanWrite() = 0;
	virtual int32_t GetFD() = 0;
	virtual int32_t GetEvents() = 0;
};
typedef std::shared_ptr<NSSmartUtils::IEventHandler> EventHandlerPtr_t;


class CEventEngine
{
	DISABLE_COPY(CEventEngine)
	DISABLE_MOVE(CEventEngine)
public:
	CEventEngine();
	~CEventEngine();

public:
	void AsyncAddEvent(EventHandlerPtr_t &pEvtHandler);
	void AsyncRemoveEvent(EventHandlerPtr_t &pEvtHandler);

	inline void CheckOnce();
private:
	///
	typedef std::unordered_set<NSSmartUtils::EventHandlerPtr_t> EventHandlersSet_t;
	EventHandlersSet_t EventHandlersSet_;

	///
	typedef std::vector<NSSmartUtils::EventHandlerPtr_t> EventHandlersVec_t;
	std::mutex EventHandlersAddVecMtx_;
	EventHandlersVec_t TmpAddEventHandlersVec_;
	std::mutex EventHandlersRemoveVecMtx_;
	EventHandlersVec_t TmpRemoveEventHandlersVec_;

	///
	int32_t epfd_;

};

} /* namespace NSSmartUtils */

#endif /* UTILS_EVENTNOTIFIER_H_ */
