#pragma once

#include "libmem/libmem.h"
#include "log.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace MemHlp
{
	enum class SigFollowMode
	{
		None,
		Relative,
		PrologueUpwards
	};

	///Summary:
	///Write assembly code to address and increase address by bytes written
	template<typename ...Args>
	bool assembleCodeAt(lm_address_t& address, const char* fmt, Args... args)
	{
		size_t size = snprintf(nullptr, 0, fmt, args...) + 1;
		char* code = reinterpret_cast<char*>(malloc(size));
		snprintf(code, size, fmt, args...);

		//TODO: Potentially replace with LM_AssembleEx and only allocate memory as needed
		static lm_inst_t inst;
		if (!LM_Assemble(code, &inst))
		{
			g_pLog->debug("Failed to assemble %s!", code);
			return false;
		}
		if (address != LM_ADDRESS_BAD && !LM_WriteMemory(address, inst.bytes, inst.size))
		{
			g_pLog->debug("Failed to write %s to %p!", code, address);
			return false;
		}

		g_pLog->debug("Wrote %s to %p with %i bytes", code, address, inst.size);
		address += inst.size;
		return true;
	}

	std::vector<int16_t> patternToBytes(const char* pattern);
	lm_address_t patternScan(const char* pattern, lm_module_t module);

	lm_address_t searchSignature(const char* name, const char* signature, lm_module_t module, SigFollowMode mode, void* extraData, size_t extraDataSize);
	lm_address_t searchSignature(const char* name, const char* signature, lm_module_t module, SigFollowMode mode);
	lm_address_t searchSignature(const char* name, const char* signature, lm_module_t module);

	lm_address_t getJmpTarget(lm_address_t address);
	lm_address_t findPrologue(lm_address_t address, lm_byte_t* prologueBytes, lm_size_t prologueSize);

	//TODO: Create hooking wrapper that calls this automatically
	bool fixPICThunkCall(const char* name, lm_address_t fn, lm_address_t tramp);
	
	template<typename tFN, typename ...Args>
	constexpr auto callVFunc(unsigned int index, void* thisPtr, Args... args)
	{
		const auto fn = reinterpret_cast<tFN>(*(*reinterpret_cast<lm_address_t***>(thisPtr) + index));
		return fn(thisPtr, args...);
	}
}
