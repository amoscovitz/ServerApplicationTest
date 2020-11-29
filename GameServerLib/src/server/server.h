#pragma once

#ifdef _CONSOLEAPP
  #include <Winsock2.h>
  #include <ws2tcpip.h>
#endif
#include <crtdbg.h>
#include <assert.h>
#include <memory>
#include <list>
#include <map>
#include <strstream>
#include "../misc/GameCodeStd.h"
#include "../actors/Actors.h"

class IPacket{
	public:
    virtual char const * const VGetType() const = 0;
	virtual char const *const VGetData() const = 0;
	virtual u_long VGetSize() const = 0;
	virtual ~IPacket(){}
};

class BinaryPacket : public IPacket{
protected:
	char *m_data;
public:
	// inline will insert the piece of code in each function call, do it for small and often used piece of code
	inline BinaryPacket(char const *const data, u_long size);
	inline BinaryPacket(u_long size);
	virtual ~BinaryPacket(){ SAFE_DELETE(m_data); }
	virtual char const *const VGetType() const { return g_Type; }
	// cannot modify pointer and value
	virtual char const *const VGetData() const { return m_data; }
	virtual u_long VGetSize() const { return ntohl(*(u_long *)m_data); }
	inline void MemCpy(char const * const data, size_t size, int destOffset);

	static const char *g_Type;
};


class TextPacket : public BinaryPacket{
public:
	TextPacket(char const * const text);
	virtual char const *const VGetType() const { return g_Type; }
	static const char *g_Type;
};

class HTTPPacket : public BinaryPacket{
public:
	HTTPPacket(char const * const text);
	virtual char const * const VGetType() const { return g_Type; }
	virtual char const *const VGetData() const { return m_data+4; }
	virtual u_long VGetSize() const { return strlen(m_data+4); }
	static const char *g_Type;
};


inline BinaryPacket::BinaryPacket(char const *const data, u_long size){
	m_data = GCC_NEW char[size + sizeof(u_long)];
	assert(m_data);	
	*(u_long*)m_data = htonl(size + sizeof(u_long));
	memcpy(m_data + sizeof(u_long), data, size);
}

inline BinaryPacket::BinaryPacket(u_long size){
	m_data = GCC_NEW char[size + sizeof(u_long)];
	assert(m_data);
	memset(m_data + sizeof(u_long), '\0', size);
	*(u_long*)m_data = htonl(size + sizeof(u_long));
}

inline void BinaryPacket::MemCpy(char const * const data, size_t size, int destOffset){
	assert(size + destOffset <= VGetSize()-sizeof(u_long));
	memcpy(m_data + destOffset+ sizeof(u_long), data, size);
}

#define MAX_PACKET_SIZE (256)
#define RECV_BUFFER_SIZE (MAX_PACKET_SIZE*512)

// Common HTTP attributes
class HTTPMessage{
public:
	/*enum request_method{
		RM_GET,
		RM_POST,
		RM_PUT,
		RM_DELETE,
		RM_HEAD,
		RM_TRACE,
		RM_CONNECT,
		RM_OPTIONS
	};

	enum reponse_code_t{
		OK = 200,
		CREATED = 201,
		BADREQUEST = 400,
		NOTFOUND = 404,
		SERVERNOTAVAILABLE = 500
	};*/

	static const char* m_head;
	static const char* m_dateOrigin;
	static const char* m_serverName;
	static const char* m_lastModified;

	static const char* m_contentType;
	static const char* m_contentLength;
	static const char* m_acceptRanges;
	static const char* m_connection;
	static const char* m_accessControlAllowOrigin;

	HTTPMessage(){};
	virtual ~HTTPMessage() { SAFE_DELETE(_httpMessageBody); SAFE_DELETE(_httpMessage); }

	virtual const char* GetHttpMessageBody(){ return _httpMessageBody; }

	virtual void SetHttpData(const char* data){ _httpMessageBody = data; }

	virtual void SetHttpResponseCode(http_response_code_t responseCode){ _responseCode = responseCode; }

	virtual http_response_code_t getResponseCode(){ return _responseCode; }

	virtual char* GetHttpMessage(){ return _httpMessage; }

	virtual void SetHttpMessage(const char* message);


protected:
	const char* _httpMessageBody;
	http_response_code_t _responseCode;
	char* _httpMessage;
};



class NetSocket{
	friend class BaseSocketManager;
	typedef std::list < std::shared_ptr<IPacket>> PacketList;

public:
	NetSocket();
	NetSocket(SOCKET new_sock, unsigned int hostIP);
	virtual ~NetSocket();

	bool Connect(unsigned int ip, unsigned int port, bool forceCoalesce = 0);
	void SetBlocking(bool blocking);
	
	// Create an HTTPNetSocket class to move those functions, they do not belong here
	void SendHttpResponse(http_response_code_t reponse_code, char* response_body = 0);
	bool IsHttpRequest(const char* message);
	//
	void Send(std::shared_ptr<IPacket> pkt, bool clearTimeout = 1);
	virtual int VHasOutput(){ return !m_OutList.empty(); }
	virtual void VHandleOutput();
	virtual void VHandleInput();
	virtual void VTimeOut(){ m_timeout = 0; }
	void HandleException(){ m_delete_flag |= 1; }
	void SetTimeOut(unsigned int ms = 45 * 1000){ m_timeout = timeGetTime() + ms; }
	int GetIpAddress(){ return m_ipaddr; }
	int GetSockId() { return m_id; }
	void SetSocketDelete(int flag) { m_delete_flag = flag; }
	
protected:
	SOCKET m_sock;
	int m_id;

	int m_delete_flag;

	PacketList m_OutList;
	PacketList m_InList;
	unsigned int m_timeout;
	unsigned int m_ipaddr;
	bool m_bBinaryProtocol;

	char m_recvBuf[RECV_BUFFER_SIZE];
	char m_responseBuf[RECV_BUFFER_SIZE];
	unsigned int m_recvOfs, m_recvBegin;

	int m_sendOfs;
	int m_internal;
	int m_timeCreated;
};


class BaseSocketManager{
protected:
	WSADATA m_WsaData; // socket system impl.

	typedef std::list<NetSocket *> SocketList;
	typedef std::map<int, NetSocket *> SocketIdMap;

	SocketList m_SockList;
	SocketIdMap m_SockMap;

	int m_NextSocketId;

	unsigned int m_outBound;
	unsigned int m_inBound;
	unsigned int m_MaxOpenSockets;

	unsigned int m_Subnet;
	unsigned int m_SubnetMask;


public:
	BaseSocketManager();
	virtual ~BaseSocketManager(){ Shutdown(); }
	bool Init();
	void Shutdown();
	int AddSocket(NetSocket *socket);
	void RemoveSocket(NetSocket *socket);

	bool Send(int sockId, std::shared_ptr<IPacket> packet);
	void DoSelect(int pauseMicroSecs, int handleInput = 1);

	int GetIpAddress(int sockId);

	void SetSubNet(unsigned int subnet, unsigned int subnetMask){
		m_Subnet = subnet,
		m_SubnetMask = subnetMask;
	}

	bool IsInternal(unsigned int ipaddr); 

	unsigned int GetHostByName(const std::string &hostname);
	const char *GetHostByAddr(unsigned int ip);

	void AddToOutBound(int rc){ m_outBound += rc; }
	void AddToInBound(int rc){ m_inBound += rc; }
	
	NetSocket *FindSocket(int sockId);

};

// declare g_pSocketManager but not init, done in constructor of BaseSocketManager
extern BaseSocketManager *g_pSocketManager;

//client
class ClientSocketManager :public BaseSocketManager{
	std::string m_hostname;
	unsigned int m_port;

public:
	ClientSocketManager(const std::string &hostname, unsigned int port){
		m_hostname = hostname;
		m_port = port;
	}
	int Connect();
};


class NetListenSocket :public NetSocket{
public: 
	NetListenSocket(){};
	NetListenSocket(int portnum){ port = 0; Init(portnum); }

	void Init(int portnum);
	SOCKET AcceptConnection(unsigned int*pAddr);

	unsigned int port;
};


//server
class GameServerListenSocket :public NetListenSocket{
public:
	GameServerListenSocket(int portnum){ Init(portnum); }
	void VRegisterNetworkEvents(void);
	void VHandleInput();
};

//remote event
class RemoteEventSocket :public NetSocket{
public:
	enum{
		NetMsg_Event,
		NetMsg_PlayerLoginOk
	};

	// server accepting a client
	RemoteEventSocket(SOCKET new_sock, unsigned int hostIP) : NetSocket(new_sock, hostIP){}

	//client attach to server
	RemoteEventSocket(){};
	virtual void VHandleInput();
};

// Build a full response message from input parameters

class HTTPMessageResponse : public HTTPMessage{
public:

	HTTPMessageResponse(){};
	~HTTPMessageResponse(){};
};


class HTTPMessageBuilder{
public:

	virtual void CreateHttpMessage() =0;

	virtual void SetMessageBody(const char* data) = 0;

	virtual void SetMessageResponse(http_response_code_t messageReponse) = 0;

	virtual void BuildHttpMessage() = 0;

	virtual char const* const GetHttpMessage() = 0;
protected:

	HTTPMessageBuilder(){};
	~HTTPMessageBuilder(){};

};

class HTTPResponseBuilder : public HTTPMessageBuilder{
public:
	HTTPResponseBuilder(){};
	~HTTPResponseBuilder(){ SAFE_DELETE(_httpResult); }

	static HTTPResponseBuilder& GetSingleton();

	void CreateHttpMessage();

	void SetMessageBody(const char* data);

	void SetMessageResponse(http_response_code_t messageReponse);

	void BuildHttpMessage();

	void DeleteHttpMessage(){ SAFE_DELETE(_httpResult); }

	char const* const GetHttpMessage() { return _httpResult->GetHttpMessage(); }

private:
	HTTPMessageResponse* _httpResult;
};