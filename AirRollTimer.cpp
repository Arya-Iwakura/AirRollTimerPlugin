#include "pch.h"
#include "AirRollTimer.h"

BAKKESMOD_PLUGIN(AirRollTimer, "Air Roll Timer", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void AirRollTimer::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("airrolltimer_plugin_enabled", "0", "Enable plugin", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_toggle_dirfront", "0", "Include front instruction", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_toggle_dirback", "0", "Include back instruction", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_toggle_dirleft", "0", "Include left instruction", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_toggle_dirright", "0", "Include right instruction", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_toggle_dirrollleft", "0", "Include air roll left instruction", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_toggle_dirrollright", "0", "Include aír roll right instruction", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_toggle_usesymbols", "0", "Use symbols instead of text instructions", true, true, 0, true, 1);
	cvarManager->registerCvar("airrolltimer_timer_min", "5", "Min timer time", true, true, 1.0f, true, 120.0f);
	cvarManager->registerCvar("airrolltimer_timer_max", "10", "Max timer time", true, true, 1.0f, true, 120.0f);
	cvarManager->registerCvar("airrolltimer_timer_posx", "0.5", "Timer position X");
	cvarManager->registerCvar("airrolltimer_timer_posy", "0.1", "Timer position Y");
	cvarManager->registerCvar("airrolltimer_timer_scale", "20.0", "Timer text scale");
	cvarManager->registerCvar("airrolltimer_timer_colorr", "255", "Timer color R");
	cvarManager->registerCvar("airrolltimer_timer_colorg", "255", "Timer color G");
	cvarManager->registerCvar("airrolltimer_timer_colorb", "255", "Timer color B");
	cvarManager->registerCvar("airrolltimer_timer_fadea", "0", "Timer fade to alpha");
	cvarManager->registerCvar("airrolltimer_timer_fadetime", "2", "Alpha fade time", true, true, 1.0f, true, 120.0f);

	gameWrapper->HookEventPost("Function TAGame.Car_TA.SetVehicleInput", std::bind(&AirRollTimer::onStartedDriving, this, std::placeholders::_1));
	gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&AirRollTimer::onMatchQuit, this, std::placeholders::_1));
	gameWrapper->HookEventPost("Function Engine.Controller.Restart", std::bind(&AirRollTimer::onReset, this, std::placeholders::_1));

	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		Render(canvas);
		});

	currentDirection = (std::string)("");
}

void AirRollTimer::UpdateTimer()
{
	if (!gameWrapper->IsInFreeplay() || started == false)
	{
		return;
	}

	CVarWrapper enabledCvar = cvarManager->getCvar("airrolltimer_plugin_enabled");
	bool isEnabled = enabledCvar.getBoolValue();
	if (isEnabled == false)
	{
		return;
	}

	timeCurrent = gameWrapper->GetGameEventAsServer().GetSecondsElapsed() - timeStart;
}

void AirRollTimer::SetRandomTimerTime()
{
	CVarWrapper timerMinCvar = cvarManager->getCvar("airrolltimer_timer_min");
	if (!timerMinCvar) { return; }
	float timerMin = timerMinCvar.getFloatValue();

	CVarWrapper timerMaxCvar = cvarManager->getCvar("airrolltimer_timer_max");
	if (!timerMaxCvar) { return; }
	float timerMax = timerMaxCvar.getFloatValue();

	if (timerMax < timerMin)
	{
		timerMax = timerMin; //cap the timer max to the timer min value if it is lower than that value
	}

	nextRandomTime = (float)RandRange((int32_t)timerMin, (int32_t)timerMax);
}

void AirRollTimer::GetDirection()
{
	if (!gameWrapper->IsInFreeplay() || started == false)
	{
		return;
	}

	directions.clear();

	CVarWrapper useSymbolsCvar = cvarManager->getCvar("airrolltimer_toggle_usesymbols");
	if (!useSymbolsCvar) { return; }
	bool useSymbols = useSymbolsCvar.getBoolValue();

	CheckAddDirection("airrolltimer_toggle_dirfront", "Front", "^", useSymbols);
	CheckAddDirection("airrolltimer_toggle_dirback", "Back", "V", useSymbols);
	CheckAddDirection("airrolltimer_toggle_dirleft", "Left", "<-", useSymbols);
	CheckAddDirection("airrolltimer_toggle_dirright", "Right", "->", useSymbols);
	CheckAddDirection("airrolltimer_toggle_dirrollleft", "Roll Left", "<<", useSymbols);
	CheckAddDirection("airrolltimer_toggle_dirrollright", "Roll Right", ">>", useSymbols);

	if (directions.size() < 1)
	{
		currentDirection = "Any";
		return;
	}

	int32_t randNumber = RandRange((int32_t)0, (int32_t)directions.size()-1);
	currentDirection = (std::string)(directions[randNumber]);

	directions.clear();
}

void AirRollTimer::CheckAddDirection(std::string cvarString, std::string directionTextName, std::string directionSymbolName, bool useSymbols)
{
	CVarWrapper dirCvar = cvarManager->getCvar(cvarString);
	bool boolValue = dirCvar.getBoolValue();
	if (boolValue == true && currentDirection != directionTextName && currentDirection != directionSymbolName)
	{
		if (useSymbols)
		{
			directions.push_back(directionSymbolName);
		}
		else
		{
			directions.push_back(directionTextName);
		}
	}
}

void AirRollTimer::onStartedDriving(std::string eventName)
{
	if (!gameWrapper->IsInFreeplay())
	{
		started = false;
		return;
	}

	if (started == true)
	{
		return;
	}

	CVarWrapper enabledCvar = cvarManager->getCvar("airrolltimer_plugin_enabled");
	bool isEnabled = enabledCvar.getBoolValue();
	if (isEnabled == false)
	{
		return;
	}


	CarWrapper car = gameWrapper->GetLocalCar();

	if (!car.IsNull()) {
		ControllerInput controllerInput = car.GetInput();
		if (controllerInput.Throttle > 0 || controllerInput.ActivateBoost > 0) {
			started = true;
			timeStart = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
			timeCurrent = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
			SetRandomTimerTime();
			GetDirection();
		}
	}
}

void AirRollTimer::onMatchQuit(std::string eventName)
{
	started = false;
	CVarWrapper useSymbolsCvar = cvarManager->getCvar("airrolltimer_toggle_usesymbols");
	if (!useSymbolsCvar) { return; }
	bool useSymbols = useSymbolsCvar.getBoolValue();
	
	if (useSymbols == true)
	{
		currentDirection = "-";
	}
	else
	{
		currentDirection = "Waiting";
	}
}

void AirRollTimer::onReset(std::string eventName)
{
	if (!gameWrapper->IsInFreeplay())
		return;
	
	started = false;
	
	CVarWrapper useSymbolsCvar = cvarManager->getCvar("airrolltimer_toggle_usesymbols");
	if (!useSymbolsCvar) { return; }
	bool useSymbols = useSymbolsCvar.getBoolValue();

	if (useSymbols == true)
	{
		currentDirection = "-";
	}
	else
	{
		currentDirection = "Waiting";
	}
}

void AirRollTimer::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInFreeplay())
	{
		return;
	}

	CVarWrapper enabledCvar = cvarManager->getCvar("airrolltimer_plugin_enabled");
	bool isEnabled = enabledCvar.getBoolValue();
	if (isEnabled == false)
	{
		return;
	}

	if (started)
	{
		UpdateTimer();

		if (timeCurrent > nextRandomTime)
		{
			SetRandomTimerTime();
			timeStart = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
			GetDirection();
		}
	}

	auto screenSize = canvas.GetSize();

	SetTextColors(canvas);

	CVarWrapper timePosXCvar = cvarManager->getCvar("airrolltimer_timer_posx");
	if (!timePosXCvar) { return; }
	float timerPosX = timePosXCvar.getFloatValue();

	CVarWrapper timePosYCvar = cvarManager->getCvar("airrolltimer_timer_posy");
	if (!timePosYCvar) { return; }
	float timerPosY = timePosYCvar.getFloatValue();

	CVarWrapper timerScaleCvar = cvarManager->getCvar("airrolltimer_timer_scale");
	if (!timerScaleCvar) { return; }
	float textScale = timerScaleCvar.getFloatValue();

	Vector2F stringSize = canvas.GetStringSize(currentDirection, textScale, textScale);
	canvas.SetPosition(Vector2F{ (float)(screenSize.X * timerPosX) - (float)(stringSize.X * 0.5), (float)(screenSize.Y * timerPosY)});
	canvas.DrawString(currentDirection, textScale, textScale, false); //string, scale x, scale y, drop shadow
}

void AirRollTimer::SetTextColors(CanvasWrapper canvas)
{
	CVarWrapper timeColorRCvar = cvarManager->getCvar("airrolltimer_timer_colorr");
	if (!timeColorRCvar) { return; }
	float timerColorR = timeColorRCvar.getFloatValue();
	CVarWrapper timeColorGCvar = cvarManager->getCvar("airrolltimer_timer_colorg");
	if (!timeColorGCvar) { return; }
	float timerColorG = timeColorGCvar.getFloatValue();
	CVarWrapper timeColorBCvar = cvarManager->getCvar("airrolltimer_timer_colorb");
	if (!timeColorBCvar) { return; }
	float timerColorB = timeColorBCvar.getFloatValue();
	CVarWrapper timerFadeACvar = cvarManager->getCvar("airrolltimer_timer_fadea");
	if (!timerFadeACvar) { return; }
	float timerFadeA = (255.0f - timerFadeACvar.getFloatValue()) / 255.0f;

	LinearColor colors;
	colors.R = timerColorR;
	colors.G = timerColorG;
	colors.B = timerColorB;

	if (started)
	{
		CVarWrapper fadeTimerCvar = cvarManager->getCvar("airrolltimer_timer_fadetime");
		if (!fadeTimerCvar) { return; }
		float fadeTimer = fadeTimerCvar.getFloatValue();

		CVarWrapper timerMaxCvar = cvarManager->getCvar("airrolltimer_timer_max");
		if (!timerMaxCvar) { return; }
		float timerMax = timerMaxCvar.getFloatValue();

		if (fadeTimer > timerMax)
		{
			fadeTimer = timerMax; //cap the fadeTimer to the timer max value if it is larger than that value
		}

		float timeNormalized = timeCurrent / fadeTimer; //get the normalized time value using fadeTimer
		float timeCapped = std::min((timeNormalized), timerFadeA); //cap the normalized time between 0.0f and the cap value
		colors.A = 255.0f * (1.0f - timeCapped); //invert the time value such that the alpha fades from 1.0f to the cap value over time
	}
	else
	{
		colors.A = 255;
	}

	canvas.SetColor(colors);
}

void AirRollTimer::RenderSettings() 
{
	SetBoolCvarSettings("airrolltimer_plugin_enabled", "Enable plugin", "Toggle Air Roll Timer plugin");

	ImGui::TextUnformatted("General Settings");

	SetBoolCvarSettings("airrolltimer_toggle_usesymbols", "Use Symbols Instead of Text", "Use symbols instead of text instructions");

	ImGui::TextUnformatted("Direction Instructions To Include");

	SetBoolCvarSettings("airrolltimer_toggle_dirfront", "Include Front", "Include the fly forwards instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirback", "Include Back", "Include the fly backwards instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirleft", "Include Left", "Include the fly facing left instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirright", "Include Right", "Include the fly facing right instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirrollleft", "Include Roll Left", "Include the roll left instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirrollright", "Include Roll Right", "Include the roll right instruction");

	ImGui::TextUnformatted("Timer Settings");

	SetSliderFloatCvarSettings("airrolltimer_timer_min", 1.0f, 120.0f, "Min Timer Value", "Min timer is ", "");
	SetSliderFloatCvarSettings("airrolltimer_timer_max", 1.0f, 120.0f, "Max Timer Value", "Max timer is ", ". If lower than Min timer value this will be set to the Min value.");
	SetSliderFloatCvarSettings("airrolltimer_timer_fadetime", 1.0f, 120.0f, "Alpha Fade Timer Value", "Fade timer is ", ". If greater than Max timer value this will be set to the Max value.");
	SetSliderFloatCvarSettings("airrolltimer_timer_posx", 0.0f, 1.0f, "Screen Position X", "Timer X screen position percent ", "");
	SetSliderFloatCvarSettings("airrolltimer_timer_posy", 0.0f, 1.0f, "Screen Position Y", "Timer Y screen position percent ", "");

	ImGui::TextUnformatted("Timer Scale");

	SetSliderFloatCvarSettings("airrolltimer_timer_scale", 1.0f, 20.0f, "Text Scale", "Timer text scale is ", "");

	ImGui::TextUnformatted("Timer Color RGBA");

	SetSliderFloatCvarSettings("airrolltimer_timer_colorr", 0.0f, 255.0f, "Text Color R", "Timer text color R is ", "");
	SetSliderFloatCvarSettings("airrolltimer_timer_colorg", 0.0f, 255.0f, "Text Color G", "Timer text color G is ", "");
	SetSliderFloatCvarSettings("airrolltimer_timer_colorb", 0.0f, 255.0f, "Text Color B", "Timer text color B is ", "");
	SetSliderFloatCvarSettings("airrolltimer_timer_fadea", 0.0f, 255.0f, "Text Fade Alpha", "Timer fade to alpha time is ", "");
}

void AirRollTimer::SetBoolCvarSettings(std::string cvarString, std::string descriptionText, std::string toolTipText)
{
	CVarWrapper cvar = cvarManager->getCvar(cvarString);
	if (!cvar) { return; }
	bool boolValue = cvar.getBoolValue();
	if (ImGui::Checkbox(descriptionText.c_str(), &boolValue)) {
		cvar.setValue(boolValue);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(toolTipText.c_str());
	}
}

void AirRollTimer::SetSliderFloatCvarSettings(std::string cvarString, float min, float max, std::string descriptionText, std::string toolTipText1, std::string toolTipText2)
{
	CVarWrapper cvar = cvarManager->getCvar(cvarString);
	if (!cvar) { return; }
	float floatValue = cvar.getFloatValue();
	if (ImGui::SliderFloat(descriptionText.c_str(), &floatValue, min, max)) {
		cvar.setValue(floatValue);
	}
	if (ImGui::IsItemHovered()) {
		std::string hoverText = toolTipText1 + std::to_string(floatValue) + toolTipText2;
		ImGui::SetTooltip(hoverText.c_str());
	}
}

int32_t AirRollTimer::RandRange(int32_t min, int32_t max)
{
	std::random_device randDevice;
	std::mt19937 gen(randDevice());
	std::uniform_int_distribution<> distrib(min, max);
	return distrib(gen);
}
