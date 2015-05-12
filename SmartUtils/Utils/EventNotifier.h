//============================================================================
// Name        : EventNotifier.h
// Author      : TheRockLiuHY
// Date        : Apr 29, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartUtils
//============================================================================

#ifndef UTILS_EVENTNOTIFIER_H_
#define UTILS_EVENTNOTIFIER_H_

#include <memory>
#include <unordered_set>
#include <vector>
#include <mutex>

#include "../Base/Base.h"

namespace NSSmartUtils
{

	class IEventNotifier
	{
	public:
		IEventNotifier()
		{
		}
		virtual ~IEventNotifier()
		{
		}

	public:
		virtual int32_t Open() = 0;
		virtual int32_t Close() = 0;
		virtual void OnAdded(bool Suc) = 0;
		virtual void OnRemoved(bool Suc) = 0;
		virtual int32_t GetFD() = 0;
		virtual uint32_t GetEvents() = 0;
		virtual void HandleEvents(uint32_t evts) = 0;
	};
	typedef std::shared_ptr<NSSmartUtils::IEventNotifier> EventNotifierPtr_t;

	/**
	 * timer event handler
	 * */
	class CTimerBase: public IEventNotifier
	{
		DISABLE_COPY(CTimerBase)
		DISABLE_MOVE(CTimerBase)

	public:
		enum ETimerType
		{
			ETT_REALTIME = 0, ETT_MONOTONIC = 1
		};

	public:
		explicit CTimerBase(const ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos);
		virtual ~CTimerBase();

		int32_t Open();
		int32_t Close();
		uint32_t GetEvents();
		void HandleEvents(uint32_t evts);
		int32_t GetFD()
		{
			return fd_;
		}

		virtual void HandleTimeout(uint64_t times) = 0;

	public:
		int64_t GetIntervalSeconds()
		{
			return IntervalSeconds_;
		}

		int64_t GetIntervalNanos()
		{
			return IntervalNanos_;
		}

	private:
		int32_t fd_;
		int32_t TimerType_;
		int64_t InitExpireSeconds_;
		int64_t InitExpireNanos_;
		int64_t IntervalSeconds_;
		int64_t IntervalNanos_;
	};
	typedef std::shared_ptr<NSSmartUtils::CTimerBase> TimerBasePtr_t;

	/**
	 *
	 *
	 * */
	class CEventBase: public IEventNotifier
	{
	public:
		explicit CEventBase();
		virtual ~CEventBase();

	public:
		int32_t Open();
		int32_t Close();
		uint32_t GetEvents();
		void HandleEvents(uint32_t evts);
		int32_t GetFD()
		{
			return fd_;
		}
		void Notify(uint64_t val);

		virtual void HandleEvent(uint64_t val) = 0;

	private:
		int32_t fd_;
	};
	typedef std::shared_ptr<NSSmartUtils::CEventBase> EventBasePtr_t;

	/**
	 *
	 *
	 * */
	class CNotifierEngine
	{
		DISABLE_COPY(CNotifierEngine)
		DISABLE_MOVE(CNotifierEngine)
	public:
		CNotifierEngine();
		~CNotifierEngine();

	public:
		///not thread-safe.
		int32_t Open();
		///not thread-safe
		int32_t Close();
		///
		bool Opened();
		void AsyncAddEvent(EventNotifierPtr_t &pEvtHandler);
		void AsyncRemoveEvent(EventNotifierPtr_t &pEvtHandler);
		void CheckOnce(int32_t TimeoutMS = -1);
	private:
		///
		typedef std::unordered_set<NSSmartUtils::EventNotifierPtr_t> EventHandlersSet_t;
		EventHandlersSet_t EventHandlersSet_;

		///
		typedef std::vector<NSSmartUtils::EventNotifierPtr_t> EventHandlersVec_t;
		std::mutex EventHandlersAddVecMtx_;
		EventHandlersVec_t TmpAddEventHandlersVec_;
		std::mutex EventHandlersRemoveVecMtx_;
		EventHandlersVec_t TmpRemoveEventHandlersVec_;

		///
		int32_t epfd_;

	};

} /* namespace NSSmartUtils */

#endif /* UTILS_EVENTNOTIFIER_H_ */
