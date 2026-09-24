#include "Assets/FontFactory.h"
#include "Assets/TextureFactory.h"
#include "Assets/WorldSetupFactory.h"
#include "Engine/Engine.h"
#include "Framework/GameWorld.h"

#include <GLFW/glfw3.h>

#include <chrono>

i32 main()
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

	AssetHandle<Texture> spriteTexture = Factory<Texture>::FromFile("Textures/default.png");
	AssetHandle<Texture> cubeTexture = Factory<Texture>::FromFile("Textures/cube.png");
	AssetHandle<Texture> ballTexture = Factory<Texture>::FromFile("Textures/ball.png");
	AssetHandle<Texture> cylinderTexture = Factory<Texture>::FromFile("Textures/cylinder.png");
	AssetHandle<Texture> capsuleTexture = Factory<Texture>::FromFile("Textures/capsule.png");
	AssetHandle<Texture> coneTexture = Factory<Texture>::FromFile("Textures/cone.png");

	AssetHandle<Font> fontArianaVioleta = Factory<Font>::FromFile("Fonts/ArianaVioleta-dz2K.ttf");
	AssetHandle<Font> fontOrbitron = Factory<Font>::FromFile("Fonts/Orbitron-Regular.ttf");

	AssetHandle<WorldSetup> worldSetup = Factory<WorldSetup>::FromFile("Worlds/TestWorld.mage");

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

	return 0;
}
