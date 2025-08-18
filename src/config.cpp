#include "config.hpp"

#include "log.hpp"
#include "yaml-cpp/yaml.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

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
"#DenuvoGames:\n"
"#  SteamId:\n"
"#    -  AppId1\n"
"#    -  AppId2\n"
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
"#Automatically disable SLSsteam when steamclient.so does not match a predefined file hash that is known to work\n"
"#You should enable this if you're planing to use SLSsteam with Steam Deck's gamemode\n"
"SafeMode: no\n\n"
"#Warn user via notification when steamclient.so hash differs from known safe hash\n"
"#Mostly useful for development so I don't accidentally miss an update\n"
"WarnHashMissmatch: no\n\n"
"#Logs all calls to Steamworks (this makes the logfile huge! Only useful for debugging/analyzing\n"
"ExtendedLogging: no";

std::string CConfig::getDir()
{
	char pathBuf[255];
	const char* configDir = getenv("XDG_CONFIG_HOME"); //Most users should have this set iirc
	if (configDir != NULL)
	{
		sprintf(pathBuf, "%s/SLSsteam", configDir);
	} else
	{
		const char* home = getenv("HOME");
		sprintf(pathBuf, "%s/.config/SLSsteam", home);
	}

	return std::string(pathBuf);
}

std::string CConfig::getPath()
{
	return getDir().append("/config.yaml");
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
				g_pLog->notify("Unable to create config directory at %s!\n", dir.c_str());
				return false;
			}

			g_pLog->debug("Created config directory at %s\n");
		}

		FILE* file = fopen(path.c_str(), "w");
		if (!file)
		{
			g_pLog->notify("Unable to create config at %s!\n", path.c_str());
			return false;
		}

		fputs(defaultConfig, file);
		fflush(file);
		fclose(file);
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
	warnHashMissmatch = getSetting<bool>(node, "WarnHashMissmatch", false);
	extendedLogging = getSetting<bool>(node, "ExtendedLogging", false);
	denuvoSpoof = getSetting<bool>(node, "DenuvoSpoof", false);

	//TODO: Create smart logging function to log them automatically via getSetting
	g_pLog->info("DisableFamilyShareLock: %i\n", disableFamilyLock);
	g_pLog->info("UseWhitelist: %i\n", useWhiteList);
	g_pLog->info("AutoFilterList: %i\n", automaticFilter);
	g_pLog->info("PlayNotOwnedGames: %i\n", playNotOwnedGames);
	g_pLog->info("SafeMode: %i\n", safeMode);
	g_pLog->info("WarnHashMissmatch: %i\n", warnHashMissmatch);
	g_pLog->info("ExtendedLogging: %i\n", extendedLogging);
	g_pLog->info("DenuvoSpoof: %i\n", denuvoSpoof);

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
				g_pLog->info("Added %u to AppIds\n", appId);
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
				g_pLog->info("Added %u to AdditionalApps\n", appId);
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
				g_pLog->debug("Adding DlcData for %u\n", parentId);

				for(auto& dlc : app.second)
				{
					const uint32_t dlcId = dlc.first.as<uint32_t>();
					//There's more efficient types to store strings, but they mostly do not work
					const std::string dlcName = dlc.second.as<std::string>();

					data.dlcIds[dlcId] = dlcName;
					g_pLog->debug("DlcId %u -> %s\n", dlcId, dlcName.c_str());
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
					g_pLog->debug("Added DenuvoGame %u\n", appId, steamId);
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
	g_pLog->once("Force owned %u\n", appId); //once is unnessecary but just for consistency
	return true;
}

bool CConfig::shouldExcludeAppId(uint32_t appId)
{
	bool exclude = false;
	//Proper way would be with getAppType, but that seems broken so we need to do this instead
	constexpr uint32_t ONE_BILLION = 1E9; //Implicit cast from double to unsigned int, hopefully this does not break anything
	if (appId >= ONE_BILLION) //Higher and equal to 10^9 gets used by Steam Internally
	{
		exclude = true;
	}
	else
	{
		bool found = appIds.contains(appId);
		exclude = !isAddedAppId(appId) && ((useWhiteList && !found) || (!useWhiteList && found));
	}

	g_pLog->once("shouldExcludeAppId(%u) -> %i\n", appId, exclude);
	return exclude;
}

uint32_t CConfig::getDenuvoGameOwner(uint32_t appId)
{
	for(const auto& tpl : denuvoGames)
	{
		if (tpl.second.contains(appId))
		{
			g_pLog->once("%u is DenuvoGame\n", appId);
			return tpl.first;
		}
	}

	return 0;
}

CConfig g_config = CConfig();
