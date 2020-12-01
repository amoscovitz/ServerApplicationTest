#pragma once
#include <map>
#include <list>
#include "../misc/GameCodeStd.h"
#include "../server/event.h"

class Actor;

typedef std::shared_ptr<Actor> StrongActorPtr;

class BaseActor {
protected:
	ActorId m_id;
	std::string m_name;
	Vec3 m_position;
	// associated computer ip for this actor
	unsigned int m_ip;

public:
	const Vec3 GetPosition(void) const { return m_position; }
	const ActorId GetId(void) const { return m_id; }
	void SetId(ActorId id) { m_id = id; }
	void SetName(std::string name) { m_name = name; }
	std::string GetName() { return m_name; }
	void SetIp(unsigned int ip) { m_ip = ip; }
	unsigned int GetIp(void) { return m_ip; }
	void SetPosition(const Vec3 vec3) { m_position.x = vec3.x, m_position.y = vec3.y; m_position.z = vec3.z; }
};

class Actor : public BaseActor{	
public:
	explicit Actor(ActorId id, Vec3 vec3) { m_position.x = vec3.x, m_position.y = vec3.y; m_position.z = vec3.z; m_id = id; }
};


// events received as pointer
// create actor from rawdata
class ActorManager {

protected:
	// Map Id, Actor
	typedef std::map<ActorId, StrongActorPtr> ActorsMap;
	ActorsMap m_ActorMap;	

	// Map of Ip and actors associated so that Unity can get the right position for the right machine	
	typedef std::map<unsigned int, std::list<StrongActorPtr>> ActorsByIpMap;
	ActorsByIpMap m_ActorsByIpMap;

	int m_lastId;
public:
	ActorManager();

	~ActorManager() {};

	ActorId AddActor(StrongActorPtr actor);

	void RemoveActor(ActorId id);

	// getActor(id)
	StrongActorPtr GetActor(ActorId id);

	// getActorPostition(id)
	const Vec3* GetActorPosition(ActorId id);

	// loop on all actors to get the position for the right couple "ip" / "name"
	// or use todo map "ActorsByIpMap" then loop on reduced list of actors
	const StrongActorPtr GetActorByName(unsigned int ip, std::string actorName);

	ActorId GetLastId() { return m_lastId; }

	ActorId GetNextLastId() { return ++m_lastId; }

};

extern ActorManager *g_pActorManager;


class RemoteNetworkView {
protected:
	ActorManager* m_ActorManager;
	
public:
	// createActor (am->addActor) (POST)
	// = NewActorDelegate in book
	void CreateActor(IEventDataPtr pEventData);

	// Send Actor data id name position (GET)
	void GetActor(IEventDataPtr pEventData);

	// Update position (POST)
	void UpdatePositionActor(IEventDataPtr pEventData);

	// Remove actor position (POST)
	void RemoveActor(IEventDataPtr pEventData);

	// Generate Random New Vec3 position (GET)
	void GenerateNewPositionActor(IEventDataPtr pEventData);

	void SetActorManager(ActorManager* actorManager) { m_ActorManager = actorManager; }

	// register to listener to events getActor / addActor / removeActor / getPosition
	void VOnInit();

	void VOnUpdate();

	// Send event back to caller (Sent to socket)
	void ForwardEvent(IEventDataPtr pEventData);

	void RemoveHTTPSocket(IEventDataPtr pEventData);

};