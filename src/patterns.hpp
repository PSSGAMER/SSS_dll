#pragma once

#include "libmem/libmem.h"

namespace Patterns
{
	//string xref
	constexpr lm_string_t CheckAppOwnership = "38 53 ? ? ? 56 83 38 00 57 ? ? ? ? ? 33";

	//string xref
	constexpr lm_string_t FamilyGroupRunningApp = "? ? ? ? ? ? ? ? ? ? ? ? 8B 86 70 52";

	//string xref
	constexpr lm_string_t StopPlayingBorrowedApp = "? ? ? ? ? ? ? ? ? ? ? ? 33 DB 8B 50";

	//s-xref - breakpoint at call of vfunc - into for that function - xref
	constexpr lm_string_t GetSubscribedApps = "? ? ? 33 C0 ? ? ? ? ? 50 6A 04 68 8C 8F";

	//Relative TODO
	constexpr lm_string_t IsUserSubscribedAppInTicket = "E8 ? ? ? ? 89 C3 83 C4 20 8B ? ? ? ? ? 8B";

	//Relative TODO
	constexpr lm_string_t IsSubscribedApp = "E8 ? ? ? ? 83 C4 10 84 C0 74 ? 8B 95 ? ? ? ? 83 EC 04";
	
	//End of function TODO
	constexpr lm_string_t RequiresLegacyCDKey = "C3 ? ? ? ? ? 8B 44 24 ? 83 C4 1C 89 F9 89 F2 5B 5E 5F 5D 2D 94 18 00 00";

	//s-xref - breakpoint at call of vfunc - into for that function
	constexpr lm_string_t GetSteamId = "? ? ? ? ? ? 8B 91 64 E8 FF FF 8B 89 68 E8";


	//PipeLoops - string xref - biggest one

	constexpr lm_string_t IClientAppManager_PipeLoop = "? ? ? ? ? 56 57 6A 04 50 53 ? ? ? ? ? ? ? ? ? ? ? 3D B7";

	constexpr lm_string_t IClientApps_PipeLoop = "? ? ? ? ? 56 57 6A 04 50 53 ? ? ? ? ? ? ? ? ? ? ? 3D 28";

	constexpr lm_string_t IClientRemoteStorage_PipeLoop = "? ? ? ? 56 57 6A 04 50 53 ? ? ? ? ? ? ? ? ? ? ? 3D A2 AB";

	//should be where the string GetSubscribedApps is cross referenced (xrefed) - biggest one
	constexpr lm_string_t IClientUser_PipeLoop = "? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 56 57 8B 7B 0C ? ? ? 6A";

	//It should be called directly after the IClientUser and GetSubscribedApps strings get pushed onto the stack
	constexpr lm_string_t LogSteamPipeCall = "? ? ? 8B 0D D4 18 E8 38 ? ? ? ? ? ? ? ? ? ? ? ? 85 C0 ?";

}

