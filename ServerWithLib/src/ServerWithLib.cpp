#include "stdafx.h"
#include <server.h>
#include "EventManager.h"
#include "SessionManager.h"
#include "../actors/Actors.h"
#include <iostream>
#include <vector>
#include <conio.h>
using namespace std;

void testmessage(char* message){

	HTTPResponseBuilder::GetSingleton().CreateHttpMessage();
	HTTPResponseBuilder::GetSingleton().SetMessageBody(message);
	HTTPResponseBuilder::GetSingleton().SetMessageResponse(http_response_code_t::OK);
	HTTPResponseBuilder::GetSingleton().BuildHttpMessage();

	const char* httpMessage = HTTPResponseBuilder::GetSingleton().GetHttpMessage();
	printf("httpMessage:%s\n", httpMessage);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Or _cputs("Welcome");
	cout << "Welcome" << endl;
	
	// init singleton too so first to instantiate
	ActorManager *actorManager = GCC_NEW ActorManager();

	BaseSocketManager *m_pBaseSocketManager = GCC_NEW BaseSocketManager();
	m_pBaseSocketManager->Init();
	GameServerListenSocket *server = GCC_NEW GameServerListenSocket(2222);
	server->VRegisterNetworkEvents();

	m_pBaseSocketManager->AddSocket(server);

	// creation of singleton with 2nd parameter
	EventManager *eventManager = GCC_NEW EventManager("EventManager", true);

	RemoteNetworkView * rnView = GCC_NEW RemoteNetworkView();
	rnView->SetActorManager(actorManager);//TODO using the singleton g_ActorManager
	rnView->VOnInit();

	SessionManager* sessionManager = GCC_NEW SessionManager();
	sessionManager->VOnInit();

	bool listen = true;
	while (listen){
		eventManager->VUpdate(1);// max 1 sec then destroy
		m_pBaseSocketManager->DoSelect(10);		
		if (_kbhit())
		{
			//_getch read last char input and remove it from the input buffer so _kbhit is reset
			int c  = _getch();
			if (c == 'a') _cputs("TODO Display all actors position\n");
			else if (c == 'x') break;
		}
	}
	m_pBaseSocketManager->Shutdown();
	delete(rnView);
	delete(eventManager);
	delete(actorManager);
	delete(m_pBaseSocketManager);
	delete(server);

	return 0;
}

