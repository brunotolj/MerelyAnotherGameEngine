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

class OscillationComponent : public GameObjectComponent<TransformableObject>
{
public:
	mage::Transform OriginalTransform;

	glm::vec3 Extent = glm::vec3(0.0f, 0.0f, 0.0f);

	float Speed = 1.0f;

	OscillationComponent(TransformableObject& owner, const ComponentTemplate<OscillationComponent>& creationTemplate) : GameObjectComponent(owner) {}

	virtual void OnOwnerAddedToWorld(GameWorld& world)
	{
		OriginalTransform = mOwner.Transform;
	}

	virtual void UpdatePrePhysics(float deltaTime) override final
	{
		mTime += Speed * deltaTime;
		const float factor = glm::sin(mTime);

		mOwner.Transform.Position = OriginalTransform.Position + factor * Extent;
	}

private:
	float mTime = 0.0f;
};

class KillZComponent : public GameObjectComponent<TransformableObject>
{
public:
	float KillZ = 0.0f;

	KillZComponent(TransformableObject& owner, const ComponentTemplate<KillZComponent>& creationTemplate) : GameObjectComponent(owner) {}

	virtual void UpdatePostPhysics(float deltatime) override final
	{
		if (mOwner.Transform.Position.z < KillZ)
		{
			mOwner.MarkDestroyed();
		}
	}
};

void CreateCube(
	GameWorld& world, Device& device,
	const mage::Transform& transform,
	const PhysicsSystemMaterialPtr& material,
	glm::vec3 halfExtent)
{
	std::shared_ptr<TransformableObject> cube = std::make_shared<TransformableObject>();
	TransformableObject& cubeRef = *cube.get();
	cube->Transform = transform;
	
	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams.Type = PhysicsSystemObjectType::RigidStatic;
	rigidBodyTemplate.RigidBodyParams.Geometry = std::make_unique<physx::PxBoxGeometry>(halfExtent.x, halfExtent.y, halfExtent.z);
	rigidBodyTemplate.RigidBodyParams.Material = material;
	GameObject::CreateComponent(cubeRef, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = Model::CreateCube(device, halfExtent.x, halfExtent.y, halfExtent.z);
	staticMeshTemplate.Color = glm::vec3(0.5f, 0.5f, 0.5f);
	GameObject::CreateComponent(cubeRef, staticMeshTemplate);

	world.AddObject(cube);
}

void CreateCapsule(
	GameWorld& world, Device& device,
	const mage::Transform& transform,
	const PhysicsSystemMaterialPtr& material,
	float radius, float halfHeight)
{
	const glm::mat4 matrix = transform.Matrix();
	const glm::vec3 right = matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	std::shared_ptr<TransformableObject> capsule = std::make_shared<TransformableObject>();
	TransformableObject& capsuleRef = *capsule.get();
	capsule->Transform = transform;

 	OscillationComponent& oscillation = GameObject::CreateComponent(capsuleRef, ComponentTemplate<OscillationComponent>());
 	oscillation.Extent = 4.5f * right;
 	oscillation.Speed = 2.0f;

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams.Type = PhysicsSystemObjectType::RigidKinematic;
	rigidBodyTemplate.RigidBodyParams.Geometry = std::make_unique<physx::PxCapsuleGeometry>(radius, halfHeight);
	rigidBodyTemplate.RigidBodyParams.Material = material;
	GameObject::CreateComponent(capsuleRef, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = Model::CreateCapsule(device, radius, halfHeight);
	staticMeshTemplate.Color = glm::vec3(0.3f, 0.7f, 0.3f);
	GameObject::CreateComponent(capsuleRef, staticMeshTemplate);

	world.AddObject(capsule);
}

void CreateCylinder(
	GameWorld& world, Device& device,
	const mage::Transform& transform,
	const PhysicsSystemMaterialPtr& material,
	float radius, float halfHeight)
{
	std::shared_ptr<TransformableObject> cylinder = std::make_shared<TransformableObject>();
	TransformableObject& cylinderRef = *cylinder.get();
	cylinder->Transform = transform;

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams.Type = PhysicsSystemObjectType::RigidStatic;
	rigidBodyTemplate.RigidBodyParams.CustomGeometryCallbacks = std::make_shared<physx::PxCustomGeometryExt::CylinderCallbacks>(2.0f * halfHeight, radius);
	rigidBodyTemplate.RigidBodyParams.Geometry = std::make_shared<physx::PxCustomGeometry>(*rigidBodyTemplate.RigidBodyParams.CustomGeometryCallbacks.get());
	rigidBodyTemplate.RigidBodyParams.Material = material;
	GameObject::CreateComponent(cylinderRef, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = Model::CreateCylinder(device, radius, halfHeight);
	staticMeshTemplate.Color = glm::vec3(0.8f, 0.3f, 0.3f);
	GameObject::CreateComponent(cylinderRef, staticMeshTemplate);

	world.AddObject(cylinder);
}

void SpawnBall(
	GameWorld& world, Device& device,
	const mage::Transform& transform,
	const PhysicsSystemMaterialPtr& material,
	float radius)
{
	const glm::mat4 matrix = transform.Matrix();
	const glm::vec3 forward = transform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));

	std::shared_ptr<TransformableObject> ball = std::make_shared<TransformableObject>();
	TransformableObject& ballRef = *ball.get();
	ball->Transform = transform;
	
	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams.Type = PhysicsSystemObjectType::RigidDynamic;
	rigidBodyTemplate.RigidBodyParams.Geometry = std::make_shared<physx::PxSphereGeometry>(radius);
	rigidBodyTemplate.RigidBodyParams.Material = material;
	rigidBodyTemplate.InitialLinearVelocity = 10.0f * reinterpret_cast<const physx::PxVec3&>(forward);
	GameObject::CreateComponent(ballRef, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = Model::CreateSphere(device, radius);
	staticMeshTemplate.Color = glm::vec3(0.3f, 0.3f, 1.0f);
	GameObject::CreateComponent(ballRef, staticMeshTemplate);

	KillZComponent& killZComponent = GameObject::CreateComponent(ballRef, ComponentTemplate<KillZComponent>());
	killZComponent.KillZ = -10.0f;

	world.AddObject(ball);
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

	std::shared_ptr<physx::PxCustomGeometryExt::CylinderCallbacks> cylinderCollisionCallbacks = std::make_shared<physx::PxCustomGeometryExt::CylinderCallbacks>(2.0f, 1.0f, 2);
	std::shared_ptr<physx::PxGeometry> cylinderCollision = std::make_shared<physx::PxCustomGeometry>(*cylinderCollisionCallbacks.get());

	PhysicsSystemMaterialPtr material = world.GetPhysicsSystem().CreateMaterial({ 0.1f, 0.05f, 0.9f });
	PhysicsSystemMaterialPtr floorMaterial = world.GetPhysicsSystem().CreateMaterial({ 0.1f, 0.05f, -0.5f });

	{
		mage::Transform transform;

		constexpr float b = 20.0f;
		constexpr float r = 5.0f;
		constexpr float p = b - r;
		constexpr float h = 2.0f;

		constexpr float x = p + 1.0f;
		constexpr float y = 2.0f;
		constexpr float s = 3.0f;

		CreateCube(world, device, transform, floorMaterial, glm::vec3(b, b, 1.0f));

		transform.Position = glm::vec3(-x, 0.0f, y);
		CreateCapsule(world, device, transform, material, s, 0.75f * s);

		transform.Position = glm::vec3(x, 0.0f, y);
		CreateCapsule(world, device, transform, material, s, 0.75f * s);

		transform.Rotation = mage::Rotor::FromAxisAndAngle(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(90.0f));

		transform.Position = glm::vec3(p, p, h);
		CreateCylinder(world, device, transform, material, r, h);
		
		transform.Position = glm::vec3(-p, p, h);
		CreateCylinder(world, device, transform, material, r, h);
		
		transform.Position = glm::vec3(-p, -p, h);
		CreateCylinder(world, device, transform, material, r, h);

		transform.Position = glm::vec3(p, -p, h);
		CreateCylinder(world, device, transform, material, r, h);

		transform.Rotation = mage::Rotor::FromAxisAndAngle(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(90.0f));

		transform.Position = glm::vec3(0.0f, -x, y);
		CreateCapsule(world, device, transform, material, s, 0.75f * s);

		transform.Position = glm::vec3(0.0f, x, y);
		CreateCapsule(world, device, transform, material, s, 0.75f * s);
	}

	std::shared_ptr<Camera> camera = std::make_shared<Camera>();
	world.GetRenderSystem().SetCamera(camera);

	camera->mTransform.Position = glm::vec3(0.0f, -30.0f, 10.0f);

	glm::vec2 viewRotation = glm::vec2(0.0f);

	std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

	bool pendingFire = false;
	glm::dvec2 cursorMovement = glm::dvec2(0.0);

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
			SpawnBall(world, device, camera->mTransform, material, 1.0f);
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
