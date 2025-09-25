#include "log.hpp"
#include "config.hpp"

#include <cstdlib>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

CLog::CLog(const char* path) : path(path)
{
	ofstream = std::ofstream(path, std::ios::out);
	if (!ofstream.is_open())
	{
		throw std::runtime_error("Unable to open logfile!");
	}
}

CLog::~CLog()
{
	if (ofstream.is_open())
	{
		ofstream.close();
	}

	for(auto& msg : msgCache)
	{
		free(msg);
	}
}

//Dirty workaround for not being able to access g_config from __log
bool CLog::shouldNotify()
{
	return g_config.notifications;
}

CLog* CLog::createDefaultLog()
{
	const char* appdata = getenv("APPDATA");
	if (appdata)
	{
		fs::path logDir = fs::path(appdata) / "SuperSexySteam";

		// This is safe to call even if it's already there.
		fs::create_directory(logDir);

		fs::path logFile = logDir / "SSS_dll.log";

		return new CLog(logFile.string().c_str());
	}

	return nullptr;
}

std::unique_ptr<CLog> g_pLog;