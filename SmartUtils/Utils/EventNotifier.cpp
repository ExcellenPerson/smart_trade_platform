/*
 * IEvtNotifier.cpp
 *
 *  Created on: May 11, 2015
 *      Author: rock
 */

#include <sys/epoll.h>

#include "EventNotifier.h"

namespace NSSmartUtils
{

	NSSmartUtils::IEventHandler::IEventHandler()
	{
	}

	NSSmartUtils::IEventHandler::~IEventHandler()
	{
	}

	NSSmartUtils::CEventEngine::CEventEngine()
	{
	}

	NSSmartUtils::CEventEngine::~CEventEngine()
	{
	}

	void CEventEngine::AsyncAddEvent(EventHandlerPtr_t& pEvtHandler)
	{
		std::lock_guard < std::mutex > lock(EventHandlersAddVecMtx_);
		TmpAddEventHandlersVec_.push_back(pEvtHandler);
	}

	void CEventEngine::AsyncRemoveEvent(EventHandlerPtr_t& pEvtHandler)
	{
		std::lock_guard < std::mutex > lock(EventHandlersRemoveVecMtx_);
		TmpRemoveEventHandlersVec_.push_back(pEvtHandler);
	}

#define INIT_VEC_CNT (100)
	inline void CEventEngine::CheckOnce()
	{


		if (!TmpRemoveEventHandlersVec_.empty())
		{
			std::vector<EventHandlerPtr_t> TmpVec;

			{
				std::lock_guard < std::mutex > lock(EventHandlersRemoveVecMtx_);
				TmpRemoveEventHandlersVec_.swap(TmpVec);
			}
			SU_ASSERT(TmpRemoveEventHandlersVec_.empty())

			for (EventHandlersVec_t::iterator iter = TmpVec.begin(); iter != TmpVec.end(); iter++)
			{
				///In  kernel  versions  before 2.6.9, the EPOLL_CTL_DEL operation required a non-null pointer in event, even though this argument is ignored.
				///Since Linux 2.6.9, event can be specified as NULL when using EPOLL_CTL_DEL.  Applications that need to be portable to kernels before  2.6.9
				///should specify a non-null pointer in event.
				EventHandlersSet_t::size_type st = EventHandlersSet_.erase(*iter);
				SU_ASSERT(1 == st)
				(*iter)->OnRemoved(0 == st ? false : (epoll_ctl(epfd_, EPOLL_CTL_DEL, (*iter)->GetFD(), NULL) == 0));
			}
		}


		if (!TmpAddEventHandlersVec_.empty())
		{
			std::vector<EventHandlerPtr_t> TmpVec;

			{
				std::lock_guard < std::mutex > lock(EventHandlersAddVecMtx_);
				TmpAddEventHandlersVec_.swap(TmpVec);
			}
			SU_ASSERT(TmpAddEventHandlersVec_.empty())

			for (EventHandlersVec_t::iterator iter = TmpVec.begin(); iter != TmpVec.end(); iter++)
			{
				std::pair<EventHandlersSet_t::iterator, bool> ret = EventHandlersSet_.insert(*iter);
				SU_ASSERT(ret.second)
				struct epoll_event evt = {.events = (*iter)->GetEvents(), {.ptr = static_cast<void*>((*iter).get())}};
				(*iter)->OnAdded((!ret.second) ? false : (epoll_ctl(epfd_, EPOLL_CTL_ADD, (*iter)->GetFD(), &evt) == 0));//you see, i wanna you get the errno:)...
			}
		}

		///
		for (;;)
		{

		}

	}

} /* namespace NSSmartUtils */

