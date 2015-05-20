//============================================================================
// Name        : EventNotifier.h
// Author      : TheRockLiuHY
// Date        : Apr 29, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartUtils
//============================================================================

#ifndef UTILS_NOTIFIERS_H_
#define UTILS_NOTIFIERS_H_

#include <base/base.h>
#include <memory>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <signal.h>


namespace ns_smart_utils
{

	class notifier
	{
	public:
		notifier()
		{
		}

		virtual ~notifier()
		{
		}

	public:
		virtual int32_t open() = 0;
		virtual int32_t close() = 0;
		virtual void on_added(bool Suc) = 0;
		virtual void on_removed(bool Suc) = 0;
		virtual int32_t get_fd() = 0;
		virtual uint32_t get_events() = 0;
		virtual void handle_events(uint32_t evts) = 0;
	};
	typedef std::shared_ptr<ns_smart_utils::notifier> notifier_ptr_t;

	/**
	 * timer event handler
	 * */
	class timer_base: public notifier
	{
		DISABLE_COPY(timer_base)
		DISABLE_MOVE(timer_base)

	public:
		enum ETimerType
		{
			ETT_REALTIME = 0, ETT_MONOTONIC = 1
		};

	public:
		explicit timer_base(const ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos);
		virtual ~timer_base();

		int32_t open();
		int32_t close();
		uint32_t get_events();
		void handle_events(uint32_t evts);
		int32_t get_fd()
		{
			return fd_;
		}

		virtual void handle_timeout(uint64_t times) = 0;

	public:
		int64_t get_interval_s()
		{
			return interval_s_;
		}

		int64_t get_interval_ns()
		{
			return interval_ns_;
		}

	private:
		int32_t fd_;
		int32_t timer_type_;
		int64_t init_expire_s_;
		int64_t init_expire_ns_;
		int64_t interval_s_;
		int64_t interval_ns_;
	};
	typedef std::shared_ptr<ns_smart_utils::timer_base> timer_base_ptr_t;

	/**
	 *
	 *
	 * */
	class event_base: public notifier
	{
		DISABLE_COPY(event_base)
		DISABLE_MOVE(event_base)
	public:
		explicit event_base();
		virtual ~event_base();

	public:
		int32_t open();
		int32_t close();
		uint32_t get_events();
		void handle_events(uint32_t evts);
		int32_t get_fd()
		{
			return fd_;
		}
		void notify(uint64_t val);

		virtual void handle_event(uint64_t val) = 0;

	private:
		int32_t fd_;
	};
	typedef std::shared_ptr<ns_smart_utils::event_base> evnet_base_ptr_t;

	/**
	 *
	 *
	 * */
	class signal_base: public notifier
	{
		DISABLE_COPY(signal_base)
		//DISABLE_MOVE(CSignalBase)
	public:
		explicit signal_base(std::vector<int32_t> &&vec);
		virtual ~signal_base();

	public:
		int32_t open();
		int32_t close();
		uint32_t get_events();
		void handle_events(uint32_t evts);
		int32_t get_fd()
		{
			return fd_;
		}

		virtual void handle_signal(int32_t sig) = 0;

	private:
		std::vector<int> signals_;
		int32_t fd_;
	};
	typedef std::shared_ptr<ns_smart_utils::signal_base> signal_base_ptr_t;

	/**
	 *
	 *
	 * */
	class notifier_engine
	{
		DISABLE_COPY(notifier_engine)
		DISABLE_MOVE(notifier_engine)
	public:
		notifier_engine();
		~notifier_engine();

	public:
		///not thread-safe.
		int32_t open();
		///not thread-safe
		int32_t close();
		///
		bool is_opened();
		void async_add_notifier(notifier_ptr_t &pEvtHandler);
		void async_remove_notifier(notifier_ptr_t &pEvtHandler);
		void check_once(int32_t TimeoutMS = -1);
	private:
		///
		typedef std::unordered_set<ns_smart_utils::notifier_ptr_t> notifiers_t;
		notifiers_t notifiers_;

		///
		typedef std::vector<ns_smart_utils::notifier_ptr_t> tmp_notifiers_t;
		std::mutex tmp_notifier_add_mtx_;
		tmp_notifiers_t tmp_add_notifiers_;
		std::mutex tmp_notifier_remove_mtx_;
		tmp_notifiers_t tmp_remove_notifiers_;

		///
		int32_t epfd_;

	};

} /* namespace NSSmartUtils */

#endif /* UTILS_NOTIFIERS_H_ */
