#include "..\server\stdafx.h"
#include "EventManager.h"
#include <windows.h>

EventManager::EventManager(const char* pName, bool setAsGlobal)
	:IEventManager(pName, setAsGlobal)
{
	m_activeQueue = 0;
}

EventManager::~EventManager(void){
	//Nothing to do
}

bool EventManager::VAddListener(const EventListenerDelegate& eventDelegate, const EventType& type){
	EventListenerList& eventListenerList = m_eventListeners[type];//created if not found

	for (auto it = eventListenerList.begin(); it != eventListenerList.end(); ++it){
		if ((*it) == eventDelegate){
			//TODO dispaly WARNING : adding twice same delegate
			return false;
		}
	}

	eventListenerList.push_back(eventDelegate);

	return true;
}

bool EventManager::VRemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type){

	bool success = false;
	auto findIt = m_eventListeners.find(type);

	if (findIt != m_eventListeners.end()){
		EventListenerList& eventListenerList = findIt->second;
		for (auto it = eventListenerList.begin(); it != eventListenerList.end(); ++it){
			if ((*it) == eventDelegate){
				//TODO display WARNING : deleted
				eventListenerList.erase(it);
				success = true;
				break;
			}
		}
	}

	return success;
}

bool EventManager::VTriggerEvent(const IEventDataPtr& pEvent){
	if (!pEvent) {
		GCC_ERROR("Event Invalid");
		return false;
	}
	// check is someone is listening for that event
	auto findIt = m_eventListeners.find(pEvent->VGetEventType());
	if (findIt != m_eventListeners.end()) {
		const EventListenerList& eventListeners = findIt->second;
		for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it)
		{
			EventListenerDelegate listener = (*it);
			printf("\nTrigger event %s to listener\n", pEvent->GetName());
			// call each listener
			listener(pEvent);
		}
	}
	return true;
}

bool EventManager::VQueueEvent(const IEventDataPtr& pEvent){
	GCC_ASSERT(m_activeQueue >= 0);
	GCC_ASSERT(m_activeQueue < EVENTMANAGER_NUM_QUEUES);

	if (!pEvent){
		GCC_ERROR("Event Invalid");
		return false;
	}

	// check is someone is listening for that event
	auto findIt = m_eventListeners.find(pEvent->VGetEventType());
	if (findIt != m_eventListeners.end()){
		// m_activeQueue is switched by VUpdate
		m_queues[m_activeQueue].push_back(pEvent);
		printf("Event queued %s\n", pEvent->GetName());
		return true;
	}

	printf("No listener for that event, not queued %s",pEvent->GetName());
	
	return false;
}

bool EventManager::VAbortEvent(const EventType& pEventType, bool allOfType){
	GCC_ASSERT(m_activeQueue >= 0);
	GCC_ASSERT(m_activeQueue < EVENTMANAGER_NUM_QUEUES);

	EventListenerMap::iterator findIt = m_eventListeners.find(pEventType);
	bool success = false;
	// at least one component is listening, else no need to remove something not used
	if (findIt != m_eventListeners.end())
	{
		// reference to a list of objects from the array m_queues
		EventQueue& eventQueue =  m_queues[m_activeQueue];
		auto it = eventQueue.begin();
		while (it != eventQueue.end()){
			// Removing an item from the queue will invalidate the iterator, so have it point to the next member.  
			// All work inside this loop will be done using thisIt.
			auto thisIt = it;
			++it;
			//Dereference of the IEventDataPtr
			if ((*thisIt)->VGetEventType() == pEventType)
			{
				eventQueue.erase(thisIt);
				success = true;
				if (!allOfType)
				{
					break;
				}
			}
		}
	}

	return true;
}

bool EventManager::VUpdate(unsigned long maxMillis){

	unsigned long currMs = GetTickCount();
	unsigned long maxMs = ((maxMillis == IEventManager::kINFINITE) ? IEventManager::kINFINITE : currMs + maxMillis);

	// swap active queue
	int queueToProcess = m_activeQueue;
	m_activeQueue = (m_activeQueue+1)%EVENTMANAGER_NUM_QUEUES;
	m_queues[m_activeQueue].clear();

	// process queue
	while (!m_queues[queueToProcess].empty())
	{
		// get the event to process
		IEventDataPtr pEvent = m_queues[queueToProcess].front();
		//remove the event from the queue
		m_queues[queueToProcess].pop_front();

		const EventType eventType = pEvent->VGetEventType();

		// look for listeners to send the event to
		auto findIt = m_eventListeners.find(eventType);

		if (findIt != m_eventListeners.end())
		{
			const EventListenerList& eventListeners = findIt->second;
			for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it)
			{
				EventListenerDelegate listener = (*it);
				printf("\nsend event %s to listener\n", pEvent->GetName());
				// call each listener
				listener(pEvent);
			}
		}


		// stop if ran out of time
		currMs = GetTickCount();
		if (currMs >= maxMs)
		{
			break;
		}
	}

	bool queueFlushed = m_queues[queueToProcess].empty();
	// events left because of time
	// fill the active queue with remaining events
	if (!queueFlushed){
		while (!m_queues[queueToProcess].empty()){
			IEventDataPtr pEvent = m_queues[queueToProcess].back();
			m_queues[queueToProcess].pop_back();
			m_queues[m_activeQueue].push_front(pEvent);
		}
	}

	return queueFlushed;
}

