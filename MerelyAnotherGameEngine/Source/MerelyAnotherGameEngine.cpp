#include "Assets/FontFactory.h"
#include "Assets/PhysicsMaterialFactory.h"
#include "Assets/PhysicsShapeFactory.h"
#include "Assets/StaticMeshFactory.h"
#include "Assets/TextureFactory.h"
#include "Engine/Engine.h"
#include "Framework/GameWorld.h"
#include "Framework/Systems/MeshRenderSystem.h"
#include "Framework/Systems/FreeMoveSystem.h"
#include "Framework/Systems/SpriteRenderSystem.h"
#include "Framework/Systems/TextRenderSystem.h"
#include "Framework/Systems/WorldBoundsSystem.h"
#include "Game/GameplaySystem.h"
#include "Physics/PhysicsSystem.h"
#include "Vulkan/Device.h"
#include "Vulkan/Renderer.h"

#include <GLFW/glfw3.h>

#include <chrono>

static constexpr i32 gWindowWidth = 1920;
static constexpr i32 gWindowHeight = 1080;

void CreateControllableCamera(GameWorld& inWorld, mage::Transform const& inTransform, f32 inSpeed)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	inWorld.GetComponent<FreeMoveSystem>()->Setup(transformEntity->mTransformId, inSpeed);
	inWorld.GetComponent<MeshRenderSystem>()->SetCameraTransformId(transformEntity->mTransformId);
}

void CreateLevelObject(
	GameWorld& inWorld,
	mage::Transform const& inTransform,
	PhysicsRigidBodyParams inRigidBodyParams,
	AssetHandle<StaticMesh> inMesh,
	AssetHandle<Texture> inTexture)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	RigidBodyEntity* rigidBodyEntity = inWorld.CreateEntity<RigidBodyEntity>(*transformEntity, inRigidBodyParams);
	StaticMeshEntity* staticMeshEntity = inWorld.CreateEntity<StaticMeshEntity>(*transformEntity, inMesh, inTexture);
}

void CreateBallSpawnPoint(GameWorld& inWorld, mage::Transform const& inTransform, glm::vec3 inVelocity, f32 inVelocityVariance)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	inWorld.GetComponent<GameplaySystem>()->AddBallSpawner(transformEntity->mTransformId, inVelocity, inVelocityVariance);
}

void CreateCapsule(GameWorld& inWorld, u32 inPlayerIndex, const mage::Transform& inTransform, PhysicsRigidBodyParams inRigidBodyParams,
	AssetHandle<StaticMesh> inMesh, AssetHandle<Texture> inTexture, i32 inInputNeg, i32 inInputPos)
{
	TransformEntity* transformEntity = inWorld.CreateEntity<TransformEntity>(nullptr, inTransform);
	RigidBodyEntity* rigidBodyEntity = inWorld.CreateEntity<RigidBodyEntity>(*transformEntity, inRigidBodyParams);
	StaticMeshEntity* staticMeshEntity = inWorld.CreateEntity<StaticMeshEntity>(*transformEntity, inMesh, inTexture);
	inWorld.GetComponent<GameplaySystem>()->SetupPlayer(inPlayerIndex, transformEntity->mTransformId, inInputNeg, inInputPos);
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

	AssetHandle<StaticMesh> boxMesh = Factory<StaticMesh>::MakeBox({ boardSize, boardSize, 1.0f });
	AssetHandle<StaticMesh> cylinderMesh = Factory<StaticMesh>::MakeCylinder(cornerRadius, cornerHalfHeight);
	AssetHandle<StaticMesh> capsuleMesh = Factory<StaticMesh>::MakeCapsule(capsuleRadius, capsuleLength);
	AssetHandle<StaticMesh> coneMesh = Factory<StaticMesh>::MakeCone(coneRadius, coneHeight);
	AssetHandle<StaticMesh> ballMesh = Factory<StaticMesh>::MakeBall(ballRadius);

	AssetHandle<Texture> spriteTexture = Factory<Texture>::FromFile("Textures/default.png");
	AssetHandle<Texture> cubeTexture = Factory<Texture>::FromFile("Textures/cube.png");
	AssetHandle<Texture> ballTexture = Factory<Texture>::FromFile("Textures/ball.png");
	AssetHandle<Texture> cylinderTexture = Factory<Texture>::FromFile("Textures/cylinder.png");
	AssetHandle<Texture> capsuleTexture = Factory<Texture>::FromFile("Textures/capsule.png");
	AssetHandle<Texture> coneTexture = Factory<Texture>::FromFile("Textures/cone.png");

	AssetHandle<Font> fontArianaVioleta = Factory<Font>::FromFile("Fonts/ArianaVioleta-dz2K.ttf");
	AssetHandle<Font> fontOrbitron = Factory<Font>::FromFile("Fonts/Orbitron-Regular.ttf");

	AssetHandle<PhysicsShape> boardCollision = Factory<PhysicsShape>::MakeBox({ boardSize, boardSize, 1.0f });
	AssetHandle<PhysicsShape> cornerCollision = Factory<PhysicsShape>::MakeCylinder(cornerRadius, 2.0f * cornerHalfHeight);
	AssetHandle<PhysicsShape> capsuleCollision = Factory<PhysicsShape>::MakeCapsule(capsuleRadius, capsuleLength);
	AssetHandle<PhysicsShape> coneCollision = Factory<PhysicsShape>::MakeCone(coneRadius, coneHeight);
	AssetHandle<PhysicsShape> ballCollision = Factory<PhysicsShape>::MakeSphere(ballRadius);

	AssetHandle<PhysicsMaterial> defaultMaterial = Factory<PhysicsMaterial>::Create(0.2f, 0.1f, 1.0f);
	AssetHandle<PhysicsMaterial> floorMaterial = Factory<PhysicsMaterial>::Create(0.2f, 0.05f, 0.0f);

	PhysicsRigidBodyParams boxRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, boardCollision, floorMaterial };
	PhysicsRigidBodyParams cylinderRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, cornerCollision, defaultMaterial };
	PhysicsRigidBodyParams capsuleRigidBodyParams = { PhysicsSystemObjectType::RigidKinematic, capsuleCollision, defaultMaterial };
	PhysicsRigidBodyParams coneRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, coneCollision, defaultMaterial };
	PhysicsRigidBodyParams ballRigidBodyParams = { PhysicsSystemObjectType::RigidDynamic, ballCollision, defaultMaterial };

	GameWorld world;
	world.CreateComponent<TransformTree>();
	world.CreateComponent<Vulkan::Renderer>(window);
	world.CreateComponent<FreeMoveSystem>();
	world.CreateComponent<GameplaySystem>(GameplaySystemSetup{ 10.0f, 80.0f, 80.0f, 150.0f, 2.0f, ballRigidBodyParams, ballMesh, ballTexture });
	world.CreateComponent<PhysicsSystem>();
	world.CreateComponent<WorldBoundsSystem>(glm::vec3(-10000.0f, -10000.0f, -10.0f), glm::vec3(10000.0f, 10000.0f, 10000.0f));
	world.CreateComponent<MeshRenderSystem>();
	world.CreateComponent<SpriteRenderSystem>();
	world.CreateComponent<TextRenderSystem>();

	{
		CreateUserInterface(world, spriteTexture, fontArianaVioleta, fontOrbitron);

		mage::Transform transform;
		CreateLevelObject(world, transform, boxRigidBodyParams, boxMesh, cubeTexture);

		transform.Position = glm::vec3(0.0f, -30.0f, 10.0f);
		CreateControllableCamera(world, transform, 10.0f);
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(90.0f));
		
		transform.Position = {};
		//CreateLevelObject(world, transform, coneRigidBodyParams, coneMesh, coneTexture);
		
		transform.Position = glm::vec3(cornerPosition, cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, -10.0f, 10.0f }, 5.0f);
		
		transform.Position = glm::vec3(-cornerPosition, cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, -10.0f, -10.0f }, 5.0f);

		transform.Position = glm::vec3(-cornerPosition, -cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, 10.0f, -10.0f }, 5.0f);

		transform.Position = glm::vec3(cornerPosition, -cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);

		transform.Position.z += cornerHalfHeight + 2.0f * ballRadius;
		CreateBallSpawnPoint(world, transform, { 0.0f, 10.0f, 10.0f }, 5.0f);

		transform.Rotation = {};
		transform.Position = glm::vec3(-capsuleDistance, 0.0f, capsuleElevation);
		CreateCapsule(world, 0, transform, capsuleRigidBodyParams, capsuleMesh, capsuleTexture, GLFW_KEY_H, GLFW_KEY_J);
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(90.0f));
		transform.Position = glm::vec3(0.0f, capsuleDistance, capsuleElevation);
		CreateCapsule(world, 1, transform, capsuleRigidBodyParams, capsuleMesh, capsuleTexture, GLFW_KEY_U, GLFW_KEY_I);
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(180.0f));
		transform.Position = glm::vec3(capsuleDistance, 0.0f, capsuleElevation);
		CreateCapsule(world, 2, transform, capsuleRigidBodyParams, capsuleMesh, capsuleTexture, GLFW_KEY_O, GLFW_KEY_P);
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(-90.0f));
		transform.Position = glm::vec3(0.0f, -capsuleDistance, capsuleElevation);
		CreateCapsule(world, 3, transform, capsuleRigidBodyParams, capsuleMesh, capsuleTexture, GLFW_KEY_K, GLFW_KEY_L);
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
