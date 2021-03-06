/*
 * This source file is part of ARK
 * For the latest info, see https://github.com/ArkNX
 *
 * Copyright (c) 2013-2019 ArkNX authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#include <args/args.hxx>

#include "base/AFBus.hpp"
#include "base/AFMacros.hpp"
#include "base/AFDateTime.hpp"
#include "base/AFMisc.hpp"
#include "base/AFPluginManager.hpp"

using namespace ark;

bool g_exit_loop = false;

#ifdef ARK_PLATFORM_WIN
// command control
std::thread g_cmd_thread;

// mini-dump
void CreateDumpFile(const std::string& strDumpFilePathName, EXCEPTION_POINTERS* pException)
{
    HANDLE hDumpFile =
        CreateFile(strDumpFilePathName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

    CloseHandle(hDumpFile);
}

long ApplicationCrashHandler(EXCEPTION_POINTERS* pException)
{
    AFDateTime now;
    std::string dump_name =
        ARK_FORMAT("{}-{:04d}{:02d}{:02d}_{:02d}_{:02d}_{:02d}.dmp", AFPluginManager::instance()->GetAppName(),
            now.GetYear(), now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond());

    CreateDumpFile(dump_name.c_str(), pException);

    FatalAppExit(-1, dump_name.c_str());
    return EXCEPTION_EXECUTE_HANDLER;
}

void CloseXButton()
{
    HWND hWnd = GetConsoleWindow();

    if (hWnd)
    {
        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        EnableMenuItem(hMenu, SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);
    }
}

#elif defined(ARK_PLATFORM_LINUX) || defined(ARK_PLATFORM_DARWIN)

void KillHandler(int s)
{
    AFPluginManager::instance()->Stop();
}

void InitDaemon()
{
    // ignore signals
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    // kill signals
    signal(SIGTERM, KillHandler);
    signal(SIGSTOP, KillHandler);
    signal(SIGINT, KillHandler);

    int ret = daemon(1, 0);
    ARK_ASSERT_NO_EFFECT(ret == 0);
}
#endif

void PrintLogo()
{
#ifdef ARK_PLATFORM_WIN
    SetConsoleTextAttribute(
        GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif

    std::string logo = R"(
*************************************************
       _         _    
      / \   _ __| | __
     / _ \ | '__| |/ /
    / ___ \| |  |   < 
   /_/   \_\_|  |_|\_\

Copyright 2019 (c) ArkNX. All Rights Reserved.
Website: https://arknx.com
Github:  https://github.com/ArkNX
*************************************************
)";

    CONSOLE_INFO_LOG << logo << std::endl;

#ifdef ARK_PLATFORM_WIN
    SetConsoleTextAttribute(
        GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

#ifdef ARK_PLATFORM_WIN
void CreateBackThread()
{
    auto thread_func = []() {
        while (!g_exit_loop)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::string s;
            std::cin >> s;
            AFStringUtils::ToLower(s);
            if (s == "exit")
            {
                g_exit_loop = true;
            }
        }
    };

    g_cmd_thread = std::thread(thread_func);
}
#endif // ARK_PLATFORM_WIN

bool ParseArgs(int argc, char* argv[])
{
    auto close_x_button = []() {
#ifdef ARK_PLATFORM_WIN
        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
        CloseXButton();
#endif
    };

    auto use_daemon = []() {
#if defined(ARK_PLATFORM_LINUX) || defined(ARK_PLATFORM_DARWIN)
        InitDaemon();
#endif
    };

    args::ArgumentParser parser(
        "Here is ark plugin loader argument tools", "If you have any questions, please report an issue in GitHub.");
    args::HelpFlag help(parser, "help", "Display the help menu", {'h', "help"});
    args::ActionFlag xbutton(parser, "close", "Close [x] button in Windows", {'x'}, close_x_button);
    args::ActionFlag daemon(parser, "daemon", "Run application as daemon", {'d'}, use_daemon);

    args::ValueFlag<std::string> busid(parser, "busid", "Set application id(like IP address: 8.8.8.8)", {'b', "busid"},
        "8.8.8.8", args::Options::Required | args::Options::Single);
    args::ValueFlag<std::string> name(parser, "name", "Set application name", {'n', "name"}, "my-server",
        args::Options::Required | args::Options::Single);
    args::ValueFlag<std::string> plugin_cfg(parser, "plugin config path", "Set application plugin config",
        {'p', "plugin"}, "plugin.xml", args::Options::Required | args::Options::Single);
    std::string default_log_path = ARK_FORMAT("..{}binlog", ARK_FOLDER_SEP);
    args::ValueFlag<std::string> logpath(parser, "logpath", "Set application log output path", {'l', "logpath"},
        default_log_path, args::Options::Required | args::Options::Single);

    // start parse argument list
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help& help)
    {
        CONSOLE_ERROR_LOG << parser << ", error = " << help.what() << std::endl;
        return false;
    }
    catch (args::ParseError& e)
    {
        CONSOLE_ERROR_LOG << parser << ", error = " << e.what() << std::endl;
        return false;
    }
    catch (args::ValidationError& e)
    {
        CONSOLE_ERROR_LOG << parser << ", error = " << e.what() << std::endl;
        return false;
    }

    // Set bus id
    if (busid)
    {
        std::vector<AFSlice> slices;
        AFStringUtils::Split(slices, busid.Get(), ".");
        if (slices.size() != 4)
        {
            CONSOLE_ERROR_LOG << "Bus id is invalid, it likes 8.8.8.8" << std::endl;
            return false;
        }

        AFBusAddr busaddr;
        busaddr.channel_id = ARK_LEXICAL_CAST<uint8_t>(slices[0].data());
        busaddr.zone_id = ARK_LEXICAL_CAST<uint8_t>(slices[1].data());
        busaddr.app_type = ARK_LEXICAL_CAST<uint8_t>(slices[2].data());
        busaddr.inst_id = ARK_LEXICAL_CAST<uint8_t>(slices[3].data());

        AFPluginManager::instance()->SetBusID(busaddr.bus_id);
    }
    else
    {
        return false;
    }

    // Set app name
    if (name)
    {
        AFPluginManager::instance()->SetAppName(name.Get());

        std::string process_name =
            ARK_FORMAT("{}-{}-{}", name.Get(), busid.Get(), AFPluginManager::instance()->GetBusID());
        // Set process name
#ifdef ARK_PLATFORM_WIN
        SetConsoleTitle(process_name.c_str());
#elif defined(ARK_PLATFORM_LINUX)
        // Do not need to change process name
#endif
    }
    else
    {
        return false;
    }

    // Set plugin file
    if (plugin_cfg)
    {
        AFPluginManager::instance()->SetPluginConf(plugin_cfg.Get());
    }
    else
    {
        return false;
    }

    if (logpath)
    {
        AFPluginManager::instance()->SetLogPath(logpath.Get());
    }
    else
    {
        return false;
    }

#ifdef ARK_PLATFORM_WIN
    // Create back thread, for some command
    CreateBackThread();
#endif

    return true;
}

void MainLoop()
{
#ifdef ARK_PLATFORM_WIN
    __try
    {
        AFPluginManager::instance()->Update();
    }
    __except (ApplicationCrashHandler(GetExceptionInformation()))
    {
        // Do nothing for now.
    }
#else
    AFPluginManager::instance()->Update();
#endif
}

int main(int argc, char* argv[])
{
    if (!ParseArgs(argc, argv))
    {
        CONSOLE_INFO_LOG << "Application parameter is invalid, please check it..." << std::endl;
        return 0;
    }

    PrintLogo();

    AFPluginManager::instance()->Start();

    for (;; std::this_thread::sleep_for(std::chrono::milliseconds(1)))
    {
        if (g_exit_loop)
        {
            break;
        }

        MainLoop();
    }

    AFPluginManager::instance()->Stop();

#ifdef ARK_PLATFORM_WIN
    g_cmd_thread.join();
#endif

    return 0;
}
