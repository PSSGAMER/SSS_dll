#include "log.hpp"
#include "config.hpp"

#include <cstdlib>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <windows.h>

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
	try
	{
		wchar_t exePathBuffer[MAX_PATH];
		if (GetModuleFileNameW(NULL, exePathBuffer, MAX_PATH) == 0)
		{
			fprintf(stderr, "SuperSexySteam Error: GetModuleFileNameW failed.\n");
			return nullptr;
		}
		
		fs::path logDir = fs::path(exePathBuffer).parent_path();
		logDir /= "config";
		logDir /= "SuperSexySteam";

		// Create the full directory path if it doesn't exist.
		fs::create_directories(logDir);

		fs::path logFile = logDir / "supersexysteam.log";

		return new CLog(logFile.string().c_str());
	}
	catch (const fs::filesystem_error& e)
	{
		fprintf(stderr, "Filesystem error creating log file: %s\n", e.what());
		return nullptr;
	}
	catch (const std::runtime_error& e)
	{
		fprintf(stderr, "Runtime error creating log file: %s\n", e.what());
		return nullptr;
	}

	return nullptr;
}

std::unique_ptr<CLog> g_pLog;