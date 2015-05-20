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

class CMyEvent: public ns_smart_utils::event_base
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

std::shared_ptr<CMyEvent> ptr11;

class CMyTimerHandler: public ns_smart_utils::timer_base
{
public:
	CMyTimerHandler(const ns_smart_utils::timer_base::ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos)
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
		if (ptr11 != nullptr)
		{
			ptr11->notify(ui64Times);
		}
		cout << "interval: " << get_interval_s() << ", times: " << ui64Times << endl;
	}

};

ns_smart_utils::notifier_engine tms;

void f()
{
	for (;;)
	{
		tms.check_once(1);
	}
}

class CMySignalHandler: public ns_smart_utils::signal_base
{
public:
	CMySignalHandler(std::vector<int32_t> & vec)
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

int main()
{
	ns_smart_utils::singleton<int>::get_inst() = 12345;
	std::cout << "value: " << ns_smart_utils::singleton<int>::get_inst() << std::endl;

	std::shared_ptr < ns_smart_utils::timer_base > ptr = std::make_shared < CMyTimerHandler > (ns_smart_utils::timer_base::ETimerType::ETT_REALTIME, 1, 1);
	ptr->open();
	tms.open();

	ns_smart_utils::notifier_ptr_t p = static_pointer_cast<ns_smart_utils::notifier, ns_smart_utils::timer_base>(ptr);
	tms.async_add_notifier(p);

	//std::thread t(f);

	///test signal
	std::vector<int32_t> vec = {SIGINT, SIGQUIT};
	ns_smart_utils::notifier_ptr_t pSignal = std::make_shared < CMySignalHandler > (vec);
	pSignal->open();
	tms.async_add_notifier(pSignal);

	//std::this_thread::sleep_for(10s);

	ptr11 = std::make_shared<CMyEvent>();
	ptr11->open();
	ns_smart_utils::notifier_ptr_t ppp = static_pointer_cast<ns_smart_utils::notifier, ns_smart_utils::event_base>(ptr11);
	tms.async_add_notifier(ppp);

	ptr = nullptr;

	cout << "!!!stop!!!" << endl; // prints !!!Hello World!!!

	//std::this_thread::sleep_for(10s);

	ptr = std::make_shared < CMyTimerHandler > (ns_smart_utils::timer_base::ETimerType::ETT_MONOTONIC, 2, 1);

	p = static_pointer_cast<ns_smart_utils::notifier, ns_smart_utils::timer_base>(ptr);
	ptr->open();
	tms.async_add_notifier(p);

	ptr = std::make_shared < CMyTimerHandler > (ns_smart_utils::timer_base::ETimerType::ETT_REALTIME, 3, 1);
	p = static_pointer_cast<ns_smart_utils::notifier, ns_smart_utils::timer_base>(ptr);
	ptr->open();
	tms.async_add_notifier(p);

	ptr = std::make_shared < CMyTimerHandler > (ns_smart_utils::timer_base::ETimerType::ETT_MONOTONIC, 5, 1);
	p = static_pointer_cast<ns_smart_utils::notifier, ns_smart_utils::timer_base>(ptr);
	ptr->open();
	tms.async_add_notifier(p);

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

