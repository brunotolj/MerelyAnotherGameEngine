#include "Core/Asserts.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Game/InputSystem.h"
#include "Game/RigidBodyObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Camera.h"
#include "Rendering/Device.h"
#include "Rendering/Model.h"
#include "Rendering/Renderer.h"
#include "Rendering/Window.h"
#include "Rendering/RenderSystem.h"
#include "Utility/BoundedLineMovementComponent.h"
#include "Utility/KillZObjectComponent.h"

#include <array>
#include <chrono>
#include <memory>

static constexpr int gWindowWidth = 1280;
static constexpr int gWindowHeight = 720;

void MoveByInput(InputSystem& inputSystem, float deltaTime, mage::Transform& transform)
{
	glm::vec3 movement(0.0f);

	if (inputSystem.GetKeyState(GLFW_KEY_D) == GLFW_PRESS) movement.x += 1.0f;
	if (inputSystem.GetKeyState(GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1.0f;

	if (inputSystem.GetKeyState(GLFW_KEY_W) == GLFW_PRESS) movement.y += 1.0f;
	if (inputSystem.GetKeyState(GLFW_KEY_S) == GLFW_PRESS) movement.y -= 1.0f;

	movement = transform.Matrix() * glm::vec4(movement, 0.0f);

	if (inputSystem.GetKeyState(GLFW_KEY_E) == GLFW_PRESS) movement.z += 1.0f;
	if (inputSystem.GetKeyState(GLFW_KEY_Q) == GLFW_PRESS) movement.z -= 1.0f;

	transform.Position += 5.0f * deltaTime * movement;
}

void RotateCameraByInput(glm::dvec2 cursorMovement, glm::vec2& accumulatedRotation, float deltaTime, mage::Transform& transform)
{
	accumulatedRotation += 0.01f * glm::vec2(cursorMovement);
	accumulatedRotation.y = glm::clamp(accumulatedRotation.y, -glm::radians(80.0f), glm::radians(80.0f));

	transform.Rotation = mage::Rotor::Combine(
		mage::Rotor::FromAxisAndAngle({ 0.0f, 0.0f, 1.0f }, accumulatedRotation.x),
		mage::Rotor::FromAxisAndAngle({ 1.0f, 0.0f, 0.0f }, accumulatedRotation.y));
}

std::shared_ptr<TransformableObject> CreateLevelObject(
	const mage::Transform& transform,
	PhysicsRigidBodyParams rigidBodyParams,
	std::shared_ptr<Model> model,
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
	std::shared_ptr<Model> model,
	glm::vec3 color,
	int inputNeg,
	int inputPos)
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

std::shared_ptr<TransformableObject> SpawnBall(
	const mage::Transform& transform,
	PhysicsRigidBodyParams rigidBodyParams,
	std::shared_ptr<Model> model,
	glm::vec3 color)
{
	const glm::mat4 matrix = transform.Matrix();
	const glm::vec3 forward = transform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));

	std::shared_ptr<TransformableObject> ballPtr = std::make_shared<TransformableObject>();
	TransformableObject& ball = *ballPtr.get();
	ballPtr->Transform = transform;
	
	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = rigidBodyParams;
	rigidBodyTemplate.InitialLinearVelocity = 10.0f * reinterpret_cast<const physx::PxVec3&>(forward);
	GameObject::CreateComponent(ball, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = model;
	staticMeshTemplate.Color = color;
	GameObject::CreateComponent(ball, staticMeshTemplate);

	ComponentTemplate<KillZObjectComponent> killZTemplate;
	killZTemplate.KillZ = -10.0f;
	GameObject::CreateComponent(ball, killZTemplate);

	return ballPtr;
}

int main()
{
	Window window{ gWindowWidth, gWindowHeight, "Window" };
	Device device{ window };
	Renderer renderer{ window, device };

	GameWorld world(
		std::make_unique<InputSystem>(window),
		std::make_unique<RenderSystem>(device, renderer.GetSwapchainRenderPass()),
		std::make_unique<PhysicsSystem>());

	constexpr float boardSize = 20.0f;

	constexpr float cornerHalfHeight = 3.0f;
	constexpr float cornerRadius = 4.0f;
	constexpr float cornerPosition = boardSize - cornerRadius;

	constexpr float capsuleDistance = cornerPosition + 4.0f;
	constexpr float capsuleElevation = 2.0f;
	constexpr float capsuleRadius = 2.0f;
	constexpr float capsuleLength = 1.5f;

	constexpr float ballRadius = 1.0f;

	PhysicsSystemMaterialPtr defaultMaterial = world.GetPhysicsSystem().CreateMaterial({ 0.1f, 0.05f, 0.9f });
	PhysicsSystemMaterialPtr floorMaterial = world.GetPhysicsSystem().CreateMaterial({ 0.1f, 0.05f, -0.5f });

	std::shared_ptr<physx::PxGeometry> floorCollision = std::make_unique<physx::PxBoxGeometry>(boardSize, boardSize, 1.0f);
	std::shared_ptr<Model> floorModel = Model::CreateCube(device, boardSize, boardSize, 1.0f);
	PhysicsRigidBodyParams floorRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, nullptr, floorCollision, floorMaterial };
	constexpr glm::vec3 floorColor = glm::vec3(0.5f, 0.5f, 0.5f);

	std::shared_ptr<physx::PxCustomGeometryExt::CylinderCallbacks> cornerCollisionCallbacks = std::make_shared<physx::PxCustomGeometryExt::CylinderCallbacks>(2.0f * cornerHalfHeight, cornerRadius);
	std::shared_ptr<physx::PxGeometry> cornerCollision = std::make_shared<physx::PxCustomGeometry>(*cornerCollisionCallbacks.get());
	std::shared_ptr<Model> cornerModel = Model::CreateCylinder(device, cornerRadius, cornerHalfHeight);
	PhysicsRigidBodyParams cornerRigidBodyParams = { PhysicsSystemObjectType::RigidStatic, cornerCollisionCallbacks, cornerCollision, defaultMaterial };
	constexpr glm::vec3 cornerColor = glm::vec3(0.8f, 0.3f, 0.3f);

	std::shared_ptr<physx::PxGeometry> capsuleCollision = std::make_unique<physx::PxCapsuleGeometry>(capsuleRadius, capsuleLength);
	std::shared_ptr<Model> capsuleModel = Model::CreateCapsule(device, capsuleRadius, capsuleLength);
	PhysicsRigidBodyParams capsuleRigidBodyParams = { PhysicsSystemObjectType::RigidKinematic, nullptr, capsuleCollision, defaultMaterial };
	constexpr glm::vec3 capsuleColor = glm::vec3(0.3f, 0.7f, 0.3f);

	std::shared_ptr<physx::PxGeometry> ballCollision = std::make_unique<physx::PxSphereGeometry>(ballRadius);
	std::shared_ptr<Model> ballModel = Model::CreateSphere(device, ballRadius);
	PhysicsRigidBodyParams ballRigidBodyParams = { PhysicsSystemObjectType::RigidDynamic, nullptr, ballCollision, defaultMaterial };
	constexpr glm::vec3 ballColor = glm::vec3(0.3f, 0.3f, 1.0f);

	{
		mage::Transform transform;

		world.AddObject(CreateLevelObject(transform, floorRigidBodyParams, floorModel, floorColor));

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
	
	std::shared_ptr<Camera> camera = std::make_shared<Camera>();
	camera->mTransform.Position = glm::vec3(0.0f, -30.0f, 10.0f);
	world.GetRenderSystem().SetCamera(camera);

	bool pendingFire = false;
	glm::dvec2 cursorMovement = glm::dvec2(0.0);
	glm::vec2 viewRotation = glm::vec2(0.0f);

	world.GetInputSystem().BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_PRESS, [&window]() { window.SetCursorInputMode(GLFW_CURSOR_NORMAL); });
	world.GetInputSystem().BindKeyInputHandler(GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE, [&window]() { window.SetCursorInputMode(GLFW_CURSOR_DISABLED); });
	world.GetInputSystem().BindKeyInputHandler(GLFW_KEY_ESCAPE, GLFW_PRESS, [&window]() { window.Close(); });
	world.GetInputSystem().BindKeyInputHandler(GLFW_KEY_F, GLFW_PRESS, [&pendingFire]() { pendingFire = true; });

	world.GetInputSystem().BindCursorMovementHandler([&cursorMovement](glm::dvec2 movement, int cursorMode)
		{ if (cursorMode == GLFW_CURSOR_DISABLED) cursorMovement += movement; });

	while (!window.ShouldClose())
	{
		Window::PollEvents();

		const std::chrono::steady_clock::time_point newTime = std::chrono::high_resolution_clock::now();
		const float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		RotateCameraByInput(cursorMovement, viewRotation, frameTime, camera->mTransform);
		cursorMovement = glm::dvec2(0.0);

		MoveByInput(world.GetInputSystem(), frameTime, camera->mTransform);
		camera->SetPerspectiveParams(0.1f, 1000.0f, glm::radians(90.0f), renderer.GetAspectRatio());

		if (pendingFire)
		{
			world.AddObject(SpawnBall(camera->mTransform, ballRigidBodyParams, ballModel, ballColor));
			pendingFire = false;
		}

		world.Update(frameTime);

		if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
		{
			renderer.BeginSwapChainRenderPass(commandBuffer);

			world.GetRenderSystem().RenderScene(commandBuffer);

			renderer.EndSwapChainRenderPass(commandBuffer);
			renderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(device.GetDevice());

	return 0;
}
