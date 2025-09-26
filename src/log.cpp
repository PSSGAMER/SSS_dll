#include "log.hpp"
#include "config.hpp"

#include <cstdlib>
#include <memory>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

CLog::CLog(const char* path) : path(path), ofstream(path, std::ios::out)
{
	if (!ofstream.is_open())
	{
		throw std::runtime_error("Unable to open logfile: " + std::string(path));
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
	// msgCache is automatically cleared by its own destructor.
}

// Dirty workaround for not being able to access g_config from __log
bool CLog::shouldNotify()
{
	// This function requires the full definition of g_config from "config.hpp"
	return g_config.notifications;
}

CLog* CLog::createDefaultLog()
{
	const char* appdata = getenv("APPDATA");
	if (appdata)
	{
		try
		{
			fs::path logDir = fs::path(appdata) / "SuperSexySteam";

			// This is safe to call even if the directory already exists.
			fs::create_directory(logDir);

			fs::path logFile = logDir / "SSS_dll.log";

			return new CLog(logFile.string().c_str());
		}
		catch (const fs::filesystem_error& e)
		{
			fprintf(stderr, "Filesystem error creating log file: %s\n", e.what());
			return nullptr;
		}
	}

	return nullptr;
}

std::unique_ptr<CLog> g_pLog;