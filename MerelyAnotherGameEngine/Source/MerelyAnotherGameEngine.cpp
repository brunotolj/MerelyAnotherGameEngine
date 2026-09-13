#include "Assets/FontFactory.h"
#include "Assets/PhysicsMaterialFactory.h"
#include "Assets/PhysicsShapeFactory.h"
#include "Assets/StaticMeshFactory.h"
#include "Assets/TextureFactory.h"
#include "Engine/Engine.h"
#include "Framework/GameWorld.h"
#include "Framework/Systems/MeshRenderSystem.h"
#include "Framework/Systems/SpriteRenderSystem.h"
#include "Framework/Systems/TextRenderSystem.h"
#include "Framework/Systems/WorldBoundsSystem.h"
#include "Game/GameObject.h"
#include "Game/GameplaySystem.h"
#include "Game/RigidBodyObjectComponent.h"
#include "Game/SpriteObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Game/TextObjectComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Utility/BallSpawnerComponent.h"
#include "Utility/DefaultMovementComponent.h"
#include "Vulkan/Device.h"
#include "Vulkan/Renderer.h"

#include <chrono>

static constexpr i32 gWindowWidth = 1920;
static constexpr i32 gWindowHeight = 1080;

TransformableObject* CreateControllableCamera(
	GameWorld& world,
	const mage::Transform& transform,
	f32 speed,
	PhysicsRigidBodyParams ballRigidBodyParams,
	AssetHandle<StaticMesh> ballMesh,
	AssetHandle<Texture> ballTexture,
	f32 ballSpeed,
	i32 inputSpawnBall)
{
	ComponentTemplate<DefaultMovementComponent> movementTemplate;
	movementTemplate.Speed = speed;

	ComponentTemplate<BallSpawnerComponent> ballSpawnerTemplate;
	ballSpawnerTemplate.RigidBodyParams = ballRigidBodyParams;
	ballSpawnerTemplate.Mesh = ballMesh;
	ballSpawnerTemplate.Texture = ballTexture;
	ballSpawnerTemplate.Speed = ballSpeed;
	ballSpawnerTemplate.InputSpawn = inputSpawnBall;

	TransformableObject* camera = world.CreateObject<TransformableObject>(transform, movementTemplate, ballSpawnerTemplate);
	world.GetComponent<MeshRenderSystem>()->SetCameraTransformId(camera->GetTransformId());

	return camera;
}

TransformableObject* CreateLevelObject(
	GameWorld& world,
	const mage::Transform& transform,
	PhysicsRigidBodyParams rigidBodyParams,
	AssetHandle<StaticMesh> mesh,
	AssetHandle<Texture> texture)
{
	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = rigidBodyParams;

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Mesh = mesh;
	staticMeshTemplate.Texture = texture;

	return world.CreateObject<TransformableObject>(transform, rigidBodyTemplate, staticMeshTemplate);
}

TransformableObject* CreateCapsule(
	GameWorld& inWorld,
	u32 inPlayerIndex,
	const mage::Transform& inTransform,
	PhysicsRigidBodyParams inRigidBodyParams,
	AssetHandle<StaticMesh> inMesh,
	AssetHandle<Texture> inTexture,
	i32 inInputNeg,
	i32 inInputPos)
{
	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = inRigidBodyParams;

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Mesh = inMesh;
	staticMeshTemplate.Texture = inTexture;

	TransformableObject* capsule = inWorld.CreateObject<TransformableObject>(inTransform, rigidBodyTemplate, staticMeshTemplate);
	inWorld.GetComponent<GameplaySystem>()->SetupPlayer(inPlayerIndex, capsule->GetTransformId(), inInputNeg, inInputPos);

	return capsule;
}

GameObject* CreateUserInterface(
	GameWorld& world,
	AssetHandle<Texture> texture,
	AssetHandle<Font> fontA,
	AssetHandle<Font> fontB)
{
	ComponentTemplate<SpriteObjectComponent> spriteTemplate
	{
		.ScreenCoordsMin = { 50.0f, 50.0f },
		.ScreenCoordsMax = { 150.0f, 150.0f },
		.TextureCoordsMin = { 0.0f, 0.0f },
		.TextureCoordsMax = { 1.0f, 1.0f },
		.Texture = texture
	};

	ComponentTemplate<TextObjectComponent> textTemplateA
	{
		.Text = "Merely Another Game Engine",
		.Color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
		.ScreenPosition = glm::vec2(180.0f, 90.0f),
		.Scale = 40.0f,
		.Font = fontA
	};

	ComponentTemplate<TextObjectComponent> textTemplateB
	{
		.Text = "M.A.G.E.",
		.Color = glm::vec4(0.5f, 1.0f, 1.0f, 1.0f),
		.ScreenPosition = glm::vec2(180.0f, 140.0f),
		.Scale = 40.0f,
		.Font = fontB
	};

	return world.CreateObject<GameObject>(spriteTemplate, textTemplateA, textTemplateB);
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

	GameWorld world;
	world.CreateComponent<TransformTree>();
	world.CreateComponent<Vulkan::Renderer>(window);
	world.CreateComponent<GameplaySystem>(GameplaySystemSetup{ 10.0f, 80.0f, 80.0f, 150.0f });
	world.CreateComponent<PhysicsSystem>();
	world.CreateComponent<WorldBoundsSystem>(glm::vec3(-10000.0f, -10000.0f, -10.0f), glm::vec3(10000.0f, 10000.0f, 10000.0f));
	world.CreateComponent<MeshRenderSystem>();
	world.CreateComponent<SpriteRenderSystem>();
	world.CreateComponent<TextRenderSystem>();

	PhysicsRigidBodyParams boxRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, boardCollision, floorMaterial };
	PhysicsRigidBodyParams cylinderRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, cornerCollision, defaultMaterial };
	PhysicsRigidBodyParams capsuleRigidBodyParams = { PhysicsSystemObjectType::RigidKinematic, capsuleCollision, defaultMaterial };
	PhysicsRigidBodyParams coneRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, coneCollision, defaultMaterial };
	PhysicsRigidBodyParams ballRigidBodyParams = { PhysicsSystemObjectType::RigidDynamic, ballCollision, defaultMaterial };

	{
		CreateUserInterface(world, spriteTexture, fontArianaVioleta, fontOrbitron);

		mage::Transform transform;
		CreateLevelObject(world, transform, boxRigidBodyParams, boxMesh, cubeTexture);

		transform.Position = glm::vec3(0.0f, -30.0f, 10.0f);
		CreateControllableCamera(world, transform, 10.0f, ballRigidBodyParams, ballMesh, ballTexture, 10.0f, GLFW_KEY_F);
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(90.0f));
		
		transform.Position = {};
		CreateLevelObject(world, transform, coneRigidBodyParams, coneMesh, coneTexture);
		
		transform.Position = glm::vec3(cornerPosition, cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);
		
		transform.Position = glm::vec3(-cornerPosition, cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);
		
		transform.Position = glm::vec3(-cornerPosition, -cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);
		
		transform.Position = glm::vec3(cornerPosition, -cornerPosition, cornerHalfHeight);
		CreateLevelObject(world, transform, cylinderRigidBodyParams, cylinderMesh, cylinderTexture);
		
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
