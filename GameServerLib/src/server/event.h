#pragma once

#include "../misc/GameCodeStd.h"
#include "template.h"
#include "strstream"
#include "memory"

//---------------------------------------------------------------------------------------------------------------------
// Macro for event registration
//---------------------------------------------------------------------------------------------------------------------
class IEventData;

typedef unsigned long EventType;
typedef std::shared_ptr<IEventData> IEventDataPtr;

extern GenericObjectFactory<IEventData, EventType> g_eventFactory;
#define REGISTER_EVENT(eventClass) g_eventFactory.Register<eventClass>(eventClass::sk_EventType)
#define CREATE_EVENT(eventType) g_eventFactory.Create(eventType)

class IEventData{
public:
	virtual const EventType& VGetEventType(void) const = 0;
	virtual float VGetTimeStamp(void) const = 0;
	virtual void VSerialize(std::ostrstream& out) const = 0;
	virtual void VDeserialize(std::istrstream& in) = 0;

	virtual IEventDataPtr VCopy(void) const = 0;
	virtual const char* GetName(void) const = 0;
	virtual void VSetIp(const int ip) = 0;
	virtual void VSetSocketId(const int socketId) = 0;
	virtual const int VGetIp(void) const = 0;
	virtual const int VGetSocketId() const = 0;
};

class BaseEventData : public IEventData{
protected:
	const float m_timestamp;
	int m_ip;
	int m_sockId;
public:
	explicit BaseEventData(const float timeStamp = 0.0f) : m_timestamp(timeStamp){}
	virtual ~BaseEventData(void){}
	virtual const EventType& VGetEventType(void) const = 0;

	float VGetTimeStamp(void) const { return m_timestamp; }
	virtual void VSerialize(std::ostrstream& out) const{}
	virtual void VDeserialize(std::istrstream& in) {}

	virtual void VSetIp(const int ip) {  m_ip = ip; }
	const int VGetIp() const { return m_ip; }

	virtual void VSetSocketId(const int sockId) { m_sockId = sockId; }
	const int VGetSocketId() const { return m_sockId; }

};

class EventData_MoveActor : public BaseEventData{
	Vec3 m_position;
	int m_actorId;
	std::string m_actorName;
public:
	static const EventType sk_EventType;

	EventData_MoveActor(void)
	{
		m_position.x = m_position.y = m_position.z = 0;
		m_actorId = -1;
	}

	explicit EventData_MoveActor(int actorId, std::string name, Vec3 position) : m_actorId(actorId), m_actorName(name), m_position(position) {
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual void VSerialize(std::ostrstream& out) const{
		out << m_actorId << " "<< m_actorName << " " << m_position.x <<" " << m_position.y << " " << m_position.z ;
	}

	virtual void VDeserialize(std::istrstream& in) {
		in >> m_actorId;
		in >> m_actorName;
		in >> m_position.x;
		in >> m_position.y;
		in >> m_position.z;
	}

	virtual const char* GetName(void) const{
		return "EventData_MoveActor";
	}

	virtual IEventDataPtr VCopy(void) const{
		return IEventDataPtr(GCC_NEW EventData_MoveActor(m_actorId, m_actorName, m_position));
	}

	virtual Vec3& VGetPosition() { return m_position; }

	virtual int VGetActorId() { return m_actorId; }
	
	virtual std::string VGetActorName() { return m_actorName; }

};

class EventData_EndActor : public BaseEventData{
	int m_actorId;

public:
	static const EventType sk_EventType;

	EventData_EndActor(void)
	{
		m_actorId = -1;
	}

	explicit EventData_EndActor(int actorId) : m_actorId(actorId){
		//
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual void VSerialize(std::ostrstream& out) const{
		out << m_actorId;
	}

	virtual void VDeserialize(std::istrstream& in) {
		in >> m_actorId;
	}

	virtual const char* GetName(void) const{
		return "EventData_EndActor";
	}

	virtual IEventDataPtr VCopy(void) const{
		return IEventDataPtr(GCC_NEW EventData_EndActor(m_actorId));
	}

	int GetActorId() { return m_actorId; }

};


class EventData_ScoreActor : public BaseEventData{
	int m_actorId;
	int m_score;

public:
	static const EventType sk_EventType;

	EventData_ScoreActor(void)
	{
		m_actorId = -1;
	}

	explicit EventData_ScoreActor(int score, int actorId) : m_score(score){
		//
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual void VSerialize(std::ostrstream& out) const{
		out << m_score << " " << m_actorId;
	}

	virtual void VDeserialize(std::istrstream& in) {
		in >> m_score;
		in >> m_actorId;
	}

	virtual const char* GetName(void) const{
		return "EventData_ScoreActor";
	}

	virtual IEventDataPtr VCopy(void) const{
		return IEventDataPtr(GCC_NEW EventData_ScoreActor(m_score, m_actorId));
	}
};

class EventData_CreateActor : public EventData_MoveActor {
	friend EventData_MoveActor;
public:
	static const EventType sk_EventType;

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual const char* GetName(void) const {
		return "EventData_CreateActor";
	}
};

class EventData_GetActor : public BaseEventData {
protected:
	int m_actorId;
	std::string m_ActorName;
public:
	static const EventType sk_EventType;

	EventData_GetActor(void)
	{
		m_actorId = -1;
	}

	explicit EventData_GetActor(int actorId, std::string actorName) : m_actorId(actorId), m_ActorName(actorName) {
		//
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual void VSerialize(std::ostrstream& out) const {
		out << m_actorId << " " << m_ActorName;
	}

	virtual void VDeserialize(std::istrstream& in) {
		in >> m_actorId; 
		in >> m_ActorName;	
	}

	virtual const char* GetName(void) const {
		return "EventData_GetActor";
	}

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_GetActor(m_actorId, m_ActorName));
	}

	virtual unsigned int VGetActorId() {
		return m_actorId;
	}

	virtual std::string VGetActorName() {
		return m_ActorName;
	}

};

class EventData_GetNewPositionActor : public EventData_GetActor {
	friend EventData_GetActor;
public:
	static const EventType sk_EventType;

	EventData_GetNewPositionActor(void) {}

	explicit EventData_GetNewPositionActor(int actorId, std::string actorName) : EventData_GetActor(actorId,actorName) {
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual const char* GetName(void) const {
		return "EventData_GetNewPositionActor";
	}

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_GetNewPositionActor(m_actorId, m_ActorName));
	}
};

class EventData_ResponseHTTP : public BaseEventData {
protected:
	http_response_code_t m_httpResponsCode;
	std::string m_payload;
	std::string m_httpMessage;
public:
	static const EventType sk_EventType;

	EventData_ResponseHTTP(void) {
		this->m_httpResponsCode = http_response_code_t::NOTFOUND;
	}

	explicit EventData_ResponseHTTP(http_response_code_t httpResponsCode, std::string payload):m_httpResponsCode(httpResponsCode),m_payload(payload){
		BuildMessage();
	}

	virtual const char* GetName(void) const {
		return "EventData_ResponseHTTP";
	}

	virtual void VDeserialize(std::istrstream& in) {
		int reponseCodeTemp;
		in >> reponseCodeTemp;
		// !!! for this code sample no test if responseCodeTemp do not exist in enum
		m_httpResponsCode = (http_response_code_t)reponseCodeTemp;
		in >> m_payload;
		BuildMessage();
	}

	virtual void VSerialize(std::ostrstream& out) const {
		out << m_httpMessage.c_str();
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_ResponseHTTP(m_httpResponsCode, m_payload));
	}

	void BuildMessage();
};


class EventData_CloseSocketHTTP : public BaseEventData {
	friend BaseEventData;
public:
	static const EventType sk_EventType;

	EventData_CloseSocketHTTP(void) {}

	explicit EventData_CloseSocketHTTP(int socketId) {	
		m_sockId = socketId;
	}

	virtual const char* GetName(void) const {
		return "EventData_CloseSocketHTTP";
	}

	virtual void VDeserialize(std::istrstream& in) {
		in >> m_sockId;
	}

	virtual void VSerialize(std::ostrstream& out) const {
		out << m_sockId;
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_CloseSocketHTTP(m_sockId));
	}


};

class EventData_CreateSession : public BaseEventData {
	friend BaseEventData;
public:
	static const EventType sk_EventType;

	virtual const char* GetName(void) const {
		return "EventData_CreateSession";
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_CreateSession());
	}

};

class EventData_AddPlayer : public BaseEventData {
	friend BaseEventData;
public:
	static const EventType sk_EventType;
	
	virtual const char* GetName(void) const {
		return "EventData_AddPlayer";
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_AddPlayer());
	}
};

class EventData_EndSession : public BaseEventData {
	friend BaseEventData;
protected:
	int m_sessionId;

public:
	static const EventType sk_EventType;

	virtual const int VGetSessionId() { return m_sessionId; }

	virtual void VDeserialize(std::istrstream& in) {
		in >> m_sessionId;
	}

	virtual void VSerialize(std::ostrstream& out) const {
		out << m_sessionId;
	}

	virtual const char* GetName(void) const {
		return "EventData_EndSession";
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_EndSession());
	}
};

class EventData_SessionActive : public BaseEventData {
	friend BaseEventData;
public:
	static const EventType sk_EventType;

	virtual const char* GetName(void) const {
		return "EventData_SessionActive";
	}

	virtual const EventType& VGetEventType(void) const { return sk_EventType; }

	virtual IEventDataPtr VCopy(void) const {
		return IEventDataPtr(GCC_NEW EventData_SessionActive());
	}
};