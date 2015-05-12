//============================================================================
// Name        : EventNotifier.cpp
// Author      : TheRockLiuHY
// Date        : Apr 29, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartUtils
//============================================================================

#include <sys/epoll.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>

#include "EventNotifier.h"

namespace NSSmartUtils
{

	CNotifierEngine::CNotifierEngine()
			: epfd_(-1)
	{
	}

	CNotifierEngine::~CNotifierEngine()
	{
	}

	void CNotifierEngine::AsyncAddEvent(EventNotifierPtr_t& pEvtHandler)
	{
		std::lock_guard < std::mutex > lock(EventHandlersAddVecMtx_);
		if (!Opened())
		{
			SU_ASSERT(false)
			return;
		}
		TmpAddEventHandlersVec_.push_back(pEvtHandler);
	}

	void CNotifierEngine::AsyncRemoveEvent(EventNotifierPtr_t& pEvtHandler)
	{
		std::lock_guard < std::mutex > lock(EventHandlersRemoveVecMtx_);
		if (!Opened())
		{
			SU_ASSERT(false)
			return;
		}
		TmpRemoveEventHandlersVec_.push_back(pEvtHandler);
	}

#define MAX_EVENTS (100)
	void CNotifierEngine::CheckOnce(int32_t TimeoutMS)
	{
		if (!Opened())
		{
			SU_ASSERT(false)
			return;
		}

		if (!TmpRemoveEventHandlersVec_.empty())
		{
			std::vector<EventNotifierPtr_t> TmpVec;

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
				(*iter)->OnRemoved(0 == st ? false : (epoll_ctl(epfd_, EPOLL_CTL_DEL, (*iter)->GetFD(),
				NULL) == 0));
			}
		}

		if (!TmpAddEventHandlersVec_.empty())
		{
			std::vector<EventNotifierPtr_t> TmpVec;

			{
				std::lock_guard < std::mutex > lock(EventHandlersAddVecMtx_);
				TmpAddEventHandlersVec_.swap(TmpVec);
			}
			SU_ASSERT(TmpAddEventHandlersVec_.empty())

			for (EventHandlersVec_t::iterator iter = TmpVec.begin(); iter != TmpVec.end(); iter++)
			{
				std::pair<EventHandlersSet_t::iterator, bool> ret = EventHandlersSet_.insert(*iter);
				SU_ASSERT(ret.second)
				struct epoll_event evt =
				{ (*iter)->GetEvents(),
				{ (*iter).get() }, };
				(*iter)->OnAdded((!ret.second) ? false : (epoll_ctl(epfd_, EPOLL_CTL_ADD, (*iter)->GetFD(), &evt) == 0));	//you see, i wanna you get the errno:)...
			}
		}

		struct epoll_event events[MAX_EVENTS];

		int32_t nfds = epoll_wait(epfd_, events, MAX_EVENTS, TimeoutMS);
		if (nfds == -1)
		{
			SU_ASSERT(false)
			return;
		}

		for (int32_t n = 0; n < nfds; ++n)
		{
			IEventNotifier* pHandler = static_cast<IEventNotifier*>(events[n].data.ptr);
			SU_ASSERT(NULL != pHandler)
			pHandler->HandleEvents(events[n].events);
		}

	}

	const int64_t NANOS_OF_ONE_SECONDS = (1000 * 1000 * 1000);

	CTimerBase::CTimerBase(const ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos)
			: fd_(-1), TimerType_(timer_type), InitExpireSeconds_(interval_seconds), InitExpireNanos_(interval_nanos), IntervalSeconds_(interval_seconds), IntervalNanos_(interval_nanos)
	{

	}

	CTimerBase::~CTimerBase()
	{
		if (-1 == fd_)
		{
			close(fd_);
		}
	}

	int32_t CTimerBase::Open()
	{

		SU_ASSERT(InitExpireNanos_ < NANOS_OF_ONE_SECONDS && IntervalNanos_ < NANOS_OF_ONE_SECONDS);

		if (ETT_REALTIME != TimerType_ && ETT_MONOTONIC != TimerType_)
		{
			return EEC_ERR;
		}

		int32_t timer_type = ETT_REALTIME == TimerType_ ? CLOCK_REALTIME : CLOCK_MONOTONIC;

		struct timespec now =
		{ 0 };
		int32_t flags = 0;
#if (__ABS_TIME__)
		if (clock_gettime(timer_type, &now) == -1)
		{
			return EEC_ERR;
		}
		flags = TFD_TIMER_ABSTIME;
#endif

		struct itimerspec new_value =
		{ 0 };
		new_value.it_value.tv_sec = now.tv_sec + InitExpireSeconds_ + (now.tv_nsec + InitExpireNanos_) / NANOS_OF_ONE_SECONDS;
		new_value.it_value.tv_nsec = (now.tv_nsec + InitExpireNanos_) % NANOS_OF_ONE_SECONDS;
		new_value.it_interval.tv_sec = IntervalSeconds_ + IntervalNanos_ / NANOS_OF_ONE_SECONDS;
		new_value.it_interval.tv_nsec = IntervalNanos_ % NANOS_OF_ONE_SECONDS;

		fd_ = timerfd_create(timer_type, 0);
		if (fd_ == -1)
		{
			return EEC_ERR;
		}

		if (timerfd_settime(fd_, flags, &new_value, NULL) == -1)
		{
			return EEC_ERR;
		}

		return EEC_SUC;

	}

	int32_t CTimerBase::Close()
	{
		SU_ASSERT(-1 == !fd_)
		close(fd_);
		fd_ = -1;

		return EEC_ERR;
	}

	uint32_t CTimerBase::GetEvents()
	{
		return EPOLLIN;
	}

	void CTimerBase::HandleEvents(uint32_t events)
	{
		SU_ASSERT(EPOLLIN == events)

		uint64_t times = 0;
		ssize_t s = read(fd_, &times, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{
			SU_ASSERT(false);
			return;
		}

		HandleTimeout(times);
	}

	int32_t CNotifierEngine::Open()
	{
		std::lock_guard < std::mutex > A(EventHandlersRemoveVecMtx_);
		std::lock_guard < std::mutex > B(EventHandlersAddVecMtx_);

		if (Opened())
		{
			return EEC_REDO_ERR;
		}

		SU_ASSERT(TmpAddEventHandlersVec_.empty() && TmpRemoveEventHandlersVec_.empty() && EventHandlersSet_.empty())
		/*
		 * In the initial epoll_create() implementation, the size argument informed the kernel of the number  of  file  descriptors  that  the  caller
		 expected  to add to the epoll instance.  The kernel used this information as a hint for the amount of space to initially allocate in inter‐
		 nal data structures describing events.  (If necessary, the kernel would allocate more space if the caller's usage exceeded the  hint  given
		 in  size.)  Nowadays, this hint is no longer required (the kernel dynamically sizes the required data structures without needing the hint),
		 but size must still be greater than zero, in order to ensure backward compatibility when new epoll applications are run on older kernels.
		 * */
		return ((epfd_ = epoll_create(MAX_EVENTS)) == -1 ? EEC_ERR : EEC_SUC);
	}

	int32_t CNotifierEngine::Close()
	{
		std::lock_guard < std::mutex > A(EventHandlersRemoveVecMtx_);
		std::lock_guard < std::mutex > B(EventHandlersAddVecMtx_);

		if (!Opened())
		{
			return EEC_ERR;
		}

		TmpAddEventHandlersVec_.clear();
		TmpRemoveEventHandlersVec_.clear();
		EventHandlersSet_.clear();

		close(epfd_);
		epfd_ = -1;

		return EEC_SUC;
	}

	bool CNotifierEngine::Opened()
	{
		return !(-1 == epfd_);
	}

	CEventBase::CEventBase()
			: fd_(-1)
	{
	}

	CEventBase::~CEventBase()
	{
		Close();
	}

	int32_t CEventBase::Open()
	{
		return (fd_ = eventfd(0, 0)) == -1 ? EEC_ERR : EEC_SUC;
	}

	int32_t CEventBase::Close()
	{
		if (-1 != fd_)
		{
			close(fd_);
			fd_ = -1;

			return EEC_SUC;
		}

		return EEC_ERR;
	}

	uint32_t CEventBase::GetEvents()
	{
		return EPOLLIN;
	}

	void CEventBase::Notify(uint64_t val)
	{
		write(fd_, &val, sizeof(uint64_t));
	}

	void CEventBase::HandleEvents(uint32_t evts)
	{
		SU_ASSERT(EPOLLIN == evts)

		uint64_t val = 0;
		ssize_t s = read(fd_, &val, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{
			SU_ASSERT(false);
			return;
		}

		HandleEvent(val);
	}

}

/* namespace NSSmartUtils */

