#include "config.hpp"
#include "globals.hpp"
#include "hooks.hpp"
#include "log.hpp"
#include "utils.hpp"

#include "libmem/libmem.h"

#include <windows.h>
#include <thread>
#include <chrono>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

static const char* EXPECTED_STEAMCLIENT_HASH = "df5f99a4b0bbb4ea5d32eb647c1806e2e6b0b6924dd8e52181b1e8a6b969c5ad";

static bool verifySteamClientHash()
{
	auto path = std::filesystem::path(g_modSteamClient.path);
	auto dir = path.parent_path();

	g_pLog->info
	(
		"steamclient.dll loaded from %s\\%s at %p to %p",
		dir.filename().string().c_str(),
		path.filename().string().c_str(),
		g_modSteamClient.base,
		g_modSteamClient.end
	);

	try
	{
		std::string sha256 = Utils::getFileSHA256(path.string().c_str());
		g_pLog->info("steamclient.dll hash is %s", sha256.c_str());

		return strcmp(sha256.c_str(), EXPECTED_STEAMCLIENT_HASH) == 0;
	}
	catch(const std::runtime_error& err)
	{
		g_pLog->warn("Unable to read steamclient.dll hash: %s", err.what());
		return false;
	}
}

static bool setup()
{
	lm_process_t proc {};
	if (!LM_GetProcess(&proc))
	{
		// On failure, the thread will just exit.
		return false;
	}

	// Target the Windows process name.
	if (strcmp(proc.name, "steam.exe") != 0)
	{
		return false;
	}

	g_pLog = std::unique_ptr<CLog>(CLog::createDefaultLog());
	if (!g_pLog)
	{
		return false;
	}

	g_pLog->debug("SuperSexySteam loading in %s", proc.name);


	if(!g_config.init())
	{
		g_pLog->warn("Failed to initialize config.");
		return false;
	}

	return true;
}

static void load()
{
	/*
	// This check is already performed in the Init thread before calling load()
	if (!LM_FindModule("steamclient.dll", &g_modSteamClient))
	{
		g_pLog->warn("load() called but steamclient.dll not found!");
		return;
	}
	*/

	if (!verifySteamClientHash())
	{
		if (g_config.safeMode)
		{
			g_pLog->warn("Unknown steamclient.dll hash! Aborting in safe mode...");
			return;
		}
		else if (g_config.warnHashMissmatch)
		{
			g_pLog->warn("steamclient.dll hash mismatch! Please update the expected hash if this is a new version.");
		}
	}

	if (!Hooks::setup())
	{
		g_pLog->warn("Failed to setup hooks!");
		return;
	}

	if (g_config.notifyInit)
	{
		g_pLog->notify("Loaded successfully");
	}
}

// This is the main function for our initialization thread.
HMODULE g_hModule = NULL;
DWORD WINAPI Init(LPVOID lpParam)
{
	// Global g_hModule that stores the dll path
	g_hModule = static_cast<HMODULE>(lpParam); 
	if (!setup())
	{
		OutputDebugStringA("SuperSexySteam: setup() failed. Unloading DLL.");
		FreeLibraryAndExitThread(g_hModule, 1);
	}

	g_pLog->info("Waiting for steamclient.dll to be loaded...");

	while (!LM_FindModule("steamclient.dll", &g_modSteamClient))
	{
		// Sleep for a short duration
		// TODO: find a better way to detech steamclient.dll for loading
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	g_pLog->info("steamclient.dll found. Proceeding with loading hooks.");

	load();

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved); // Shuts up the compiler

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			// This code runs when the DLL is first injected.
			DisableThreadLibraryCalls(hModule);

			// Create a new thread to run our initialization code. to prevent deadlocks
			HANDLE hThread = CreateThread(nullptr, 0, Init, hModule, 0, nullptr);
			if (hThread)
			{
				// We don't need to manage the thread handle, so we can close it.
				CloseHandle(hThread);
			}
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			if (g_pLog)
			{
				g_pLog->info("Unloading SuperSexySteam...");
			}
			Hooks::remove();
			break;
		}
	}
	return TRUE;
}