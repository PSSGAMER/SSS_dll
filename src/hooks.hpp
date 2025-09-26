#pragma once
#include "memhlp.hpp"

#include "libmem/libmem.h"

#include "sdk/CAppOwnershipInfo.hpp"

#include <cstddef>
#include <memory>
#include <string>

#define THISCALL_TYPE __thiscall   // For original C++ member functions
#define FASTCALL_TYPE __fastcall     // For our hook handlers that intercept __thiscall


template<typename T>
union FunctionUnion_t
{
	T fn;
	lm_address_t address;
};

// Base Hook class now uses two template types: one for the original function, one for our hook.
template<typename OriginalFnT, typename HookFnT>
class Hook
{
public:
	std::string name;
	FunctionUnion_t<OriginalFnT> originalFn;
	FunctionUnion_t<HookFnT> hookFn;

	Hook(const char* name);

	virtual void place() = 0;
	virtual void remove() = 0;
};

template<typename OriginalFnT, typename HookFnT>
class DetourHook : public Hook<OriginalFnT, HookFnT>
{
public:
	FunctionUnion_t<OriginalFnT> tramp;
	size_t size;

	DetourHook(const char* name);

	virtual void place();
	virtual void remove();

	bool setup(const char* pattern, const MemHlp::SigFollowMode followMode, lm_byte_t* extraData, lm_size_t extraDataSize, HookFnT hookFn);
	bool setup(const char* pattern, const MemHlp::SigFollowMode followMode, HookFnT hookFn);
};

template<typename OriginalFnT, typename HookFnT>
class VFTHook : public Hook<OriginalFnT, HookFnT>
{
public:
	std::shared_ptr<lm_vmt_t> vft;
	unsigned int index;
	bool hooked;

	VFTHook(const char* name);

	virtual void place();
	virtual void remove();

	void setup(std::shared_ptr<lm_vmt_t> vft, unsigned int index, HookFnT hookFn);
};

namespace Hooks
{
	// These hook non-member functions, so their original and hook types are the same.
	typedef void(*LogSteamPipeCall_t)(const char*, const char*);
	typedef bool(*CheckAppOwnership_t)(void*, uint_fast32_t, CAppOwnershipInfo*);

	// For member functions, we need two types: the original __thiscall type,
	// and our hook's __fastcall type which captures the 'this' pointer in ECX.
	typedef void (THISCALL_TYPE *IClientAppManager_PipeLoop_t)(void*, void*, void*, void*);
	typedef void (FASTCALL_TYPE *IClientAppManager_PipeLoop_Hook_t)(void* ecx, void* edx, void* a1, void* a2, void* a3);

	typedef void (THISCALL_TYPE *IClientApps_PipeLoop_t)(void*, void*, void*, void*);
	typedef void (FASTCALL_TYPE *IClientApps_PipeLoop_Hook_t)(void* ecx, void* edx, void* a1, void* a2, void* a3);

	typedef void (THISCALL_TYPE *IClientRemoteStorage_PipeLoop_t)(void*, void*, void*, void*);
	typedef void (FASTCALL_TYPE *IClientRemoteStorage_PipeLoop_Hook_t)(void* ecx, void* edx, void* a1, void* a2, void* a3);

	typedef bool (THISCALL_TYPE *IClientUser_BIsSubscribedApp_t)(void*, uint32_t);
	typedef bool (FASTCALL_TYPE *IClientUser_BIsSubscribedApp_Hook_t)(void* ecx, void* edx, uint32_t appId);

	typedef uint32_t (THISCALL_TYPE *IClientUser_GetSubscribedApps_t)(void*, uint32_t*, size_t, bool);
	typedef uint32_t (FASTCALL_TYPE *IClientUser_GetSubscribedApps_Hook_t)(void* ecx, void* edx, uint32_t* pAppList, size_t size, bool a3);

	typedef uint8_t (THISCALL_TYPE *IClientUser_IsUserSubscribedAppInTicket_t)(void*, uint32_t, uint32_t, uint32_t, uint32_t);
	typedef uint8_t (FASTCALL_TYPE *IClientUser_IsUserSubscribedAppInTicket_Hook_t)(void* ecx, void* edx, uint32_t steamId, uint32_t a2, uint32_t a3, uint32_t appId);

	typedef bool (THISCALL_TYPE *IClientUser_RequiresLegacyCDKey_t)(void*, uint32_t, uint32_t*);
	typedef bool (FASTCALL_TYPE *IClientUser_RequiresLegacyCDKey_Hook_t)(void* ecx, void* edx, uint32_t appId, uint32_t* a2);

	typedef bool(THISCALL_TYPE *IClientAppManager_BIsDlcEnabled_t)(void*, uint32_t, uint32_t, void*);
	typedef bool(FASTCALL_TYPE *IClientAppManager_BIsDlcEnabled_Hook_t)(void* ecx, void* edx, uint32_t appId, uint32_t dlcId, void* a3);

	typedef bool(THISCALL_TYPE *IClientAppManager_GetAppUpdateInfo_t)(void*, uint32_t, uint32_t*);
	typedef bool(FASTCALL_TYPE *IClientAppManager_GetAppUpdateInfo_Hook_t)(void* ecx, void* edx, uint32_t appId, uint32_t* a2);

	typedef void*(THISCALL_TYPE *IClientAppManager_LaunchApp_t)(void*, uint32_t*, void*, void*, void*);
	typedef void*(FASTCALL_TYPE *IClientAppManager_LaunchApp_Hook_t)(void* ecx, void* edx, uint32_t* pAppId, void* a2, void* a3, void* a4);

	typedef bool(THISCALL_TYPE *IClientAppManager_IsAppDlcInstalled_t)(void*, uint32_t, uint32_t);
	typedef bool(FASTCALL_TYPE *IClientAppManager_IsAppDlcInstalled_Hook_t)(void* ecx, void* edx, uint32_t appId, uint32_t dlcId);

	typedef unsigned int(THISCALL_TYPE *IClientApps_GetDLCCount_t)(void*, uint32_t);
	typedef unsigned int(FASTCALL_TYPE *IClientApps_GetDLCCount_Hook_t)(void* ecx, void* edx, uint32_t appId);

	typedef bool(THISCALL_TYPE *IClientApps_GetDLCDataByIndex_t)(void*, uint32_t, int, uint32_t*, bool*, char*, size_t);
	typedef bool(FASTCALL_TYPE *IClientApps_GetDLCDataByIndex_Hook_t)(void* ecx, void* edx, uint32_t appId, int dlcIndex, uint32_t* pDlcId, bool* pIsAvailable, char* pChDlcName, size_t dlcNameLen);

	typedef bool(THISCALL_TYPE *IClientRemoteStorage_IsCloudEnabledForApp_t)(void*, uint32_t);
	typedef bool(FASTCALL_TYPE *IClientRemoteStorage_IsCloudEnabledForApp_Hook_t)(void* ecx, void* edx, uint32_t appId);


	// For non-member function hooks, both template arguments are the same.
	extern DetourHook<LogSteamPipeCall_t, LogSteamPipeCall_t> LogSteamPipeCall;
	extern DetourHook<CheckAppOwnership_t, CheckAppOwnership_t> CheckAppOwnership;

	// For member function hooks, specify the original __thiscall type and our __fastcall hook type.
	extern DetourHook<IClientAppManager_PipeLoop_t, IClientAppManager_PipeLoop_Hook_t> IClientAppManager_PipeLoop;
	extern DetourHook<IClientApps_PipeLoop_t, IClientApps_PipeLoop_Hook_t> IClientApps_PipeLoop;
	extern DetourHook<IClientRemoteStorage_PipeLoop_t, IClientRemoteStorage_PipeLoop_Hook_t> IClientRemoteStorage_PipeLoop;
	extern DetourHook<IClientUser_BIsSubscribedApp_t, IClientUser_BIsSubscribedApp_Hook_t> IClientUser_BIsSubscribedApp;
	extern DetourHook<IClientUser_IsUserSubscribedAppInTicket_t, IClientUser_IsUserSubscribedAppInTicket_Hook_t> IClientUser_IsUserSubscribedAppInTicket;
	extern DetourHook<IClientUser_GetSubscribedApps_t, IClientUser_GetSubscribedApps_Hook_t> IClientUser_GetSubscribedApps;
	extern DetourHook<IClientUser_RequiresLegacyCDKey_t, IClientUser_RequiresLegacyCDKey_Hook_t> IClientUser_RequiresLegacyCDKey;

	extern VFTHook<IClientAppManager_BIsDlcEnabled_t, IClientAppManager_BIsDlcEnabled_Hook_t> IClientAppManager_BIsDlcEnabled;
	extern VFTHook<IClientAppManager_GetAppUpdateInfo_t, IClientAppManager_GetAppUpdateInfo_Hook_t> IClientAppManager_GetAppUpdateInfo;
	extern VFTHook<IClientAppManager_LaunchApp_t, IClientAppManager_LaunchApp_Hook_t> IClientAppManager_LaunchApp;
	extern VFTHook<IClientAppManager_IsAppDlcInstalled_t, IClientAppManager_IsAppDlcInstalled_Hook_t> IClientAppManager_IsAppDlcInstalled;

	extern VFTHook<IClientApps_GetDLCDataByIndex_t, IClientApps_GetDLCDataByIndex_Hook_t> IClientApps_GetDLCDataByIndex;
	extern VFTHook<IClientApps_GetDLCCount_t, IClientApps_GetDLCCount_Hook_t> IClientApps_GetDLCCount;

	extern VFTHook<IClientRemoteStorage_IsCloudEnabledForApp_t, IClientRemoteStorage_IsCloudEnabledForApp_Hook_t> IClientRemoteStorage_IsCloudEnabledForApp;

	extern lm_address_t IClientUser_GetSteamId;

	bool setup();
	void place();
	void remove();
}