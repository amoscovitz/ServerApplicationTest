#include "SessionManager.h"
#include "../events/IEventManager.h"
#include "../server/server.h"
#include "../ThirdPart/FastDelegate.h"

SessionManager::SessionManager() {
	g_pSessionManager = this;
	for (int x = 0; x < MAX_SESSION; ++x) {
		sessions.session[x] = 0;
	}
	for (int x = 0; x < MAX_SESSION*MAX_PLAYER; ++x) {
		playersessions[x].sessionid = 0;
		playersessions[x].playerid = 0;
	}
}

void SessionManager::CreateSession(IEventDataPtr pEventData) {
	int sessionid = 0;
	std::string payload;
	// too much for converting single int, use itos
	std::ostringstream sstream;

	if (CURRENT_SESSION == MAX_SESSION) {
		std::cout << "max session reached" << std::endl;
	}
	else {
		for (int x = 0; x < MAX_SESSION; ++x) {
			if (sessions.session[x] == 0) {
				sessionid = sessions.session[x] = GenerateSessionId();
				break;
			}
		}
		++CURRENT_SESSION;
	}

	sstream << sessionid;
	payload.append(sstream.str());
	SendHTTPResponse(http_response_code_t::OK, payload);
}


void SessionManager::SendHTTPResponse(http_response_code_t response_code, std::string payload) {
	char response[4];
	std::string httpinmsg;
	IEventDataPtr pResponseHttpEvent(CREATE_EVENT(EventData_ResponseHTTP::sk_EventType));

	_itoa_s((int)response_code, response, _countof(response), 10);
	httpinmsg.append(response);
	httpinmsg.append(" ");
	httpinmsg.append(payload);

	std::istrstream in(httpinmsg.c_str(), httpinmsg.size());
	pResponseHttpEvent->VDeserialize(in);
	IEventManager::Get()->VTriggerEvent(pResponseHttpEvent);
}

void SessionManager::DeleteSession(int sessionid) {
	for (int x = 0; x < MAX_SESSION; ++x) {
		if (sessions.session[x] == sessionid) {
			sessions.session[x] = 0;
			--SessionManager::CURRENT_SESSION;
			break;
		}
	}
}

void SessionManager::PrintSessions() {
	char buffer[50];
	for (int x = 0; x < SessionManager::CURRENT_SESSION; ++x) {
		sprintf_s(buffer, "Sessions:%d", SessionManager::sessions.session[x]);
		std::cout << buffer << std::endl;
	}
}

int SessionManager::GenerateSessionId() {
	return 1;
}

void SessionManager::VOnInit() {
	// register to listener to events CreateSession
	IEventManager::Get()->VAddListener(	MakeDelegate(this, &SessionManager::CreateSession), EventData_CreateSession::sk_EventType);
}
