// -------------------------------------------------------------------------
//    @FileName         :    NFMasterServerPlugin.cpp
//    @Author           :    Ark Game Tech
//    @Date             :    2012-07-14 08:51
//    @Module           :    CentreLogicModule
//
// -------------------------------------------------------------------------


#include "AFCMasterModule.h"
#include "AFMasterServerPlugin.h"

//
//
#ifdef NF_DYNAMIC_PLUGIN

ARK_EXPORT void DllStartPlugin(AFIPluginManager* pm)
{
#if ARK_PLATFORM == PLATFORM_WIN
    SetConsoleTitle("NFMasterServer");
#endif

    CREATE_PLUGIN(pm, NFMasterServerPlugin)

    //std::cout << "DllStartPlugin::thread id=" << GetCurrentThreadId() << std::endl;
};

ARK_EXPORT void DllStopPlugin(AFIPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NFMasterServerPlugin)
};

#endif
//////////////////////////////////////////////////////////////////////////

const int NFMasterServerPlugin::GetPluginVersion()
{
    return 0;
}

const std::string NFMasterServerPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFMasterServerPlugin)
}

void NFMasterServerPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, AFIMasterModule, AFCMasterModule)


}

void NFMasterServerPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, AFIMasterModule, AFCMasterModule)
}
