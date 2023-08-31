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

	bool isPluginEnabled = false;
	bool shouldUseSymbols = true;
	int currentSeed = 0;
	float currentMinTimer = 0.0f;
	float currentMaxTimer = 0.0f;
	float currentTimerPosX = 0.0f;
	float currentTimerPosY = 0.0f;
	float currentTimerScale = 0.0f;
	float currentTimerColorR = 0.0f;
	float currentTimerColorG = 0.0f;
	float currentTimerColorB = 0.0f;
	float currentTimerFadeA = 0.0f;
	float currentTimerFadeTime = 0.0f;

	std::string currentDirection = "";
	std::vector<std::string> directions;
	std::vector<int> directionWeights;
	std::mt19937 generator;

	void ResetValues();
	void ResetSeed();
	void CheckAddDirection(std::string cvarString, std::string directionTextName, std::string directionSymbolName, bool useSymbols);
	void SetInitialDirections();
	int GetRandomDirectionFromWeights();
	void UpdateDirectionWeights(int inSelected);
	void SetInitialWeights();
	void SetTextColors(CanvasWrapper canvas);
	void SetBoolCvarSettings(std::string cvarString, std::string descriptionText, std::string toolTipText);
	void SetSliderFloatCvarSettings(std::string cvarString, float min, float max, std::string descriptionText, std::string toolTipText1, std::string toolTipText2);
	void SetSliderIntCvarSettings(std::string cvarString, int min, int max, std::string descriptionText, std::string toolTipText1, std::string toolTipText2);
	int32_t RandRange(int32_t min, int32_t max, int inRandomSeed);
	int32_t RandRange(int32_t min, int32_t max);
public:
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
