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
#include <sys/signalfd.h>

#include <utils/notifiers.h>

namespace ns_smart_utils
{

	notifier_engine::notifier_engine()
			: epfd_(-1)
	{
	}

	notifier_engine::~notifier_engine()
	{
	}

	void notifier_engine::async_add_event(notifier_ptr_t& pEvtHandler)
	{
		std::lock_guard < std::mutex > lock(tmp_notifier_add_mtx_);
		if (!is_opened())
		{
			SU_ASSERT(false)
			return;
		}
		tmp_add_notifiers_.push_back(pEvtHandler);
	}

	void notifier_engine::async_remove_event(notifier_ptr_t& pEvtHandler)
	{
		std::lock_guard < std::mutex > lock(tmp_notifier_remove_mtx_);
		if (!is_opened())
		{
			SU_ASSERT(false)
			return;
		}
		tmp_remove_notifiers_.push_back(pEvtHandler);
	}

#define MAX_EVENTS (100)
	void notifier_engine::check_once(int32_t TimeoutMS)
	{
		if (!is_opened())
		{
			SU_ASSERT(false)
			return;
		}

		if (!tmp_remove_notifiers_.empty())
		{
			std::vector<notifier_ptr_t> TmpVec;

			{
				std::lock_guard < std::mutex > lock(tmp_notifier_remove_mtx_);
				tmp_remove_notifiers_.swap(TmpVec);
			}
			SU_ASSERT(tmp_remove_notifiers_.empty())

			for (tmp_notifiers_t::iterator iter = TmpVec.begin(); iter != TmpVec.end(); iter++)
			{
				///In  kernel  versions  before 2.6.9, the EPOLL_CTL_DEL operation required a non-null pointer in event, even though this argument is ignored.
				///Since Linux 2.6.9, event can be specified as NULL when using EPOLL_CTL_DEL.  Applications that need to be portable to kernels before  2.6.9
				///should specify a non-null pointer in event.
				notifiers_t::size_type st = notifiers_.erase(*iter);
				SU_ASSERT(1 == st)
				(*iter)->OnRemoved(0 == st ? false : (epoll_ctl(epfd_, EPOLL_CTL_DEL, (*iter)->GetFD(),
				NULL) == 0));
			}
		}

		if (!tmp_add_notifiers_.empty())
		{
			std::vector<notifier_ptr_t> TmpVec;

			{
				std::lock_guard < std::mutex > lock(tmp_notifier_add_mtx_);
				tmp_add_notifiers_.swap(TmpVec);
			}
			SU_ASSERT(tmp_add_notifiers_.empty())

			for (tmp_notifiers_t::iterator iter = TmpVec.begin(); iter != TmpVec.end(); iter++)
			{
				std::pair<notifiers_t::iterator, bool> ret = notifiers_.insert(*iter);
				SU_ASSERT(ret.second)
				struct epoll_event evt =
				{ (*iter)->get_events(),
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
			notifier* pHandler = static_cast<notifier*>(events[n].data.ptr);
			SU_ASSERT(NULL != pHandler)
			pHandler->handle_events(events[n].events);
		}

	}

	const int64_t NANOS_OF_ONE_SECONDS = (1000 * 1000 * 1000);

	timer_base::timer_base(const ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos)
			: fd_(-1), TimerType_(timer_type), init_expire_s_(interval_seconds), init_expire_ns_(interval_nanos), interval_s_(interval_seconds), interval_ns_(interval_nanos)
	{

	}

	timer_base::~timer_base()
	{
		if (-1 == fd_)
		{
			close(fd_);
		}
	}

	int32_t timer_base::open()
	{

		SU_ASSERT(init_expire_ns_ < NANOS_OF_ONE_SECONDS && interval_ns_ < NANOS_OF_ONE_SECONDS);

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
		new_value.it_value.tv_sec = now.tv_sec + init_expire_s_ + (now.tv_nsec + init_expire_ns_) / NANOS_OF_ONE_SECONDS;
		new_value.it_value.tv_nsec = (now.tv_nsec + init_expire_ns_) % NANOS_OF_ONE_SECONDS;
		new_value.it_interval.tv_sec = interval_s_ + interval_ns_ / NANOS_OF_ONE_SECONDS;
		new_value.it_interval.tv_nsec = interval_ns_ % NANOS_OF_ONE_SECONDS;

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

	int32_t timer_base::close()
	{
		SU_ASSERT(-1 == !fd_)
		close(fd_);
		fd_ = -1;

		return EEC_ERR;
	}

	uint32_t timer_base::get_events()
	{
		return EPOLLIN;
	}

	void timer_base::handle_events(uint32_t events)
	{
		SU_ASSERT(EPOLLIN == events)

		uint64_t times = 0;
		ssize_t s = read(fd_, &times, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{
			SU_ASSERT(false);
			return;
		}

		handle_timeout(times);
	}

	int32_t notifier_engine::open()
	{
		std::lock_guard < std::mutex > A(tmp_notifier_remove_mtx_);
		std::lock_guard < std::mutex > B(tmp_notifier_add_mtx_);

		if (is_opened())
		{
			return EEC_REDO_ERR;
		}

		SU_ASSERT(tmp_add_notifiers_.empty() && tmp_remove_notifiers_.empty() && notifiers_.empty())
		/*
		 * In the initial epoll_create() implementation, the size argument informed the kernel of the number  of  file  descriptors  that  the  caller
		 expected  to add to the epoll instance.  The kernel used this information as a hint for the amount of space to initially allocate in inter‐
		 nal data structures describing events.  (If necessary, the kernel would allocate more space if the caller's usage exceeded the  hint  given
		 in  size.)  Nowadays, this hint is no longer required (the kernel dynamically sizes the required data structures without needing the hint),
		 but size must still be greater than zero, in order to ensure backward compatibility when new epoll applications are run on older kernels.
		 * */
		return ((epfd_ = epoll_create(MAX_EVENTS)) == -1 ? EEC_ERR : EEC_SUC);
	}

	int32_t notifier_engine::close()
	{
		std::lock_guard < std::mutex > A(tmp_notifier_remove_mtx_);
		std::lock_guard < std::mutex > B(tmp_notifier_add_mtx_);

		if (!is_opened())
		{
			return EEC_ERR;
		}

		tmp_add_notifiers_.clear();
		tmp_remove_notifiers_.clear();
		notifiers_.clear();

		close(epfd_);
		epfd_ = -1;

		return EEC_SUC;
	}

	bool notifier_engine::is_opened()
	{
		return !(-1 == epfd_);
	}

	event_base::event_base()
			: fd_(-1)
	{
	}

	event_base::~event_base()
	{
		close();
	}

	int32_t event_base::open()
	{
		return (fd_ = eventfd(0, 0)) == -1 ? EEC_ERR : EEC_SUC;
	}

	int32_t event_base::close()
	{
		if (-1 != fd_)
		{
			close(fd_);
			fd_ = -1;

			return EEC_SUC;
		}

		return EEC_ERR;
	}

	uint32_t event_base::get_events()
	{
		return EPOLLIN;
	}

	void event_base::notify(uint64_t val)
	{
		write(fd_, &val, sizeof(uint64_t));
	}

	void event_base::handle_events(uint32_t evts)
	{
		SU_ASSERT(EPOLLIN == evts)

		uint64_t val = 0;
		ssize_t s = read(fd_, &val, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{
			SU_ASSERT(false);
			return;
		}

		handle_event(val);
	}

	signal_base::signal_base(std::vector<int32_t> &&vec)
			: signals_(vec)
	{
	}

	signal_base::~signal_base()
	{
	}

	int32_t signal_base::open()
	{
		sigset_t mask;
		sigemptyset(&mask);
		SU_ASSERT(!signals_.empty())
		for (std::vector<int32_t>::iterator iter = signals_.begin(); iter != signals_.end(); iter++)
		{
			sigaddset(&mask, *iter);
			//std::cout<<*iter<<std::endl;
		}

		if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
		{
			SU_ASSERT(false);
			return EEC_ERR;
		}

		fd_ = signalfd(-1, &mask, SFD_NONBLOCK);
		if (fd_ == -1)
		{
			SU_ASSERT(false);
			return EEC_ERR;
		}

		return EEC_SUC;
	}

	int32_t signal_base::close()
	{
		SAFE_CLOSE_FD(fd_)

		return EEC_SUC;
	}

	uint32_t signal_base::get_events()
	{
		return EPOLLIN;
	}

	void signal_base::handle_events(uint32_t evts)
	{
		//std::cout<<"get signal"<<std::endl;
		struct signalfd_siginfo fdsi;
		ssize_t s;
		for (;;)
		{
			s = read(fd_, &fdsi, sizeof(struct signalfd_siginfo));
			if (s != sizeof(struct signalfd_siginfo))
			{
				SU_ASSERT(EAGAIN == errno)
				break;
			}

			handle_signal(fdsi.ssi_signo);
		}
	}

}

/* namespace NSSmartUtils */

