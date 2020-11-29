#include "..\server\stdafx.h"
#include "IEventManager.h"
#include <assert.h>

static IEventManager* g_pEventMgr = NULL;

//static from book
//IEventManager* Get(void){
//	assert(g_pEventMgr);
//	return g_pEventMgr;
//}

// from cpp fix auto
IEventManager * IEventManager::Get(void)
{
	assert(g_pEventMgr);
	return g_pEventMgr;
}



IEventManager::IEventManager(const char* pName, bool setAsGlobal){
	if (setAsGlobal){
		if (g_pEventMgr){
			//TODO log :override existing EventManager
			delete g_pEventMgr;
		}
		g_pEventMgr = this;
	}
}

IEventManager::~IEventManager(){
	if (g_pEventMgr == this){
		g_pEventMgr = NULL;
	}
}
