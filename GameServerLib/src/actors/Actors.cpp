#include "Actors.h"
#include <iostream>
#include "../events/IEventManager.h"
#include "../server/server.h"
#include "../ThirdPart/FastDelegate.h"


ActorManager::ActorManager() {
	m_lastId = -1; g_pActorManager = this;
};


ActorId ActorManager::AddActor(StrongActorPtr pActor) {
	ActorId id;
	// if an actor already exists, do not create
	StrongActorPtr actorTest = GetActorByName(pActor->GetIp(),pActor->GetName());
	if (actorTest == NULL) {
		id = pActor->GetId();
		// No id affected yet
		if (id == -1) {
			id = GetNextLastId();
			pActor->SetId(id);
			m_ActorMap[id] = pActor;
			//alternative
			//m_ActorMap.insert(std::pair<ActorId, Actor>(id, actor));
			// add the actor for this Ip
			m_ActorsByIpMap[pActor->GetIp()].push_back(pActor);
		}
	}
	else {
		id = actorTest->GetId();
	}
	return id;
}

void ActorManager::RemoveActor(ActorId id) {
	m_ActorMap.erase(id);
	//
	StrongActorPtr actor = GetActor(id);
	std::list<StrongActorPtr> actorList = m_ActorsByIpMap[actor->GetIp()];
	for (std::list<StrongActorPtr>::iterator i = actorList.begin(); i != actorList.end(); ++i) {
		StrongActorPtr pActor = *i;
		if (pActor->GetName().compare(actor->GetName()) == 0) {
			actorList.erase(i);			
			//we break the iterator but we leave ah ah.
			break;
		}
	}
}

StrongActorPtr ActorManager::GetActor(ActorId id) {
	ActorsMap::iterator i = m_ActorMap.find(id);
	if (i != m_ActorMap.end()) {
		return (*i).second;
	}
	return NULL;
}

const StrongActorPtr ActorManager::GetActorByName(unsigned int ip, std::string actorName) {
	std::list<StrongActorPtr> actorList = m_ActorsByIpMap[ip];
//
	for (std::list<StrongActorPtr>::iterator i = actorList.begin(); i != actorList.end(); ++i) {
		StrongActorPtr pActor = *i;
		if (pActor->GetName().compare(actorName)==0) {
			return pActor;
		}
	}
	return NULL;
}

const Vec3* ActorManager::GetActorPosition(ActorId id) {
	ActorsMap::iterator i = m_ActorMap.find(id);
	if (i != m_ActorMap.end()) {
		StrongActorPtr actor = ((*i).second);
		return &(actor->GetPosition());// Warning here return local or temp address
	}
	return NULL;
}

void RemoteNetworkView::CreateActor(IEventDataPtr pEventData) {
	std::shared_ptr<EventData_CreateActor> pCastEventData = std::static_pointer_cast<EventData_CreateActor>(pEventData);
	ActorId id = pCastEventData->VGetActorId();
	Vec3 position = pCastEventData->VGetPosition();
	Actor* actor = GCC_NEW Actor(id, position);
	actor->SetName(pCastEventData->VGetActorName());
	// With that Unity can safely call an Actor Name without messing up if there is simultaneous connection
	actor->SetIp(pCastEventData->VGetIp());
	std::shared_ptr<Actor> pActor(actor);
	m_ActorManager->AddActor(pActor);
}


void RemoteNetworkView::GenerateNewPositionActor(IEventDataPtr pEventData) {
	char response[4];
	std::string httpinmsg;
	IEventDataPtr pResponseHttpEvent(CREATE_EVENT(EventData_ResponseHTTP::sk_EventType));

	std::shared_ptr<EventData_GetNewPositionActor> pCastEventData = std::static_pointer_cast<EventData_GetNewPositionActor>(pEventData);

	std::string name = pCastEventData->VGetActorName();
	unsigned int ip = pCastEventData->VGetIp();

	//look for actor data in actor manager by ip / actor name
	StrongActorPtr pActor = m_ActorManager->GetActorByName(ip, name);

	if (pActor == NULL) {
		_itoa_s((int)http_response_code_t::NOTFOUND, response, _countof(response), 10);
		httpinmsg.append(response);
		std::istrstream in(httpinmsg.c_str(), httpinmsg.size());
		pResponseHttpEvent->VDeserialize(in);
		IEventManager::Get()->VTriggerEvent(pResponseHttpEvent);
		return;
	}

	// calculate new coord random (great AI !)
	int random = rand();
	Vec3 newPosition;
	newPosition.x = random % 512;
	newPosition.y = random % 480;
	newPosition.z = 0;

	pActor->SetPosition(newPosition);
	// send back all data	
	GetActor(pEventData);
}

void RemoteNetworkView::UpdatePositionActor(IEventDataPtr pEventData) {
	std::shared_ptr<EventData_MoveActor> pCastEventData = std::static_pointer_cast<EventData_MoveActor>(pEventData);
	std::string name = pCastEventData->VGetActorName();
	unsigned int ip = pCastEventData->VGetIp();
	Vec3 newPosition = pCastEventData->VGetPosition();
	//look for actor data in actor manager by ip / actor name
	StrongActorPtr pActor = m_ActorManager->GetActorByName(ip, name);
	if (pActor != NULL) {
		pActor->SetPosition(newPosition);
	}
}

void RemoteNetworkView::GetActor(IEventDataPtr pEventData) {
	char response[4];
	std::string httpinmsg;
	IEventDataPtr pResponseHttpEvent(CREATE_EVENT(EventData_ResponseHTTP::sk_EventType));

	std::shared_ptr<EventData_GetActor> pCastEventData = std::static_pointer_cast<EventData_GetActor>(pEventData);
	std::string name = pCastEventData->VGetActorName();
	unsigned int ip = pCastEventData->VGetIp();
	pResponseHttpEvent->VSetSocketId(pEventData->VGetSocketId());
	pResponseHttpEvent->VSetIp(ip);
	//look for actor data in actor manager by ip / actor name
	StrongActorPtr pActor = m_ActorManager->GetActorByName(ip, name);

	if (pActor == NULL) {
		_itoa_s((int)http_response_code_t::NOTFOUND, response, _countof(response), 10);
		httpinmsg.append(response);
		std::istrstream in(httpinmsg.c_str(),httpinmsg.size());
		pResponseHttpEvent->VDeserialize(in);
		IEventManager::Get()->VTriggerEvent(pResponseHttpEvent);
	}
	else {
		// Better (de-)serialize Actor with streams
		std::string buffer;
		_itoa_s((int)http_response_code_t::OK, response, _countof(response), 10);
		buffer.append(response);
		buffer.append(" ");

		char* id = new char[10];
		sprintf_s(id, 10, "%d", pActor->GetId());
		buffer.append(id);
		buffer.append("#");
		buffer.append(pActor->GetName());
		buffer.append("#");
		Vec3 position = pActor->GetPosition();
		char *xpos = new char[10];
		sprintf_s(xpos, 10, "%.3f", position.x);
		buffer.append(xpos);
		buffer.append("#");
		char *ypos = new char[10];
		sprintf_s(ypos, 10, "%.3f", position.y);
		buffer.append(ypos);
		buffer.append("#");
		char *zpos = new char[10];
		sprintf_s(zpos, 10, "%.3f", position.z);
		buffer.append(zpos);	
		buffer.push_back('\0');
		// when found, send answer	
		std::istrstream in(buffer.c_str(), buffer.size());
		pResponseHttpEvent->VDeserialize(in);
		IEventManager::Get()->VTriggerEvent(pResponseHttpEvent);
	}
}


void RemoteNetworkView::RemoveActor(IEventDataPtr pEventData) {
	std::shared_ptr<EventData_EndActor> pCastEventData = std::static_pointer_cast<EventData_EndActor>(pEventData);
	int actorId = pCastEventData->GetActorId();
	m_ActorManager->RemoveActor(actorId);

}

void RemoteNetworkView::VOnInit() {
	// register to listener to events getActor / addActor / updateActor / removeActor 
	IEventManager::Get()->VAddListener( 
		MakeDelegate(this,&RemoteNetworkView::CreateActor), 
		EventData_CreateActor::sk_EventType);

	IEventManager::Get()->VAddListener(
		MakeDelegate(this, &RemoteNetworkView::GetActor),
		EventData_GetActor::sk_EventType);

	IEventManager::Get()->VAddListener(
		MakeDelegate(this, &RemoteNetworkView::RemoveActor),
		EventData_EndActor::sk_EventType);

	IEventManager::Get()->VAddListener(
		MakeDelegate(this, &RemoteNetworkView::GenerateNewPositionActor),
		EventData_GetNewPositionActor::sk_EventType);

	IEventManager::Get()->VAddListener(
		MakeDelegate(this, &RemoteNetworkView::UpdatePositionActor),
		EventData_MoveActor::sk_EventType);

	IEventManager::Get()->VAddListener(
		MakeDelegate(this, &RemoteNetworkView::ForwardEvent),
		EventData_ResponseHTTP::sk_EventType);

	IEventManager::Get()->VAddListener(
		MakeDelegate(this, &RemoteNetworkView::RemoveHTTPSocket),
		EventData_CloseSocketHTTP::sk_EventType);

}

void RemoteNetworkView::VOnUpdate() {

}

void RemoteNetworkView::ForwardEvent(IEventDataPtr pEventData) {
	std::string httpinmsg;
	if (!pEventData) {
		GCC_ERROR("Event Invalid");
		return;
	}
	int socketId = pEventData->VGetSocketId();

	std::ostrstream out;

	pEventData->VSerialize(out);
	out << std::ends;

	IPacket *packetBack = NULL;
	if (!strncmp("HTTP", &out.rdbuf()->str()[0], 4)) {
		packetBack = GCC_NEW HTTPPacket(out.rdbuf()->str());
	}
	else {
		//TODO create binary packet
	}
	std::shared_ptr<IPacket> ipBack(packetBack);
	printf("RemoteNetworkView Forward Event socket:%d %s\n", socketId, pEventData->GetName());
	g_pSocketManager->Send(socketId, ipBack);

	if (!strncmp("HTTP", &out.rdbuf()->str()[0], 4)) {
		IEventDataPtr pCloseSocketHttpEvent(CREATE_EVENT(EventData_CloseSocketHTTP::sk_EventType));
		char* id = new char[100];
		sprintf_s(id, 100, "%d", socketId);
		httpinmsg.append(id);
		std::istrstream in(httpinmsg.c_str(), httpinmsg.size());
		pCloseSocketHttpEvent->VDeserialize(in);
		IEventManager::Get()->VQueueEvent(pCloseSocketHttpEvent);
	}
}

void RemoteNetworkView::RemoveHTTPSocket(IEventDataPtr pEventData) {
	printf("Remove HTTP Socket %d\n", pEventData->VGetSocketId());
	NetSocket* netSocket = g_pSocketManager->FindSocket(pEventData->VGetSocketId());
	// say to delete socket AFTER packet treatment
	if (netSocket) {
		netSocket->SetSocketDelete(4);
	}
}