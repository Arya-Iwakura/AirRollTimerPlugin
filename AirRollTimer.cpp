#include "pch.h"
#include "AirRollTimer.h"

BAKKESMOD_PLUGIN(AirRollTimer, "Air Roll Timer", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void AirRollTimer::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("airrolltimer_plugin_enabled", "0", "Enable plugin", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			isPluginEnabled = cvar.getBoolValue();
			ResetValues();
			ResetSeed();
		});
	cvarManager->registerCvar("airrolltimer_toggle_dirfront", "1", "Include front instruction", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			SetInitialDirections();
			SetInitialWeights();
		});
	cvarManager->registerCvar("airrolltimer_toggle_dirback", "1", "Include back instruction", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			SetInitialDirections();
			SetInitialWeights();
		});
	cvarManager->registerCvar("airrolltimer_toggle_dirleft", "0", "Include left instruction", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			SetInitialDirections();
			SetInitialWeights();
		});
	cvarManager->registerCvar("airrolltimer_toggle_dirright", "0", "Include right instruction", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			SetInitialDirections();
			SetInitialWeights();
		});
	cvarManager->registerCvar("airrolltimer_toggle_dirrollleft", "0", "Include air roll left instruction", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			SetInitialDirections();
			SetInitialWeights();
		});
	cvarManager->registerCvar("airrolltimer_toggle_dirrollright", "0", "Include aír roll right instruction", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			SetInitialDirections();
			SetInitialWeights();
		});
	cvarManager->registerCvar("airrolltimer_toggle_usesymbols", "1", "Use symbols instead of text instructions", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			shouldUseSymbols = cvar.getBoolValue();
			SetInitialDirections();
			SetInitialWeights();
		});
	cvarManager->registerCvar("airrolltimer_timer_min", "5", "Min timer time", true, true, 1.0f, true, 120.0f)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentMinTimer = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_max", "15", "Max timer time", true, true, 1.0f, true, 120.0f)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentMaxTimer = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_posx", "0.5", "Timer position X")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerPosX = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_posy", "0.1", "Timer position Y")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerPosY = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_scale", "20.0", "Timer text scale")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerScale = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_colorr", "255", "Timer color R")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerColorR = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_colorg", "255", "Timer color G")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerColorG = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_colorb", "255", "Timer color B")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerColorB = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_fadea", "0", "Timer fade to alpha")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerFadeA = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_timer_fadetime", "1.5", "Alpha fade time", true, true, 1.0f, true, 120.0f)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentTimerFadeTime = cvar.getFloatValue();
		});
	cvarManager->registerCvar("airrolltimer_random_seed", "0", "Random seed", true, true, 0, true, 10000)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			currentSeed = cvar.getIntValue();
			ResetSeed();
		});

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

	if (isPluginEnabled == false)
	{
		return;
	}

	timeCurrent = gameWrapper->GetGameEventAsServer().GetSecondsElapsed() - timeStart;
}

void AirRollTimer::SetRandomTimerTime()
{
	if (currentMaxTimer < currentMinTimer)
	{
		currentMaxTimer = currentMinTimer; //cap the timer max to the timer min value if it is lower than that value
	}

	nextRandomTime = (float)RandRange((int32_t)currentMinTimer, (int32_t)currentMaxTimer, currentSeed);
}

void AirRollTimer::GetDirection()
{
	if (!gameWrapper->IsInFreeplay() || started == false)
	{
		return;
	}

	if (directions.size() < 1)
	{
		currentDirection = "?";
		return;
	}

	int randDirectionNum = GetRandomDirectionFromWeights();
	UpdateDirectionWeights(randDirectionNum);
	currentDirection = (std::string)(directions[randDirectionNum]);
}

void AirRollTimer::CheckAddDirection(std::string cvarString, std::string directionTextName, std::string directionSymbolName, bool useSymbols)
{
	CVarWrapper dirCvar = cvarManager->getCvar(cvarString);
	bool boolValue = dirCvar.getBoolValue();
	if (boolValue == true)
	{
		if (shouldUseSymbols)
		{
			directions.push_back(directionSymbolName);
		}
		else
		{
			directions.push_back(directionTextName);
		}
	}
}

void AirRollTimer::SetInitialDirections()
{
	if (!gameWrapper->IsInFreeplay() || started == false)
	{
		return;
	}

	directions.clear();

	CheckAddDirection("airrolltimer_toggle_dirfront", "Front", "^", shouldUseSymbols);
	CheckAddDirection("airrolltimer_toggle_dirback", "Back", "V", shouldUseSymbols);
	CheckAddDirection("airrolltimer_toggle_dirleft", "Left", "<-", shouldUseSymbols);
	CheckAddDirection("airrolltimer_toggle_dirright", "Right", "->", shouldUseSymbols);
	CheckAddDirection("airrolltimer_toggle_dirrollleft", "Roll Left", "<<", shouldUseSymbols);
	CheckAddDirection("airrolltimer_toggle_dirrollright", "Roll Right", ">>", shouldUseSymbols);
}

int AirRollTimer::GetRandomDirectionFromWeights()
{
	int weightSum = 0;

	for (int i = 0; i < directionWeights.size(); i++)
	{
		weightSum += directionWeights[i];
	}

	int32_t randNumber = RandRange((int32_t)0, weightSum-1, currentSeed);

	for (int i = 0; i < directionWeights.size(); i++)
	{
		if (randNumber < directionWeights[i])
		{
			return i;
		}
		randNumber -= directionWeights[i];
	}

	return 0;
}

void AirRollTimer::UpdateDirectionWeights(int inSelected)
{
	int weightIncreaseAmount = 1;
	int maxWeight = 1024;

	for (int i = 0; i < directionWeights.size(); i++)
	{
		directionWeights[i] += weightIncreaseAmount;
		if (directionWeights[i] > maxWeight)
		{
			if (i < directionWeights.size())
			{
				directionWeights[i] = maxWeight;
			}
		}
	}

	directionWeights[inSelected] = 0;
}

void AirRollTimer::SetInitialWeights()
{
	directionWeights.clear();

	for (int i = 0; i < directions.size(); i++)
	{
		directionWeights.push_back(1);
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

	if (isPluginEnabled == false)
	{
		return;
	}

	CarWrapper car = gameWrapper->GetLocalCar();

	if (!car.IsNull()) {
		ControllerInput controllerInput = car.GetInput();
		if (controllerInput.Throttle > 0 || controllerInput.ActivateBoost > 0) {
			ResetValues();
			started = true;
			timeStart = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
			timeCurrent = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
			SetRandomTimerTime();
			SetInitialDirections();
			SetInitialWeights();
			GetDirection();
		}
	}
}

void AirRollTimer::onMatchQuit(std::string eventName)
{
	ResetValues();
	ResetSeed();
}

void AirRollTimer::onReset(std::string eventName)
{
	if (!gameWrapper->IsInFreeplay())
	{
		return;
	}

	ResetValues();
}

void AirRollTimer::ResetValues()
{
	started = false;

	if (shouldUseSymbols == true)
	{
		currentDirection = "-";
	}
	else
	{
		currentDirection = "Waiting";
	}
}

void AirRollTimer::ResetSeed()
{
	if (currentSeed != 0)
	{
		generator.seed(currentSeed);
	}
}

void AirRollTimer::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInFreeplay())
	{
		return;
	}

	if (isPluginEnabled == false)
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

	Vector2F stringSize = canvas.GetStringSize(currentDirection, currentTimerScale, currentTimerScale);
	canvas.SetPosition(Vector2F{ (float)(screenSize.X * currentTimerPosX) - (float)(stringSize.X * 0.5), (float)(screenSize.Y * currentTimerPosY)});
	canvas.DrawString(currentDirection, currentTimerScale, currentTimerScale, false); //string, scale x, scale y, drop shadow
}

void AirRollTimer::SetTextColors(CanvasWrapper canvas)
{
	LinearColor colors;
	colors.R = currentTimerColorR;
	colors.G = currentTimerColorG;
	colors.B = currentTimerColorB;

	if (started)
	{
		if (currentTimerFadeTime > currentMaxTimer)
		{
			currentTimerFadeTime = currentMaxTimer; //cap the fadeTimer to the timer max value if it is larger than that value
		}

		float timeNormalized = timeCurrent / currentTimerFadeTime; //get the normalized time value using fadeTimer
		float timeCapped = std::min((timeNormalized), currentTimerFadeA); //cap the normalized time between 0.0f and the cap value
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

	SetBoolCvarSettings("airrolltimer_toggle_dirfront", "Include Front ( ^ )", "Include the fly forwards instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirback", "Include Back ( v )", "Include the fly backwards instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirleft", "Include Left ( <- )", "Include the fly facing left instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirright", "Include Right ( -> )", "Include the fly facing right instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirrollleft", "Include Roll Left ( << )", "Include the roll left instruction");
	SetBoolCvarSettings("airrolltimer_toggle_dirrollright", "Include Roll Right ( >> )", "Include the roll right instruction");

	ImGui::TextUnformatted("Timer Settings");

	SetSliderFloatCvarSettings("airrolltimer_timer_min", 1.0f, 120.0f, "Min Timer Value", "Min timer is ", "");
	SetSliderFloatCvarSettings("airrolltimer_timer_max", 1.0f, 120.0f, "Max Timer Value", "Max timer is ", ". If lower than Min timer value this will be set to the Min value.");
	SetSliderFloatCvarSettings("airrolltimer_timer_fadetime", 1.0f, 120.0f, "Alpha Fade Timer Value", "Fade timer is ", ". If greater than Max timer value this will be set to the Max value.");
	SetSliderFloatCvarSettings("airrolltimer_timer_posx", 0.0f, 1.0f, "Screen Position X", "Timer X screen position percent ", "");
	SetSliderFloatCvarSettings("airrolltimer_timer_posy", 0.0f, 1.0f, "Screen Position Y", "Timer Y screen position percent ", "");

	ImGui::TextUnformatted("Timer Scale");

	SetSliderFloatCvarSettings("airrolltimer_timer_scale", 1.0f, 20.0f, "Text Scale", "Timer text scale is ", "");

	ImGui::TextUnformatted("Timer Color RGBA");

	SetSliderIntCvarSettings("airrolltimer_timer_colorr", 0, 255, "Text Color R", "Timer text color R is ", "");
	SetSliderIntCvarSettings("airrolltimer_timer_colorg", 0, 255, "Text Color G", "Timer text color G is ", "");
	SetSliderIntCvarSettings("airrolltimer_timer_colorb", 0, 255, "Text Color B", "Timer text color B is ", "");
	SetSliderIntCvarSettings("airrolltimer_timer_fadea", 0, 255, "Text Fade Alpha", "Timer fade to alpha time is ", "");
	
	SetSliderIntCvarSettings("airrolltimer_random_seed", 0, 10000, "Random Seed Number", "Seed is ", ". Set to zero for a random seed.");
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

void AirRollTimer::SetSliderIntCvarSettings(std::string cvarString, int min, int max, std::string descriptionText, std::string toolTipText1, std::string toolTipText2)
{
	CVarWrapper cvar = cvarManager->getCvar(cvarString);
	if (!cvar) { return; }
	int intValue = cvar.getIntValue();
	if (ImGui::SliderInt(descriptionText.c_str(), &intValue, min, max)) {
		cvar.setValue(intValue);
	}
	if (ImGui::IsItemHovered()) {
		std::string hoverText = toolTipText1 + std::to_string(intValue) + toolTipText2;
		ImGui::SetTooltip(hoverText.c_str());
	}
}

int32_t AirRollTimer::RandRange(int32_t min, int32_t max, int inRandomSeed)
{
	if (inRandomSeed != 0)
	{
		std::uniform_int_distribution<> distrib(min, max);
		return distrib(generator);
	}
	
	return (RandRange(min, max));
}

int32_t AirRollTimer::RandRange(int32_t min, int32_t max)
{
	std::random_device randDevice;
	std::mt19937 gen(randDevice());
	std::uniform_int_distribution<> distrib(min, max);
	return distrib(gen);
}