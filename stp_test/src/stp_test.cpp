//============================================================================
// Name        : STPTest.cpp
// Author      : theRockLiuhy
// Version     :
// Copyright   : liuhongyang's copyright
// Description : Hello World in C++, Ansi-style
//============================================================================

#define __TEST_TIMER__ (1)

#if (__TEST_TIMER__)

#include <iostream>
#include <thread>
using namespace std;

#include <utils/notifiers.h>
#include <utils/singleton.h>

class my_event: public smart_utils::event_base
{
public:
	void on_added(bool b)
	{
		cout << "add event: " << b << endl;
	}

	void on_removed(bool b)
	{
		cout << "removed: " << b << endl;
	}

	void handle_event(uint64_t val)
	{
		cout << "event: " << val << endl;
	}

};

smart_utils::event_base::pointer_t pevent;

class my_timer: public smart_utils::timer_base
{
public:
	my_timer(const smart_utils::timer_base::ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos)
			: timer_base(timer_type, interval_seconds, interval_nanos)
	{
	}

public:

	void on_added(bool b)
	{
		cout << "added: " << b << endl;
	}

	void on_removed(bool b)
	{
		cout << "removed: " << b << endl;
	}

	void handle_timeout(uint64_t ui64Times)
	{
		if (pevent != nullptr)
		{
			pevent->notify(ui64Times);
		}
		cout << "interval: " << get_interval_s() << ", times: " << ui64Times << endl;
	}

};


class my_signal: public smart_utils::signal_base
{
public:
	my_signal(std::vector<int32_t> & vec)
			: signal_base(std::move(vec))
	{

	}

	virtual void on_added(bool Suc)
	{
		cout<<"signal added" <<endl;
	}
	virtual void on_removed(bool Suc)
	{

	}

public:
	void handle_signal(int32_t sig)
	{
		cout<<"Got signal : " << sig <<endl;
	}

};

smart_utils::notifier_engine tms;

void f()
{
	for (;;)
	{
		tms.check_once(1);
	}
}

int main()
{
	///test singleton
	smart_utils::singleton<int>::get_inst() = 12345;
	std::cout << "value: " << smart_utils::singleton<int>::get_inst() << std::endl;

	///test notifier engine.
	///start the engine
	tms.open();

	///test timer.
	smart_utils::timer_base::pointer_t ptmp = std::make_shared < my_timer > (smart_utils::timer_base::ETimerType::ETT_REALTIME, 1, 1);
	smart_utils::notifier::pointer_t ptimer = static_pointer_cast<smart_utils::notifier, smart_utils::timer_base>(ptmp);
	ptimer->open();
	tms.async_add_notifier(ptimer);

	//std::thread t(f);

	///test signal
	std::vector<int32_t> vec = {SIGINT, SIGQUIT};
	smart_utils::notifier::pointer_t psignal = std::make_shared < my_signal > (vec);
	psignal->open();
	tms.async_add_notifier(psignal);

	//std::this_thread::sleep_for(10s);
	///test event
	pevent = std::make_shared<my_event>();
	smart_utils::notifier::pointer_t pevt = static_pointer_cast<smart_utils::notifier, smart_utils::event_base>(pevent);
	pevt->open();
	tms.async_add_notifier(pevt);

	//std::this_thread::sleep_for(10s);
	///test timer
	smart_utils::notifier::pointer_t ptm1 = std::make_shared < my_timer > (smart_utils::timer_base::ETimerType::ETT_MONOTONIC, 2, 1);
	ptm1->open();
	tms.async_add_notifier(ptm1);

	///test timer
	smart_utils::notifier::pointer_t ptm2 = std::make_shared < my_timer > (smart_utils::timer_base::ETimerType::ETT_REALTIME, 3, 1);
	ptm2->open();
	tms.async_add_notifier(ptm2);


	///test timer
	smart_utils::notifier::pointer_t ptm3 = std::make_shared < my_timer > (smart_utils::timer_base::ETimerType::ETT_MONOTONIC, 5, 1);
	//ptimer = static_pointer_cast<smart_utils::notifier, smart_utils::timer_base>(ptr);
	ptm3->open();
	tms.async_add_notifier(ptm3);

	//std::this_thread::sleep_for(20s);

	//tms.AsyncRemoveEvent(p);

	//ptr1 = nullptr;

	//std::this_thread::sleep_for(20s);

	//ptr2 = nullptr;

	//std::this_thread::sleep_for(20s);

	//tms.Stop();

	f();

	std::this_thread::sleep_for(50s);


	cout << "!!!test over!!!" << endl; // prints !!!Hello World!!!
	return 0;
}

#endif

