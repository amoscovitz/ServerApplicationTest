#include "..\server\event.h"
#include "..\ThirdPart\FastDelegate.h"

typedef fastdelegate::FastDelegate1<IEventDataPtr> EventListenerDelegate;
//typedef concurrent_queue<IEventDataPtr> ThreadSafeEventQueue;

class IEventManager
{
public:

	enum eConstants {kINFINITE=0xffffffff };

	/*
	* pName : name of the manager
	* setAsGlobal : singleton or not
	*/
	explicit IEventManager(const char* pName, bool setAsGlobal);

	virtual ~IEventManager(void);

	virtual bool VAddListener(const EventListenerDelegate& eventDelegate, const EventType& type) = 0;

	virtual bool VRemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type) = 0;

	/* trigger an Event NOW*/
	virtual bool VTriggerEvent(const IEventDataPtr& pEvent) = 0;

	/* queuing an Event */
	virtual bool VQueueEvent(const IEventDataPtr& pEvent) = 0;

	/* Find and remove next event in the queue until is it actually being processed*/
	/* allOfType to true removes all events of that type in the queue */
	virtual bool VAbortEvent(const EventType& pEventType, bool allOfType = 0) = 0;

	/*
	* Allow for processing of any queued messages
	* maxMillis : specify a timeout to process events 
	* (In this case all events maybe not processed until next round)
	*/
	virtual bool VUpdate(unsigned long maxMillis = kINFINITE) = 0;

	/*
	* Getter for the event manager
	*/
	static IEventManager* Get(void);

};

