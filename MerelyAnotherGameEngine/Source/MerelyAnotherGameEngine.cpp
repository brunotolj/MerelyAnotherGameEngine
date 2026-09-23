#include "Assets/FontFactory.h"
#include "Assets/TextureFactory.h"
#include "Assets/WorldSetupFactory.h"
#include "Engine/Engine.h"
#include "Framework/GameWorld.h"
#include "Framework/Systems/MeshRenderSystem.h"
#include "Framework/Systems/FreeMoveSystem.h"
#include "Framework/Systems/PhysicsSystem.h"
#include "Framework/Systems/SpriteRenderSystem.h"
#include "Framework/Systems/TextRenderSystem.h"
#include "Framework/Systems/WorldBoundsSystem.h"
#include "Game/GameplayEntities.h"
#include "Game/GameplaySystem.h"
#include "Vulkan/Device.h"
#include "Vulkan/Renderer.h"

#include <GLFW/glfw3.h>

#include <chrono>

static constexpr i32 gWindowWidth = 1920;
static constexpr i32 gWindowHeight = 1080;

void CreateControllableCamera(GameWorld& inWorld, mage::Transform const& inTransform)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	inWorld.GetComponent<FreeMoveSystem>()->SetTargetTransformId(transformEntity->mTransformId);
	inWorld.GetComponent<MeshRenderSystem>()->SetCameraTransformId(transformEntity->mTransformId);
}

void CreateLevelObject(
	GameWorld& inWorld,
	mage::Transform const& inTransform,
	AssetHandle<StaticMesh> inMesh,
	AssetHandle<Texture> inTexture,
	AssetHandle<PhysicsShape> inShape,
	AssetHandle<PhysicsMaterial> inMaterial)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	inWorld.CreateEntity<StaticRigidBodyEntity>(*transformEntity, inShape, inMaterial);
	inWorld.CreateEntity<StaticMeshEntity>(*transformEntity, inMesh, inTexture);
}

void CreateBallSpawnPoint(GameWorld& inWorld, mage::Transform const& inTransform, glm::vec3 inVelocity, glm::vec3 inVelocityVariance)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	inWorld.CreateEntity<BallSpawnerEntity>(*transformEntity, inVelocity, inVelocityVariance);
}

void CreateCapsule(GameWorld& inWorld, u32 inPlayerIndex, const mage::Transform& inTransform, AssetHandle<PhysicsShape> inPhysicsShape,
	AssetHandle<PhysicsMaterial> inPhysicsMaterial, AssetHandle<StaticMesh> inMesh, AssetHandle<Texture> inTexture, PlayerEntity* inPlayer)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	inWorld.CreateEntity<DynamicRigidBodyEntity>(*transformEntity, inPhysicsShape, inPhysicsMaterial, true);
	inWorld.CreateEntity<StaticMeshEntity>(*transformEntity, inMesh, inTexture);
	inPlayer->SetTransformId(transformEntity->mTransformId);
}

void CreateUserInterface(GameWorld& inWorld, AssetHandle<Texture> inTexture, AssetHandle<Font> inFontA, AssetHandle<Font> inFontB)
{
	inWorld.CreateEntity<SpriteEntity>(nullptr, glm::vec2(50.0f), glm::vec2(150.0f), glm::vec2(0.0f), glm::vec2(1.0f), inTexture);
	inWorld.CreateEntity<TextEntity>(nullptr, "Merely Another Game Engine", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), glm::vec2(180.0f, 90.0f), 40.0f, inFontA);
	inWorld.CreateEntity<TextEntity>(nullptr, "M.A.G.E", glm::vec4(0.5f, 1.0f, 1.0f, 1.0f), glm::vec2(180.0f, 140.0f), 40.0f, inFontB);
}

i32 main()
{
	Engine engine;

	WindowInfo windowInfo
	{
		.Name = "Merely Another Game Engine",
		.Width = gWindowWidth,
		.Height = gWindowHeight,
		.CursorMode = CursorInputMode::Disabled
	};

	WindowHandle window = engine.mWindowManager.CreateWindow(windowInfo);

	constexpr f32 boardSize = 20.0f;

	constexpr f32 cornerHalfHeight = 3.0f;
	constexpr f32 cornerRadius = 4.0f;
	constexpr f32 cornerPosition = boardSize - cornerRadius;

	constexpr f32 capsuleDistance = cornerPosition + 4.0f;
	constexpr f32 capsuleElevation = 2.0f;
	constexpr f32 capsuleRadius = 2.0f;
	constexpr f32 capsuleLength = 1.5f;

	constexpr f32 coneHeight = 8.0f;
	constexpr f32 coneRadius = 5.0f;

	constexpr f32 ballRadius = 1.0f;

	AssetHandle<Texture> spriteTexture = Factory<Texture>::FromFile("Textures/default.png");
	AssetHandle<Texture> cubeTexture = Factory<Texture>::FromFile("Textures/cube.png");
	AssetHandle<Texture> ballTexture = Factory<Texture>::FromFile("Textures/ball.png");
	AssetHandle<Texture> cylinderTexture = Factory<Texture>::FromFile("Textures/cylinder.png");
	AssetHandle<Texture> capsuleTexture = Factory<Texture>::FromFile("Textures/capsule.png");
	AssetHandle<Texture> coneTexture = Factory<Texture>::FromFile("Textures/cone.png");

	AssetHandle<Font> fontArianaVioleta = Factory<Font>::FromFile("Fonts/ArianaVioleta-dz2K.ttf");
	AssetHandle<Font> fontOrbitron = Factory<Font>::FromFile("Fonts/Orbitron-Regular.ttf");

	AssetHandle<WorldSetup> worldSetup = Factory<WorldSetup>::FromFile("Worlds/TestWorld.mage");

	AssetHandle<StaticMesh> boxMesh = engine.mAssetManager.GetHandle<StaticMesh>("Worlds/TestWorld.mage:BoxMesh");
	AssetHandle<StaticMesh> cylinderMesh = engine.mAssetManager.GetHandle<StaticMesh>("Worlds/TestWorld.mage:CylinderMesh");
	AssetHandle<StaticMesh> capsuleMesh = engine.mAssetManager.GetHandle<StaticMesh>("Worlds/TestWorld.mage:CapsuleMesh");
	AssetHandle<StaticMesh> ballMesh = engine.mAssetManager.GetHandle<StaticMesh>("Worlds/TestWorld.mage:BallMesh");

	AssetHandle<PhysicsShape> boardCollision = engine.mAssetManager.GetHandle<PhysicsShape>("Worlds/TestWorld.mage:BoardCollision");
	AssetHandle<PhysicsShape> cornerCollision = engine.mAssetManager.GetHandle<PhysicsShape>("Worlds/TestWorld.mage:CornerCollision");
	AssetHandle<PhysicsShape> capsuleCollision = engine.mAssetManager.GetHandle<PhysicsShape>("Worlds/TestWorld.mage:CapsuleCollision");
	AssetHandle<PhysicsShape> ballCollision = engine.mAssetManager.GetHandle<PhysicsShape>("Worlds/TestWorld.mage:BallCollision");

	AssetHandle<PhysicsMaterial> defaultMaterial = engine.mAssetManager.GetHandle<PhysicsMaterial>("Worlds/TestWorld.mage:DefaultMaterial");
	AssetHandle<PhysicsMaterial> floorMaterial = engine.mAssetManager.GetHandle<PhysicsMaterial>("Worlds/TestWorld.mage:FloorMaterial");
	AssetHandle<PhysicsMaterial> ballMaterial = engine.mAssetManager.GetHandle<PhysicsMaterial>("Worlds/TestWorld.mage:BallMaterial");

	GameWorld world;
	world.CreateComponent<TransformTree>();
	world.CreateComponent<Vulkan::Renderer>();
	world.CreateComponent<FreeMoveSystem>(10.0f);
	world.CreateComponent<GameplaySystem>(GameplaySystemSetup{ 10.0f, 80.0f, 80.0f, 150.0f, 2.0f, ballCollision, ballMaterial, ballMesh, ballTexture });
	world.CreateComponent<PhysicsSystem>();
	world.CreateComponent<WorldBoundsSystem>(glm::vec3(-10000.0f, -10000.0f, -10.0f), glm::vec3(10000.0f, 10000.0f, 10000.0f));
	world.CreateComponent<MeshRenderSystem>();
	world.CreateComponent<SpriteRenderSystem>();
	world.CreateComponent<TextRenderSystem>();

	{
		mage::Array<PlayerEntity*> players;
		players.Add(world.CreateEntity<PlayerEntity>(GLFW_KEY_H, GLFW_KEY_J));
		players.Add(world.CreateEntity<PlayerEntity>(GLFW_KEY_U, GLFW_KEY_I));
		players.Add(world.CreateEntity<PlayerEntity>(GLFW_KEY_O, GLFW_KEY_P));
		players.Add(world.CreateEntity<PlayerEntity>(GLFW_KEY_K, GLFW_KEY_L));

		CreateUserInterface(world, spriteTexture, fontArianaVioleta, fontOrbitron);

		mage::Transform transform;
		CreateLevelObject(world, transform, boxMesh, cubeTexture, boardCollision, floorMaterial);

		transform.Position = glm::vec3(0.0f, -30.0f, 10.0f);
		CreateControllableCamera(world, transform);
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(90.0f));
		
		transform.Position = glm::vec3(cornerPosition, cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderMesh, cylinderTexture, cornerCollision, defaultMaterial);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, -10.0f, 10.0f }, { 5.0f, 5.0f, 0.0f });

		transform.Position = glm::vec3(-cornerPosition, cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderMesh, cylinderTexture, cornerCollision, defaultMaterial);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, -10.0f, -10.0f }, { 5.0f, 5.0f, 0.0f });

		transform.Position = glm::vec3(-cornerPosition, -cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderMesh, cylinderTexture, cornerCollision, defaultMaterial);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, 10.0f, -10.0f }, { 5.0f, 5.0f, 0.0f });

		transform.Position = glm::vec3(cornerPosition, -cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderMesh, cylinderTexture, cornerCollision, defaultMaterial);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, 10.0f, 10.0f }, { 5.0f, 5.0f, 0.0f });

		transform.Rotation = {};
		transform.Position = glm::vec3(-capsuleDistance, 0.0f, capsuleElevation);
		CreateCapsule(world, 0, transform, capsuleCollision, defaultMaterial, capsuleMesh, capsuleTexture, players[0]);

		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(90.0f));
		transform.Position = glm::vec3(0.0f, capsuleDistance, capsuleElevation);
		CreateCapsule(world, 1, transform, capsuleCollision, defaultMaterial, capsuleMesh, capsuleTexture, players[1]);

		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(180.0f));
		transform.Position = glm::vec3(capsuleDistance, 0.0f, capsuleElevation);
		CreateCapsule(world, 2, transform, capsuleCollision, defaultMaterial, capsuleMesh, capsuleTexture, players[2]);

		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(-90.0f));
		transform.Position = glm::vec3(0.0f, -capsuleDistance, capsuleElevation);
		CreateCapsule(world, 3, transform, capsuleCollision, defaultMaterial, capsuleMesh, capsuleTexture, players[3]);
	}

	std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

	engine.mInputHandler.BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS, [&window]() { window.SetCursorInputMode(CursorInputMode::Normal); });
	engine.mInputHandler.BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE, [&window]() { window.SetCursorInputMode(CursorInputMode::Disabled); });
	engine.mInputHandler.BindKeyInputHandler(GLFW_KEY_ESCAPE, GLFW_PRESS, [&engine]() { engine.RequestExit(); });

	while (!engine.ShouldExit())
	{
		std::chrono::steady_clock::time_point newTime = std::chrono::high_resolution_clock::now();
		f32 frameTime = std::chrono::duration<f32, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		world.Update(frameTime);
		engine.mWindowManager.PollEvents();
	}

	engine.mVulkanDevice.GetVkDevice().waitIdle();

	return 0;
}
