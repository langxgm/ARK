#include "SDK/Interface/AFPlatform.h"

#ifdef NF_DEBUG_MODE

#if NF_PLATFORM == NF_PLATFORM_WIN
#pragma comment( lib, "ws2_32" )
//#pragma comment( lib, "RakNet_LibStatic_Debug_x64.lib" )
#pragma comment( lib, "NFNet_d.lib" )
#pragma comment( lib, "AFCore_d.lib" )
#pragma comment( lib, "NFMessageDefine_d.lib" )
#pragma comment( lib, "libprotobuf_d.lib" )
#pragma comment( lib, "libevent_core.lib" )

#elif NF_PLATFORM == NF_PLATFORM_LINUX || NF_PLATFORM == NF_PLATFORM_ANDROID
//#pragma comment( lib, "RakNet_LibStatic_Debug_x64.a" )
#pragma comment( lib, "NFNet_d.a" )
#pragma comment( lib, "AFCore_d.a" )
#pragma comment( lib, "NFMessageDefine_d.a" )
#elif NF_PLATFORM == NF_PLATFORM_APPLE || NF_PLATFORM == NF_PLATFORM_APPLE_IOS
#endif

#else

#if NF_PLATFORM == NF_PLATFORM_WIN
#pragma comment( lib, "ws2_32" )
#pragma comment( lib, "NFNet.lib" )
#pragma comment( lib, "AFCore.lib" )
#pragma comment( lib, "NFMessageDefine.lib" )
#pragma comment( lib, "libprotobuf.lib" )
#pragma comment( lib, "libevent_core.lib" )

#elif NF_PLATFORM == NF_PLATFORM_LINUX || NF_PLATFORM == NF_PLATFORM_ANDROID
#pragma comment( lib, "NFNet.a" )
#pragma comment( lib, "AFCore.a" )
#pragma comment( lib, "NFMessageDefine.a" )
#elif NF_PLATFORM == NF_PLATFORM_APPLE || NF_PLATFORM == NF_PLATFORM_APPLE_IOS
#endif

#endif
