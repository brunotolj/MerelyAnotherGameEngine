#include "Assets/WorldSetup.h"
#include "Engine/Engine.h"
#include "Framework/GameWorld.h"

#include <GLFW/glfw3.h>

#include <chrono>

i32 main(i32 argc, cstr argv[])
{
	Engine engine;

	WindowInfo windowInfo
	{
		.Name = "Merely Another Game Engine",
		.Width = 1920,
		.Height = 1080,
		.CursorMode = CursorInputMode::Disabled
	};

	WindowHandle window = engine.mWindowManager.CreateWindow(windowInfo);

	engine.mInputHandler.BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS, [&window]() { window.SetCursorInputMode(CursorInputMode::Normal); });
	engine.mInputHandler.BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE, [&window]() { window.SetCursorInputMode(CursorInputMode::Disabled); });
	engine.mInputHandler.BindKeyInputHandler(GLFW_KEY_ESCAPE, GLFW_PRESS, [&engine]() { engine.RequestExit(); });

	if (argc > 1)
	{
		AssetHandle<WorldSetup> worldSetup = Factory<WorldSetup>::FromFile(argv[1]);

		GameWorld world(*worldSetup.GetAsset());

		std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

		while (!engine.ShouldExit())
		{
			std::chrono::steady_clock::time_point newTime = std::chrono::high_resolution_clock::now();
			f32 frameTime = std::chrono::duration<f32, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			world.Update(frameTime);
			engine.mWindowManager.PollEvents();
		}
	}

	return 0;
}
