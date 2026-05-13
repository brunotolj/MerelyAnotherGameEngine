#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Game/InputSystem.h"
#include "Game/CameraComponent.h"
#include "Game/RigidBodyObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Rendering/Systems/TextRenderSystem.h"
#include "Utility/BallSpawnerComponent.h"
#include "Utility/BoundedLineMovementComponent.h"
#include "Utility/DefaultMovementComponent.h"
#include "Vulkan/Model.h"
#include "Vulkan/Renderer.h"
#include "Vulkan/VulkanInterface.h"
#include "Vulkan/Window.h"

#include <chrono>
#include <memory>

static constexpr i32 gWindowWidth = 1920;
static constexpr i32 gWindowHeight = 1080;

std::shared_ptr<TransformableObject> CreateControllableCamera(
	const mage::Transform& transform,
	f32 speed,
	PhysicsRigidBodyParams ballRigidBodyParams,
	std::shared_ptr<Vulkan::Model> ballModel,
	u32 ballTextureIndex,
	f32 ballSpeed,
	i32 inputSpawnBall)
{
	std::shared_ptr<TransformableObject> objectPtr = std::make_shared<TransformableObject>();
	TransformableObject& object = *objectPtr.get();
	object.Transform = transform;

	ComponentTemplate<DefaultMovementComponent> movementTemplate;
	movementTemplate.Speed = speed;
	GameObject::CreateComponent(object, movementTemplate);

	ComponentTemplate<CameraComponent> cameraTemplate;
	GameObject::CreateComponent(object, cameraTemplate);

	ComponentTemplate<BallSpawnerComponent> ballSpawnerTemplate;
	ballSpawnerTemplate.RigidBodyParams = ballRigidBodyParams;
	ballSpawnerTemplate.Model = ballModel;
	ballSpawnerTemplate.TextureIndex = ballTextureIndex;
	ballSpawnerTemplate.Speed = ballSpeed;
	ballSpawnerTemplate.InputSpawn = inputSpawnBall;
	GameObject::CreateComponent(object, ballSpawnerTemplate);

	return objectPtr;
}

std::shared_ptr<TransformableObject> CreateLevelObject(
	const mage::Transform& transform,
	PhysicsRigidBodyParams rigidBodyParams,
	std::shared_ptr<Vulkan::Model> model,
	u32 textureIndex)
{
	std::shared_ptr<TransformableObject> objectPtr = std::make_shared<TransformableObject>();
	TransformableObject& object = *objectPtr.get();
	object.Transform = transform;

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = rigidBodyParams;
	GameObject::CreateComponent(object, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = model;
	staticMeshTemplate.TextureIndex = textureIndex;
	GameObject::CreateComponent(object, staticMeshTemplate);

	return objectPtr;
}

std::shared_ptr<TransformableObject> CreateCapsule(
	const mage::Transform& transform,
	PhysicsRigidBodyParams rigidBodyParams,
	std::shared_ptr<Vulkan::Model> model,
	u32 textureIndex,
	i32 inputNeg,
	i32 inputPos)
{
	std::shared_ptr<TransformableObject> capsulePtr = std::make_shared<TransformableObject>();
	TransformableObject& capsule = *capsulePtr.get();
	capsulePtr->Transform = transform;

	ComponentTemplate<BoundedLineMovementComponent> movementTemplate;
	movementTemplate.Extent = 10.0f * transform.Rotation.Rotate({0.0f, -1.0f, 0.0f});
 	movementTemplate.InputNeg = inputNeg;
 	movementTemplate.InputPos = inputPos;
 	movementTemplate.Acceleration = 80.0f;
 	movementTemplate.Deceleration = 150.0f;
 	movementTemplate.MaxSpeed = 80.0f;
	GameObject::CreateComponent(capsule, movementTemplate);

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = rigidBodyParams;
	GameObject::CreateComponent(capsule, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = model;
	staticMeshTemplate.TextureIndex = textureIndex;
	GameObject::CreateComponent(capsule, staticMeshTemplate);

	return capsulePtr;
}

i32 main()
{
	Vulkan::WindowInfo windowCreateInfo
	{
		.Name = "Merely Another Game Engine",
		.Width = gWindowWidth,
		.Height = gWindowHeight
	};

	Vulkan::Instance vulkan;
	Vulkan::Window window = vulkan.CreateWindow(windowCreateInfo);
	Vulkan::Renderer renderer{ vulkan, window };

	mage::Array<mage::StringView> meshTexturePaths
	{
		"Textures/cube.png",
		"Textures/ball.png",
		"Textures/cylinder.png",
		"Textures/capsule.png",
		"Textures/cone.png"
	};

	mage::Array<mage::StringView> spriteTexturePaths
	{
		"Textures/default.png"
	};

	mage::Array<mage::StringView> fontPaths
	{
		"Fonts/ArianaVioleta-dz2K.ttf",
		"Fonts/Orbitron-Regular.ttf",
	};

	Vulkan::ShaderCompiler shaderCompiler;

	GameWorld world(
		std::make_unique<InputSystem>(window),
		std::make_unique<PhysicsSystem>(),
		std::make_unique<MeshRenderSystem>(renderer, shaderCompiler, meshTexturePaths),
		std::make_unique<SpriteRenderSystem>(renderer, shaderCompiler, spriteTexturePaths),
		std::make_unique<TextRenderSystem>(renderer, shaderCompiler, fontPaths));

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

	PhysicsSystemMaterialPtr defaultMaterial = world.GetPhysicsSystem().CreateMaterial({ 0.2f, 0.1f, 1.0f });
	PhysicsSystemMaterialPtr floorMaterial = world.GetPhysicsSystem().CreateMaterial({ 0.2f, 0.05f, 0.0f });
	
	std::shared_ptr<physx::PxGeometry> boxCollision = std::make_unique<physx::PxBoxGeometry>(boardSize, boardSize, 1.0f);
	std::shared_ptr<Vulkan::Model> boxModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeBox({ boardSize, boardSize, 1.0f }));
	PhysicsRigidBodyParams boxRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, nullptr, boxCollision, floorMaterial };
	
	std::shared_ptr<physx::PxCustomGeometryExt::CylinderCallbacks> cylinderCollisionCallbacks = std::make_shared<physx::PxCustomGeometryExt::CylinderCallbacks>(2.0f * cornerHalfHeight, cornerRadius);
	std::shared_ptr<physx::PxGeometry> cylinderCollision = std::make_shared<physx::PxCustomGeometry>(*cylinderCollisionCallbacks.get());
	std::shared_ptr<Vulkan::Model> cylinderModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeCylinder(cornerRadius, cornerHalfHeight));
	PhysicsRigidBodyParams cylinderRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, cylinderCollisionCallbacks, cylinderCollision, defaultMaterial };
	
	std::shared_ptr<physx::PxGeometry> capsuleCollision = std::make_unique<physx::PxCapsuleGeometry>(capsuleRadius, capsuleLength);
	std::shared_ptr<Vulkan::Model> capsuleModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeCapsule(capsuleRadius, capsuleLength));
	PhysicsRigidBodyParams capsuleRigidBodyParams = { PhysicsSystemObjectType::RigidKinematic, nullptr, capsuleCollision, defaultMaterial };
	
	std::shared_ptr<physx::PxCustomGeometryExt::ConeCallbacks> coneCollisionCallbacks = std::make_shared<physx::PxCustomGeometryExt::ConeCallbacks>(coneHeight, coneRadius);
	std::shared_ptr<physx::PxGeometry> coneCollision = std::make_shared<physx::PxCustomGeometry>(*coneCollisionCallbacks.get());
	std::shared_ptr<Vulkan::Model> coneModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeCone(coneRadius, coneHeight));
	PhysicsRigidBodyParams coneRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, coneCollisionCallbacks, coneCollision, defaultMaterial };

	std::shared_ptr<physx::PxGeometry> ballCollision = std::make_unique<physx::PxSphereGeometry>(ballRadius);
	std::shared_ptr<Vulkan::Model> ballModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeBall(ballRadius));
	PhysicsRigidBodyParams ballRigidBodyParams = { PhysicsSystemObjectType::RigidDynamic, nullptr, ballCollision, defaultMaterial };

	{
		mage::Transform transform;

		world.AddObject(CreateLevelObject(transform, boxRigidBodyParams, boxModel, 0));

		transform.Position = glm::vec3(0.0f, -30.0f, 10.0f);
		world.AddObject(CreateControllableCamera(transform, 10.0f, ballRigidBodyParams, ballModel, 1, 10.0f, GLFW_KEY_F));
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(90.0f));
		
		transform.Position = {};
		world.AddObject(CreateLevelObject(transform, coneRigidBodyParams, coneModel, 4));
		
		transform.Position = glm::vec3(cornerPosition, cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cylinderRigidBodyParams, cylinderModel, 2));
		
		transform.Position = glm::vec3(-cornerPosition, cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cylinderRigidBodyParams, cylinderModel, 2));
		
		transform.Position = glm::vec3(-cornerPosition, -cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cylinderRigidBodyParams, cylinderModel, 2));
		
		transform.Position = glm::vec3(cornerPosition, -cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cylinderRigidBodyParams, cylinderModel, 2));
		
		transform.Rotation = {};
		transform.Position = glm::vec3(-capsuleDistance, 0.0f, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, 3, GLFW_KEY_H, GLFW_KEY_J));
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(90.0f));
		transform.Position = glm::vec3(0.0f, capsuleDistance, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, 3, GLFW_KEY_U, GLFW_KEY_I));
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(180.0f));
		transform.Position = glm::vec3(capsuleDistance, 0.0f, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, 3, GLFW_KEY_O, GLFW_KEY_P));
		
		transform.Rotation = mage::Rotor(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(-90.0f));
		transform.Position = glm::vec3(0.0f, -capsuleDistance, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, 3, GLFW_KEY_K, GLFW_KEY_L));
	}

	std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

	world.GetInputSystem().BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS, [&window]() { window.SetCursorInputMode(GLFW_CURSOR_NORMAL); });
	world.GetInputSystem().BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE, [&window]() { window.SetCursorInputMode(GLFW_CURSOR_DISABLED); });
	world.GetInputSystem().BindKeyInputHandler(GLFW_KEY_ESCAPE, GLFW_PRESS, [&window]() { window.RequestClose(); });

	while (!window.ShouldClose())
	{
		Vulkan::Window::PollEvents();

		const std::chrono::steady_clock::time_point newTime = std::chrono::high_resolution_clock::now();
		const f32 frameTime = std::chrono::duration<f32, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		world.Update(frameTime);

		world.Render(renderer);
	}

	renderer.WaitIdle();

	return 0;
}
