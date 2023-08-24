#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include <random>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class AirRollTimer: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	void onLoad() override;
	//void onUnload() override; // Uncomment and implement if you need a unload method
	void UpdateTimer();
	void GetDirection();
	void SetRandomTimerTime();
	void onStartedDriving(std::string eventName);
	void onMatchQuit(std::string eventName);
	void onReset(std::string eventName);
	void Render(CanvasWrapper canvas);
private:
	float timeStart = 0.0f;
	float timeCurrent = 0.0f;
	float nextRandomTime = 0.0f;
	bool started = false;
	std::string currentDirection = "";
	std::vector<std::string> directions;
	void CheckAddDirection(std::string cvarString, std::string directionTextName, std::string directionSymbolName, bool useSymbols);
	void SetTextColors(CanvasWrapper canvas);
	void SetBoolCvarSettings(std::string cvarString, std::string descriptionText, std::string toolTipText);
	void SetSliderFloatCvarSettings(std::string cvarString, float min, float max, std::string descriptionText, std::string toolTipText1, std::string toolTipText2);
	int32_t RandRange(int32_t min, int32_t max);
public:
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
