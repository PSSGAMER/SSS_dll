#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <string>

enum class LogLevel : unsigned int
{
	Once,
	Debug,
	Info,
	NotifyShort,
	NotifyLong,
	Warn,
	None
};

class CLog
{
	std::ofstream ofstream;
	std::unordered_set<char*> msgCache;

	constexpr const char* logLvlToStr(LogLevel& lvl)
	{
		switch(lvl)
		{
			case LogLevel::Once:
				return "Once";
			case LogLevel::Debug:
				return "Debug";
			case LogLevel::Info:
				return "Info";
			case LogLevel::NotifyShort:
			case LogLevel::NotifyLong:
				return "Notify";
			case LogLevel::Warn:
				return "Warn";
			
			// Default to satisfy all compilers regarding return paths
			default:
				return "Unknown";
		}
	}

	template<typename ...Args>
	// __attribute__((hot)) // REMOVED: This is a GCC/Clang attribute not supported by MSVC.
	void __log(LogLevel lvl, const char* msg, Args... args)
	{
		size_t size = snprintf(nullptr, 0, msg, args...) + 1; // Allocate one more byte for zero termination
		char* formatted = reinterpret_cast<char*>(malloc(size));
		if (!formatted) return; // Always check malloc result
		snprintf(formatted, size, msg, args...);

		// Trim a single trailing newline if it exists
		size_t len = strlen(formatted);
		if (len > 0 && formatted[len - 1] == '\n')
		{
			formatted[len - 1] = '\0';
		}

		bool freeFormatted = true;
		if (lvl == LogLevel::Once)
		{
			// Can't use match functions from unordered_set because it's to unprecise.
			// We could replace it with our own if we deem it necessary though
			// Loop variable renamed to avoid shadowing the 'msg' parameter.
			for(auto& cached_msg : msgCache)
			{
				if (strcmp(cached_msg, formatted) == 0)
				{
					free(formatted);
					return;
				}
			}

			msgCache.emplace(formatted);
			freeFormatted = false;
		}

		std::stringstream notifySS;

		switch(lvl) // TODO for Windows
		{
			//TODO: Fix possible breakage when there's only one " in formatted
			case LogLevel::NotifyShort:
				// notifySS << "notify-send -t 10000 -u \"normal\" \"SuperSexySteam\" \"" << formatted << "\"";
				break;
			case LogLevel::NotifyLong:
				// notifySS << "notify-send -t 30000 -u \"normal\" \"SuperSexySteam\" \"" << formatted << "\"";
				break;
			case LogLevel::Warn:
				// notifySS << "notify-send -u \"critical\" \"SuperSexySteam\" \"" << formatted << "\"";
				break;
			default:
				break;
		}

		ofstream << "[" << logLvlToStr(lvl) << "] " << formatted << std::endl;

		// if (!notifySS.str().empty())
		// {
		// 	system(notifySS.str().c_str());
		// 	ofstream << "[Debug] system(\"" << notifySS.str() << "\")" << std::endl;
		// }

		if (freeFormatted)
		{
			free(formatted);
		}
	}

public:
	std::string path;

	CLog(const char* path);
	~CLog();

	template<typename ...Args>
	void once(const char* msg, Args... args)
	{
		__log(LogLevel::Once, msg, args...);
	}

	template<typename ...Args>
	void debug(const char* msg, Args... args)
	{
		__log(LogLevel::Debug, msg, args...);
	}

	template<typename ...Args>
	void info(const char* msg, Args... args)
	{
		__log(LogLevel::Info, msg, args...);
	}

	template<typename ...Args>
	void notify(const char* msg, Args... args)
	{
		__log(LogLevel::NotifyShort, msg, args...);
	}

	template<typename ...Args>
	void notifyLong(const char* msg, Args... args)
	{
		__log(LogLevel::NotifyLong, msg, args...);
	}

	template<typename ...Args>
	void warn(const char* msg, Args... args)
	{
		__log(LogLevel::Warn, msg, args...);
	}

	//Do not include config.hpp in this header, otherwise things will break :) (proly due to recursive inclusion)
	static bool shouldNotify();
	static CLog* createDefaultLog();
};

extern std::unique_ptr<CLog> g_pLog;