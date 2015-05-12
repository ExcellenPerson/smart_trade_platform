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

#include <Utils/EventNotifier.h>
#include <Utils/Singleton.h>

class CMyTimerHandler: public NSSmartUtils::CTimerEventHandler
{
public:
	CMyTimerHandler(const NSSmartUtils::CTimerEventHandler::ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos)
			: CTimerEventHandler(timer_type, interval_seconds, interval_nanos)
	{
	}

public:

	void OnAdded(bool b)
	{
		cout<<"added: " <<b<<endl;
	}

	void OnRemoved(bool b)
	{
		cout<<"removed: " <<b<<endl;
	}

	void HandleTimer(uint64_t ui64Times)
	{
		cout << "interval: " << GetIntervalSeconds() << ", times: " << ui64Times << endl;
	}

};

NSSmartUtils::CEventEngine tms;

void f()
{
	for (;;)
	{
		tms.CheckOnce(1);
	}
}

int main()
{
	NSSmartUtils::CSingleton<int>::GetInst() = 12345;
	std::cout << "value: " << NSSmartUtils::CSingleton<int>::GetInst() << std::endl;

	std::shared_ptr<NSSmartUtils::CTimerEventHandler> ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerEventHandler::ETimerType::ETT_REALTIME, 1, 1);

	ptr->Open();
	tms.Open();

	NSSmartUtils::EventHandlerPtr_t p = static_pointer_cast<NSSmartUtils::IEventHandler, NSSmartUtils::CTimerEventHandler>(ptr);
	tms.AsyncAddEvent(p);

	std::thread t(f);

	std::this_thread::sleep_for(10s);

	ptr = nullptr;

	cout << "!!!stop!!!" << endl; // prints !!!Hello World!!!

	std::this_thread::sleep_for(10s);

	ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerEventHandler::ETimerType::ETT_MONOTONIC, 2, 1);

	p = static_pointer_cast<NSSmartUtils::IEventHandler, NSSmartUtils::CTimerEventHandler>(ptr);
	ptr->Open();
	tms.AsyncAddEvent(p);

	ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerEventHandler::ETimerType::ETT_REALTIME, 3, 1);
	p = static_pointer_cast<NSSmartUtils::IEventHandler, NSSmartUtils::CTimerEventHandler>(ptr);
	ptr->Open();
	tms.AsyncAddEvent(p);

	ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerEventHandler::ETimerType::ETT_MONOTONIC, 5, 1);
	p = static_pointer_cast<NSSmartUtils::IEventHandler, NSSmartUtils::CTimerEventHandler>(ptr);
	ptr->Open();
	tms.AsyncAddEvent(p);

	std::this_thread::sleep_for(20s);

	tms.AsyncRemoveEvent(p);

	//ptr1 = nullptr;

	std::this_thread::sleep_for(20s);

	//ptr2 = nullptr;

	std::this_thread::sleep_for(20s);

	//tms.Stop();

	cout << "!!!test over!!!" << endl; // prints !!!Hello World!!!
	return 0;
}

#endif

