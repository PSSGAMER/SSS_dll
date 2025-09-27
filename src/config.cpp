#include "config.hpp"

#include "log.hpp"
#include "yaml-cpp/yaml.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <fstream>
#include <windows.h>

//TODO: Move into own .yaml file somehow
static const char* defaultConfig = 
"#Example AppIds Config for those not familiar with YAML:\n"
"#AppIds:\n"
"#  - 440\n"
"#  - 730\n"
"#Take care of not messing up your spaces! Otherwise it won't work\n\n"
"#Example of DlcData:\n"
"#DlcData:\n"
"#  AppId:\n"
"#    FirstDlcAppId: \"Dlc Name\"\n"
"#    SecondDlcAppId: \"Dlc Name\"\n\n"
"#Example of DenuvoGames:\n"
"#DenuvoGames:\n"
"#  SteamId:\n"
"#    -  AppId1\n"
"#    -  AppId2\n\n"
"#Disables Family Share license locking for self and others\n"
"DisableFamilyShareLock: yes\n\n"
"#Switches to whitelist instead of the default blacklist\n"
"UseWhitelist: no\n\n"
"#Automatically filter Apps in CheckAppOwnership. Filters everything but Games and Applications. Should not affect DLC checks\n"
"#Overrides black-/whitelist. Gets overriden by AdditionalApps\n"
"AutoFilterList: yes\n\n"
"#List of AppIds to ex-/include\n"
"AppIds:\n\n"
"#Enables playing of not owned games. Respects black-/whitelist AppIds\n"
"PlayNotOwnedGames: no\n\n"
"#Additional AppIds to inject (Overrides your black-/whitelist & also overrides OwnerIds for apps you got shared!) Best to use this only on games NOT in your library.\n"
"AdditionalApps:\n\n"
"#Extra Data for Dlcs belonging to a specific AppId. Only needed\n"
"#when the App you're playing is hit by Steams 64 DLC limit\n"
"DlcData:\n\n"
"#Blocks games from unlocking on wrong accounts\n"
"DenuvoGames:\n\n"
"#Spoof Denuvo Games owner instead of blocking them\n"
"DenuvoSpoof: no\n\n"
"#Automatically disable SuperSexySteam when steamclient.dll does not match a predefined file hash that is known to work\n"
"#You should enable this if you're planing to use SuperSexySteam with Steam Deck's gamemode\n"
"SafeMode: no\n\n"
"#Toggles notifications via notify-send\n"
"Notifications: yes\n\n"
"#Warn user via notification when steamclient.dll hash differs from known safe hash\n"
"#Mostly useful for development so I don't accidentally miss an update\n"
"WarnHashMissmatch: no\n\n"
"#Notify when SuperSexySteam is done initializing\n"
"NotifyInit: yes\n\n"
"#Logs all calls to Steamworks (this makes the logfile huge! Only useful for debugging/analyzing\n"
"ExtendedLogging: no";

std::string CConfig::getDir()
{
	wchar_t exePathBuffer[MAX_PATH];

	// Get the full path of the host executable (e.g., steam.exe).
	if (GetModuleFileNameW(NULL, exePathBuffer, MAX_PATH) == 0)
	{
		g_pLog->notify("Could not get host executable path. Config cannot be loaded or created.");
		return "";
	}

	// Get the directory containing the executable and append the config path.
	std::filesystem::path configPath = std::filesystem::path(exePathBuffer).parent_path();
	configPath /= "config";
	configPath /= "SuperSexySteam";

	return configPath.string();
}

std::string CConfig::getPath()
{
	std::filesystem::path configPath = getDir();
	if (configPath.empty())
	{
		return "";
	}
	configPath /= "config.yaml";
	return configPath.string();
}

bool CConfig::createFile()
{
	std::string path = getPath();
	if (!std::filesystem::exists(path))
	{
		std::string dir = getDir();
		if (!std::filesystem::exists(dir))
		{
			if (!std::filesystem::create_directory(dir))
			{
				g_pLog->notify("Unable to create config directory at %s!", dir.c_str());
				return false;
			}

			g_pLog->debug("Created config directory at %s", dir.c_str());
		}

		std::ofstream configFile(path);
		if (!configFile.is_open())
		{
			g_pLog->notify("Unable to create config at %s!", path.c_str());
			return false;
		}

		configFile << defaultConfig;
	}

	return true;
}

bool CConfig::init()
{
	createFile();
	loadSettings();
	return true;
}

bool CConfig::loadSettings()
{
	YAML::Node node;
	try
	{
		node = YAML::LoadFile(getPath());
	}
	catch (YAML::BadFile& bf)
	{
		g_pLog->notifyLong("Can not read config.yaml! %s\nUsing defaults", bf.msg.c_str());
		node = YAML::Node(); //Create empty node and let defaults kick in
	}
	catch (YAML::ParserException& pe)
	{
		g_pLog->notifyLong("Error parsing config.yaml! %s\nUsing defaults", pe.msg.c_str());
		node = YAML::Node(); //Create empty node and let defaults kick in
	}
	
	disableFamilyLock = getSetting<bool>(node, "DisableFamilyShareLock", true);
	useWhiteList = getSetting<bool>(node, "UseWhitelist", false);
	automaticFilter = getSetting<bool>(node, "AutoFilterList", true);
	playNotOwnedGames = getSetting<bool>(node, "PlayNotOwnedGames", false);
	safeMode = getSetting<bool>(node, "SafeMode", false);
	notifications = getSetting<bool>(node, "Notifications", true);
	warnHashMissmatch = getSetting<bool>(node, "WarnHashMissmatch", false);
	notifyInit = getSetting<bool>(node, "NotifyInit", true);
	extendedLogging = getSetting<bool>(node, "ExtendedLogging", false);
	denuvoSpoof = getSetting<bool>(node, "DenuvoSpoof", false);

	//TODO: Create smart logging function to log them automatically via getSetting
	g_pLog->info("DisableFamilyShareLock: %i", disableFamilyLock);
	g_pLog->info("UseWhitelist: %i", useWhiteList);
	g_pLog->info("AutoFilterList: %i", automaticFilter);
	g_pLog->info("PlayNotOwnedGames: %i", playNotOwnedGames);
	g_pLog->info("SafeMode: %i", safeMode);
	g_pLog->info("Notifications: %i", notifications);
	g_pLog->info("WarnHashMissmatch: %i", warnHashMissmatch);
	g_pLog->info("NotifyInit: %i", notifyInit);
	g_pLog->info("ExtendedLogging: %i", extendedLogging);
	g_pLog->info("DenuvoSpoof: %i", denuvoSpoof);

	//TODO: Create function to parse these kinda nodes, instead of c+p them
	const auto appIdsNode = node["AppIds"];
	if (appIdsNode)
	{
		for(auto& appIdNode : appIdsNode)
		{
			try
			{
				uint32_t appId = appIdNode.as<uint32_t>();
				this->appIds.emplace(appId);
				g_pLog->info("Added %u to AppIds", appId);
			}
			catch(...)
			{
				g_pLog->notify("Failed to parse %s in AppIds!", appIdNode.as<std::string>().c_str());
			}
		}
	}
	else
	{
		g_pLog->notify("Missing AppIds entry in config!");
	}

	const auto additionalAppsNode = node["AdditionalApps"];
	if (additionalAppsNode)
	{
		for(auto& appIdNode : additionalAppsNode)
		{
			try
			{
				uint32_t appId = appIdNode.as<uint32_t>();
				this->addedAppIds.emplace(appId);
				g_pLog->info("Added %u to AdditionalApps", appId);
			}
			catch(...)
			{
				g_pLog->notify("Failed to parse %s in AdditionalApps!", appIdNode.as<std::string>().c_str());
			}
		}
	}
	else
	{
		g_pLog->notify("Missing AdditionalApps entry in config!");
	}

	const auto dlcDataNode = node["DlcData"];
	if(dlcDataNode)
	{
		for(auto& app : dlcDataNode)
		{
			try
			{
				const uint32_t parentId = app.first.as<uint32_t>();

				CDlcData data;
				data.parentId = parentId;
				g_pLog->debug("Adding DlcData for %u", parentId);

				for(auto& dlc : app.second)
				{
					const uint32_t dlcId = dlc.first.as<uint32_t>();
					//There's more efficient types to store strings, but they mostly do not work
					const std::string dlcName = dlc.second.as<std::string>();

					data.dlcIds[dlcId] = dlcName;
					g_pLog->debug("DlcId %u -> %s", dlcId, dlcName.c_str());
				}

				dlcData[parentId] = data;
			}
			catch(...)
			{
				g_pLog->notify("Failed to parse DlcData!");
				break;
			}
		}
	}
	else
	{
		g_pLog->notify("Missing DlcData entry in config!");
	}

	const auto denuvoGamesNode = node["DenuvoGames"];
	if (denuvoGamesNode)
	{
		for (auto& steamIdNode : denuvoGamesNode)
		{
			try
			{
				const uint32_t steamId = steamIdNode.first.as<uint32_t>();
				denuvoGames[steamId] = std::unordered_set<uint32_t>();

				for (auto& appIdNode : steamIdNode.second)
				{
					const uint32_t appId = appIdNode.as<uint32_t>();
					denuvoGames[steamId].emplace(appId);

					//Again, not loggin SteamId because of privacy
					g_pLog->debug("Added DenuvoGame %u", appId);
				}
			}
			catch (...)
			{
				g_pLog->notify("Failed to parse DenuvoGames!");
			}
		}
	}
	else
	{
		g_pLog->notify("Missing DenuvoGames entry in config!");
	}

	return true;
}

bool CConfig::isAddedAppId(uint32_t appId)
{
	return addedAppIds.contains(appId);
}

bool CConfig::addAdditionalAppId(uint32_t appId)
{
	if (isAddedAppId(appId))
		return false;

	addedAppIds.emplace(appId);
	g_pLog->once("Force owned %u", appId); //once is unnessecary but just for consistency
	return true;
}

bool CConfig::shouldExcludeAppId(uint32_t appId)
{
	bool exclude = false;
	//Proper way would be with getAppType, but that seems broken so we need to do this instead
	constexpr uint32_t ONE_BILLION = 1000000000; //Use integer literal 1 billion or 1E9 to avoid implicit cast
	if (appId >= ONE_BILLION) //Higher and equal to 10^9 gets used by Steam Internally
	{
		exclude = true;
	}
	else
	{
		bool found = appIds.contains(appId);
		exclude = !isAddedAppId(appId) && ((useWhiteList && !found) || (!useWhiteList && found));
	}

	g_pLog->once("shouldExcludeAppId(%u) -> %i", appId, exclude);
	return exclude;
}

uint32_t CConfig::getDenuvoGameOwner(uint32_t appId)
{
	for(const auto& tpl : denuvoGames)
	{
		if (tpl.second.contains(appId))
		{
			//g_pLog->once("%u is DenuvoGame", appId);
			return tpl.first;
		}
	}

	return 0;
}

CConfig g_config = CConfig();