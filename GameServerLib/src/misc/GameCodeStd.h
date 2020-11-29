#pragma once

#include <string>

#include "..\ThirdPart\FastDelegate.h"
using fastdelegate::MakeDelegate;

// in  Msvc/GameCodeStd.h to put in include c++
#if !defined(SAFE_DELETE)
#define SAFE_DELETE(x) if(x) delete x; x=NULL;
#endif

#if defined(_DEBUG)
#       define GCC_NEW new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#       define GCC_NEW new
#endif


namespace Logger
{
	// This class is used by the debug macros and shouldn't be accessed externally.
	class ErrorMessenger
	{
		bool m_enabled;

	public:
		ErrorMessenger(void);
		void Show(const std::string& errorMessage, bool isFatal, const char* funcName, const char* sourceFile, unsigned int lineNum);
	};

	// construction; must be called at the beginning and end of the program
	void Init(const char* loggingConfigFilename);
	void Destroy(void);

	// logging functions
	void Log(const std::string& tag, const std::string& message, const char* funcName, const char* sourceFile, unsigned int lineNum);
	void SetDisplayFlags(const std::string& tag, unsigned char flags);
}


// do {}while if used only for brackets in macro to more than 1 statement (to be developed)
#if 1 // ndef NDEBUG
#define GCC_ASSERT(expr) \
		do \
		{ \
			if (!(expr)) \
				{ \
					static Logger::ErrorMessenger* pErrorMessenger = GCC_NEW Logger::ErrorMessenger; \
					pErrorMessenger->Show(#expr, false, __FUNCTION__, __FILE__, __LINE__); \
				} \
		} \
		while (0) \

#define GCC_ERROR(str) \
		do \
		{ \
			static Logger::ErrorMessenger* pErrorMessenger = GCC_NEW Logger::ErrorMessenger; \
			std::string s((str)); \
			pErrorMessenger->Show(s, false, __FUNCTION__, __FILE__, __LINE__); \
		} \
		while (0)\

#else  // NDEBUG is defined
#define GCC_ASSERT(expr) do { (void)sizeof(expr); } while(0) 
#endif

#define END_OF_LINE "\r\n"

typedef unsigned int ActorId;

typedef struct D3DXVECTOR3 {
	float x;
	float y;
	float z;
} D3DXVECTOR3, *LPD3DXVECTOR3;

class Vec3 : public D3DXVECTOR3 {
public:
	Vec3() :D3DXVECTOR3() {}
	Vec3(const float _x, const float _y, const float _z) { x = _x; y = _y; z = _z; }
};

enum request_method {
	RM_GET,
	RM_POST,
	RM_PUT,
	RM_DELETE,
	RM_HEAD,
	RM_TRACE,
	RM_CONNECT,
	RM_OPTIONS
};

enum class http_response_code_t :int{
	OK = 200,
	CREATED = 201,
	BADREQUEST = 400,
	NOTFOUND = 404,
	SERVERNOTAVAILABLE = 500
};
