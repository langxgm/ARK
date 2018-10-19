﻿/*
* This source file is part of ArkGameFrame
* For the latest info, see https://github.com/ArkGame
*
* Copyright (c) 2013-2018 ArkGame authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "Common/AFProtoCPP.hpp"
#include "Common/AFDataDefine.hpp"
#include "AFCGameNetClientModule.h"

namespace ark
{

    bool AFCGameNetClientModule::Init()
    {
        m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
        m_pClassModule = pPluginManager->FindModule<AFIClassModule>();
        m_pConfigModule = pPluginManager->FindModule<AFIConfigModule>();
        m_pLogModule = pPluginManager->FindModule<AFILogModule>();
        m_pBusModule = pPluginManager->FindModule<AFIBusModule>();
        m_pGameNetServerModule = pPluginManager->FindModule<AFIGameNetServerModule>();
        m_pNetClientManagerModule = pPluginManager->FindModule<AFINetClientManagerModule>();
        m_pMsgModule = pPluginManager->FindModule<AFIMsgModule>();

        m_pKernelModule->AddClassCallBack(ark::Player::ThisName(), this, &AFCGameNetClientModule::OnObjectClassEvent);

        return true;
    }

    bool AFCGameNetClientModule::PreUpdate()
    {
        int ret = StartClient();
        return (ret == 0);
    }

    int AFCGameNetClientModule::StartClient()
    {
        //创建所有与对端链接的client
        int ret = m_pNetClientManagerModule->CreateClusterClients();
        if (ret != 0)
        {
            ARK_LOG_ERROR("Cannot start server net, busid = {}, error = {}", m_pBusModule->GetSelfBusName(), ret);
            ARK_ASSERT_NO_EFFECT(0);
            return ret;
        }

        //if need to add a member
        AFINetClientService* pNetClient = m_pNetClientManagerModule->GetNetClientService(ARK_APP_TYPE::ARK_APP_WORLD);
        if (pNetClient == nullptr)
        {
            return -1;
        }

        //TODO:will add a transfer callback
        //pNetClient->AddNetRecvCallback(this, &AFCGameNetClientModule::TransPBToProxy);



        //pNetClient->AddNetEventCallBack(this, &AFCGameNetClientModule::OnSocketEvent);

        return 0;
    }

    void AFCGameNetClientModule::OnSocketEvent(const NetEventType event, const AFGUID& conn_id, const std::string& ip, const int bus_id)
    {
        AFBusAddr bus_addr(bus_id);
        switch (event)
        {
        case CONNECTED:
            ARK_LOG_INFO("Connected success, conn_id = {}, bus_id = {}", conn_id.ToString(), bus_addr.ToString());
            Register(bus_id);
            break;
        case DISCONNECTED:
            ARK_LOG_ERROR("Connected failed, conn_id = {}, bus_id = {}, try to reconnect...", conn_id.ToString(), bus_addr.ToString())
            break;
        default:
            break;
        }
    }

    int AFCGameNetClientModule::OnObjectClassEvent(const AFGUID& self, const std::string& strClassName, const ARK_ENTITY_EVENT eClassEvent, const AFIDataList& var)
    {
        if (strClassName == ark::Player::ThisName())
        {
            if (ARK_ENTITY_EVENT::ENTITY_EVT_DESTROY == eClassEvent)
            {
                SendOffline(self);
            }
            else if (ARK_ENTITY_EVENT::ENTITY_EVT_ALL_FINISHED == eClassEvent)
            {
                SendOnline(self);
            }
        }

        return 0;
    }

    void AFCGameNetClientModule::Register(const int bus_id)
    {
        AFINetClientService* pNetClient = m_pNetClientManagerModule->GetNetClientServiceByBusID(bus_id);
        if (pNetClient == nullptr)
        {
            ARK_ASSERT_NO_EFFECT(0);
            return;
        }

        const AFServerConfig* server_config = m_pBusModule->GetAppServerInfo();
        if (server_config == nullptr)
        {
            ARK_ASSERT_NO_EFFECT(0);
            return;
        }

        AFMsg::msg_ss_server_report msg;

        msg.set_bus_id(server_config->self_id);
        msg.set_cur_online(0);
        msg.set_url(server_config->public_ep_.to_string());
        msg.set_max_online(server_config->max_connection);
        msg.set_logic_status(AFMsg::E_ST_NARMAL);

        m_pMsgModule->SendParticularSSMsg(bus_id, AFMsg::E_SS_MSG_ID_SERVER_REPORT, msg);
        ARK_LOG_INFO("Register self server_id = {}, target_id = {}", server_config->self_id, bus_id);
    }

    void AFCGameNetClientModule::RefreshWorldInfo()
    {
        //do nothing for now
    }

    void AFCGameNetClientModule::SendOnline(const AFGUID& self)
    {
        AFMsg::RoleOnlineNotify xMsg;

        const AFGUID& xGuild = m_pKernelModule->GetNodeObject(self, "GuildID");
        *xMsg.mutable_guild() = AFIMsgModule::GUIDToPB(xGuild);

        //m_pNetClientModule->SendSuitByPB(xGuild.nLow, AFMsg::EGMI_ACK_ONLINE_NOTIFY, xMsg, self);
        m_pMsgModule->SendSuitSSMsg(ARK_APP_WORLD, xGuild.nLow, AFMsg::EGMI_ACK_ONLINE_NOTIFY, xMsg, self);
    }

    void AFCGameNetClientModule::SendOffline(const AFGUID& self)
    {
        AFMsg::RoleOfflineNotify xMsg;

        const AFGUID& xGuild = m_pKernelModule->GetNodeObject(self, "GuildID");
        *xMsg.mutable_guild() = AFIMsgModule::GUIDToPB(xGuild);

        //m_pNetClientModule->SendSuitByPB(xGuild.nLow, AFMsg::EGMI_ACK_OFFLINE_NOTIFY, xMsg, self);
        m_pMsgModule->SendSuitSSMsg(ARK_APP_WORLD, xGuild.nLow, AFMsg::EGMI_ACK_OFFLINE_NOTIFY, xMsg, self);
    }

    void AFCGameNetClientModule::TransPBToProxy(const ARK_PKG_BASE_HEAD& head, const int msg_id, const char* msg, const uint32_t msg_len, const AFGUID& conn_id)
    {
        ARK_PROCESS_ACTOR_STRING_MSG(head, msg, msg_len);
        m_pGameNetServerModule->SendMsgPBToGate(msg_id, msg_data, actor_id);
    }

}