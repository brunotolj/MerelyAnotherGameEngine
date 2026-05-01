#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Game/InputSystem.h"
#include "Game/CameraComponent.h"
#include "Game/RigidBodyObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Utility/BallSpawnerComponent.h"
#include "Utility/BoundedLineMovementComponent.h"
#include "Utility/DefaultMovementComponent.h"
#include "Vulkan/Model.h"
#include "Vulkan/Renderer.h"
#include "Vulkan/VulkanInterface.h"
#include "Vulkan/Window.h"

#include <chrono>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

static constexpr i32 gWindowWidth = 1920;
static constexpr i32 gWindowHeight = 1080;

std::shared_ptr<TransformableObject> CreateControllableCamera(
	const mage::Transform& transform,
	f32 speed,
	PhysicsRigidBodyParams ballRigidBodyParams,
	std::shared_ptr<Vulkan::Model> ballModel,
	glm::vec3 ballColor,
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
	ballSpawnerTemplate.Color = ballColor;
	ballSpawnerTemplate.Speed = ballSpeed;
	ballSpawnerTemplate.InputSpawn = inputSpawnBall;
	GameObject::CreateComponent(object, ballSpawnerTemplate);

	return objectPtr;
}

std::shared_ptr<TransformableObject> CreateLevelObject(
	const mage::Transform& transform,
	PhysicsRigidBodyParams rigidBodyParams,
	std::shared_ptr<Vulkan::Model> model,
	glm::vec3 color)
{
	std::shared_ptr<TransformableObject> objectPtr = std::make_shared<TransformableObject>();
	TransformableObject& object = *objectPtr.get();
	object.Transform = transform;

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = rigidBodyParams;
	GameObject::CreateComponent(object, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = model;
	staticMeshTemplate.Color = color;
	GameObject::CreateComponent(object, staticMeshTemplate);

	return objectPtr;
}

std::shared_ptr<TransformableObject> CreateCapsule(
	const mage::Transform& transform,
	PhysicsRigidBodyParams rigidBodyParams,
	std::shared_ptr<Vulkan::Model> model,
	glm::vec3 color,
	i32 inputNeg,
	i32 inputPos)
{
	const glm::mat4 matrix = transform.Matrix();
	const glm::vec3 right = matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	std::shared_ptr<TransformableObject> capsulePtr = std::make_shared<TransformableObject>();
	TransformableObject& capsule = *capsulePtr.get();
	capsulePtr->Transform = transform;

	ComponentTemplate<BoundedLineMovementComponent> movementTemplate;
 	movementTemplate.Extent = 10.0f * right;
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
	staticMeshTemplate.Color = color;
	GameObject::CreateComponent(capsule, staticMeshTemplate);

	return capsulePtr;
}

i32 main()
{
	FT_Library library;
	{
		FT_Error result = FT_Init_FreeType(&library);
		mage_check(result == 0);
	}

	Vulkan::WindowInfo windowCreateInfo
	{
		.Name = "Merely Another Game Engine",
		.Width = gWindowWidth,
		.Height = gWindowHeight
	};

	Vulkan::Instance vulkan;
	Vulkan::Window window = vulkan.CreateWindow(windowCreateInfo);
	Vulkan::Renderer renderer{ vulkan, window };

	mage::Array<mage::StringView> texturePaths;
	texturePaths.Add("Textures/default.png");

	Vulkan::ShaderCompiler shaderCompiler;

	GameWorld world(
		std::make_unique<InputSystem>(window),
		std::make_unique<PhysicsSystem>(),
		std::make_unique<MeshRenderSystem>(renderer, shaderCompiler, texturePaths),
		std::make_unique<SpriteRenderSystem>(renderer, shaderCompiler, texturePaths));

	constexpr f32 boardSize = 20.0f;

	constexpr f32 cornerHalfHeight = 3.0f;
	constexpr f32 cornerRadius = 4.0f;
	constexpr f32 cornerPosition = boardSize - cornerRadius;

	constexpr f32 capsuleDistance = cornerPosition + 4.0f;
	constexpr f32 capsuleElevation = 2.0f;
	constexpr f32 capsuleRadius = 2.0f;
	constexpr f32 capsuleLength = 1.5f;

	constexpr f32 ballRadius = 1.0f;

	PhysicsSystemMaterialPtr defaultMaterial = world.GetPhysicsSystem().CreateMaterial({ 0.3f, 0.1f, 1.0f });
	PhysicsSystemMaterialPtr floorMaterial = world.GetPhysicsSystem().CreateMaterial({ 0.3f, 0.1f, 0.0f });
	
	std::shared_ptr<physx::PxGeometry> floorCollision = std::make_unique<physx::PxBoxGeometry>(boardSize, boardSize, 1.0f);
	std::shared_ptr<Vulkan::Model> floorModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeCube({ boardSize, boardSize, 1.0f }));
	PhysicsRigidBodyParams floorRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, nullptr, floorCollision, floorMaterial };
	constexpr glm::vec3 floorColor = glm::vec3(0.5f, 0.5f, 0.5f);
	
	std::shared_ptr<physx::PxCustomGeometryExt::CylinderCallbacks> cornerCollisionCallbacks = std::make_shared<physx::PxCustomGeometryExt::CylinderCallbacks>(2.0f * cornerHalfHeight, cornerRadius);
	std::shared_ptr<physx::PxGeometry> cornerCollision = std::make_shared<physx::PxCustomGeometry>(*cornerCollisionCallbacks.get());
	std::shared_ptr<Vulkan::Model> cornerModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeCylinder(cornerRadius, cornerHalfHeight));
	PhysicsRigidBodyParams cornerRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, cornerCollisionCallbacks, cornerCollision, defaultMaterial };
	constexpr glm::vec3 cornerColor = glm::vec3(0.8f, 0.3f, 0.3f);
	
	std::shared_ptr<physx::PxGeometry> capsuleCollision = std::make_unique<physx::PxCapsuleGeometry>(capsuleRadius, capsuleLength);
	std::shared_ptr<Vulkan::Model> capsuleModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeCapsule(capsuleRadius, capsuleLength));
	PhysicsRigidBodyParams capsuleRigidBodyParams = { PhysicsSystemObjectType::RigidKinematic, nullptr, capsuleCollision, defaultMaterial };
	constexpr glm::vec3 capsuleColor = glm::vec3(0.3f, 0.7f, 0.3f);
	
	std::shared_ptr<physx::PxGeometry> ballCollision = std::make_unique<physx::PxSphereGeometry>(ballRadius);
	std::shared_ptr<Vulkan::Model> ballModel = std::make_unique<Vulkan::Model>(renderer, Vulkan::Model::MakeSphere(ballRadius));
	PhysicsRigidBodyParams ballRigidBodyParams = { PhysicsSystemObjectType::RigidDynamic, nullptr, ballCollision, defaultMaterial };
	constexpr glm::vec3 ballColor = glm::vec3(0.3f, 0.3f, 1.0f);

	{
		mage::Transform transform;
		
		world.AddObject(CreateLevelObject(transform, floorRigidBodyParams, floorModel, floorColor));
		
		transform.Position = glm::vec3(0.0f, -30.0f, 10.0f);
		world.AddObject(CreateControllableCamera(transform, 10.0f, ballRigidBodyParams, ballModel, ballColor, 10.0f, GLFW_KEY_F));
		
		transform.Rotation = mage::Rotor::FromAxisAndAngle(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(90.0f));
		
		transform.Position = glm::vec3(cornerPosition, cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cornerRigidBodyParams, cornerModel, cornerColor));
		
		transform.Position = glm::vec3(-cornerPosition, cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cornerRigidBodyParams, cornerModel, cornerColor));
		
		transform.Position = glm::vec3(-cornerPosition, -cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cornerRigidBodyParams, cornerModel, cornerColor));
		
		transform.Position = glm::vec3(cornerPosition, -cornerPosition, cornerHalfHeight);
		world.AddObject(CreateLevelObject(transform, cornerRigidBodyParams, cornerModel, cornerColor));
		
		transform.Rotation = mage::Rotor::Identity();
		transform.Position = glm::vec3(capsuleDistance, 0.0f, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, capsuleColor, GLFW_KEY_U, GLFW_KEY_I));
		
		transform.Rotation = mage::Rotor::FromAxisAndAngle(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(90.0f));
		transform.Position = glm::vec3(0.0f, -capsuleDistance, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, capsuleColor, GLFW_KEY_O, GLFW_KEY_P));
		
		transform.Rotation = mage::Rotor::FromAxisAndAngle(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(180.0f));
		transform.Position = glm::vec3(-capsuleDistance, 0.0f, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, capsuleColor, GLFW_KEY_H, GLFW_KEY_J));
		
		transform.Rotation = mage::Rotor::FromAxisAndAngle(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(-90.0f));
		transform.Position = glm::vec3(0.0f, capsuleDistance, capsuleElevation);
		world.AddObject(CreateCapsule(transform, capsuleRigidBodyParams, capsuleModel, capsuleColor, GLFW_KEY_K, GLFW_KEY_L));
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
