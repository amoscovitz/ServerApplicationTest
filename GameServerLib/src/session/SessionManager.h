#pragma once
#pragma once
#include <iostream>
#include <sstream>
#include "../misc/GameCodeStd.h"
#include "../server/event.h"

constexpr auto MAX_SESSION = 1;
constexpr auto MAX_PLAYER = 2;

struct Sessions {
	int session[MAX_SESSION];
};

struct PlayerBySession {
	int playerid; 
	int sessionid;
};
//64 bits 32 bits
//char =  1 byte = 8 bits => 4 char = 32 bits = 1 align
//int = 4 bytes = 32 bits
// test TODO sizeof of the struct
/*struct Player { // Actor
	char name[32];//32*8 = 256 bits
	int idplayer;
	int lastx;
	int lasty;	
};*/
class SessionManager
{
private:
	int CURRENT_SESSION = 0;
	Sessions sessions;
	PlayerBySession playersessions[MAX_PLAYER*MAX_SESSION];

	int GenerateSessionId();

	void SendHTTPResponse(http_response_code_t response_code, std::string payload, int socketid);
//protected:

	//RemoteNetworkView* m_RemoteNetworkView;

public:
	SessionManager();

	void CreateSession(IEventDataPtr pEventData);

	void DeleteSession(IEventDataPtr pEventData);

	void PrintSessions();	

	void AddPlayerToSession(int idactor, int sessionid);

	// register to listener to events getActor / addActor / removeActor / getPosition
	void VOnInit();

	void VOnUpdate();

	//void SetRemoteNetworkView(RemoteNetworkView* rnv) { m_RemoteNetworkView = rnv; };

	// Send event back to caller (Sent to socket)
	//void ForwardEvent(IEventDataPtr pEventData);

	//void RemoveHTTPSocket(IEventDataPtr pEventData);
};

extern SessionManager* g_pSessionManager;


// Actor
/*class PlayerManager {

	int AddPlayer(char* playername);

	void RemovePlayer(int playerid);

	void SetLastPlayerPosition(int playerid, int x, int y);

	player_t* GetPlayerData(int playerid);
	
};*/
