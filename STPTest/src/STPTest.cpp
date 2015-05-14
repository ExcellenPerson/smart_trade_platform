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

class CMyEvent: public NSSmartUtils::CEventBase
{
public:
	void OnAdded(bool b)
	{
		cout << "add event: " << b << endl;
	}

	void OnRemoved(bool b)
	{
		cout << "removed: " << b << endl;
	}

	void HandleEvent(uint64_t val)
	{
		cout << "event: " << val << endl;
	}

};

std::shared_ptr<CMyEvent> ptr11;

class CMyTimerHandler: public NSSmartUtils::CTimerBase
{
public:
	CMyTimerHandler(const NSSmartUtils::CTimerBase::ETimerType timer_type, int64_t interval_seconds, int64_t interval_nanos)
			: CTimerBase(timer_type, interval_seconds, interval_nanos)
	{
	}

public:

	void OnAdded(bool b)
	{
		cout << "added: " << b << endl;
	}

	void OnRemoved(bool b)
	{
		cout << "removed: " << b << endl;
	}

	void HandleTimeout(uint64_t ui64Times)
	{
		if (ptr11 != nullptr)
		{
			ptr11->Notify(ui64Times);
		}
		cout << "interval: " << GetIntervalSeconds() << ", times: " << ui64Times << endl;
	}

};

NSSmartUtils::CNotifierEngine tms;

void f()
{
	for (;;)
	{
		tms.CheckOnce(1);
	}
}

class CMySignalHandler: public NSSmartUtils::CSignalBase
{
public:
	CMySignalHandler(std::vector<int32_t> & vec)
			: CSignalBase(std::move(vec))
	{

	}

	virtual void OnAdded(bool Suc)
	{
		cout<<"signal added" <<endl;
	}
	virtual void OnRemoved(bool Suc)
	{

	}

public:
	void HandleSignal(int32_t sig)
	{
		cout<<"Got signal : " << sig <<endl;
	}

};

int main()
{
	NSSmartUtils::CSingleton<int>::GetInst() = 12345;
	std::cout << "value: " << NSSmartUtils::CSingleton<int>::GetInst() << std::endl;

	std::shared_ptr < NSSmartUtils::CTimerBase > ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerBase::ETimerType::ETT_REALTIME, 1, 1);
	ptr->Open();
	tms.Open();

	NSSmartUtils::EventNotifierPtr_t p = static_pointer_cast<NSSmartUtils::IEventNotifier, NSSmartUtils::CTimerBase>(ptr);
	tms.AsyncAddEvent(p);

	//std::thread t(f);

	///test signal
	std::vector<int32_t> vec = {SIGINT, SIGQUIT};
	NSSmartUtils::EventNotifierPtr_t pSignal = std::make_shared < CMySignalHandler > (vec);
	pSignal->Open();
	tms.AsyncAddEvent(pSignal);

	//std::this_thread::sleep_for(10s);

	ptr11 = std::make_shared<CMyEvent>();
	ptr11->Open();
	NSSmartUtils::EventNotifierPtr_t ppp = static_pointer_cast<NSSmartUtils::IEventNotifier, NSSmartUtils::CEventBase>(ptr11);
	tms.AsyncAddEvent(ppp);

	ptr = nullptr;

	cout << "!!!stop!!!" << endl; // prints !!!Hello World!!!

	//std::this_thread::sleep_for(10s);

	ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerBase::ETimerType::ETT_MONOTONIC, 2, 1);

	p = static_pointer_cast<NSSmartUtils::IEventNotifier, NSSmartUtils::CTimerBase>(ptr);
	ptr->Open();
	tms.AsyncAddEvent(p);

	ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerBase::ETimerType::ETT_REALTIME, 3, 1);
	p = static_pointer_cast<NSSmartUtils::IEventNotifier, NSSmartUtils::CTimerBase>(ptr);
	ptr->Open();
	tms.AsyncAddEvent(p);

	ptr = std::make_shared < CMyTimerHandler > (NSSmartUtils::CTimerBase::ETimerType::ETT_MONOTONIC, 5, 1);
	p = static_pointer_cast<NSSmartUtils::IEventNotifier, NSSmartUtils::CTimerBase>(ptr);
	ptr->Open();
	tms.AsyncAddEvent(p);

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

