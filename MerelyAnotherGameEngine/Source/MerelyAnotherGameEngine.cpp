#include "Core/Asserts.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
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

void MoveByInput(Window& window, float deltaTime, mage::Transform& transform)
{
	glm::vec3 movement(0.0f);

	if (window.GetKeyState(GLFW_KEY_D) == GLFW_PRESS) movement.x += 1.0f;
	if (window.GetKeyState(GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1.0f;

	if (window.GetKeyState(GLFW_KEY_W) == GLFW_PRESS) movement.y += 1.0f;
	if (window.GetKeyState(GLFW_KEY_S) == GLFW_PRESS) movement.y -= 1.0f;

	movement = transform.Matrix() * glm::vec4(movement, 0.0f);

	if (window.GetKeyState(GLFW_KEY_E) == GLFW_PRESS) movement.z += 1.0f;
	if (window.GetKeyState(GLFW_KEY_Q) == GLFW_PRESS) movement.z -= 1.0f;

	transform.Position += 5.0f * deltaTime * movement;
}

void RotateCameraByInput(Window& window, glm::vec2& accumulatedRotation, float deltaTime, mage::Transform& transform)
{
	accumulatedRotation += 0.01f * window.ConsumeMovement();
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

	OscillationComponent(TransformableObject& owner) : GameObjectComponent(owner) {}

	virtual void OnOwnerAddedToWorld(GameWorld& world)
	{
		OriginalTransform = mOwner.Transform;
	}

	virtual void UpdatePrePhysics(float deltaTime) override
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

	KillZComponent(TransformableObject& owner) : GameObjectComponent(owner) {}

	virtual void UpdatePostPhysics(float deltatime) override
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
	std::shared_ptr<TransformableObject> gameObject = std::make_shared<TransformableObject>();
	gameObject->Transform = transform;
	
	RigidBodyObjectComponent* const rigidBody = gameObject->CreateComponent<RigidBodyObjectComponent>();
	rigidBody->RigidBodyParams.Type = PhysicsSystemObjectType::RigidStatic;
	rigidBody->RigidBodyParams.Geometry = std::make_unique<physx::PxBoxGeometry>(halfExtent.x, halfExtent.y, halfExtent.z);
	rigidBody->RigidBodyParams.Material = material;

	StaticMeshObjectComponent* const staticMesh = gameObject->CreateComponent<StaticMeshObjectComponent>();
	staticMesh->mModel = Model::CreateCube(device, halfExtent.x, halfExtent.y, halfExtent.z);
	staticMesh->mColor = glm::vec3(0.5f, 0.5f, 0.5f);

	world.AddObject(gameObject);
}

void CreateCapsule(
	GameWorld& world, Device& device,
	const mage::Transform& transform,
	const PhysicsSystemMaterialPtr& material,
	float radius, float halfHeight)
{
	const glm::mat4 matrix = transform.Matrix();
	const glm::vec3 right = matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	std::shared_ptr<TransformableObject> gameObject = std::make_shared<TransformableObject>();
	gameObject->Transform = transform;

 	OscillationComponent* const oscillation = gameObject->CreateComponent<OscillationComponent>();
 	oscillation->Extent = 4.5f * right;
 	oscillation->Speed = 2.0f;

	RigidBodyObjectComponent* const rigidBody = gameObject->CreateComponent<RigidBodyObjectComponent>();
	rigidBody->RigidBodyParams.Type = PhysicsSystemObjectType::RigidKinematic;
	rigidBody->RigidBodyParams.Geometry = std::make_shared<physx::PxCapsuleGeometry>(radius, halfHeight);
	rigidBody->RigidBodyParams.Material = material;

	StaticMeshObjectComponent* const staticMesh = gameObject->CreateComponent<StaticMeshObjectComponent>();
	staticMesh->mModel = Model::CreateCapsule(device, radius, halfHeight);
	staticMesh->mColor = glm::vec3(0.3f, 0.7f, 0.3f);

	world.AddObject(gameObject);
}

void CreateCylinder(
	GameWorld& world, Device& device,
	const mage::Transform& transform,
	const PhysicsSystemMaterialPtr& material,
	float radius, float halfHeight)
{
	std::shared_ptr<TransformableObject> gameObject = std::make_shared<TransformableObject>();
	gameObject->Transform = transform;
	
	RigidBodyObjectComponent* const rigidBody = gameObject->CreateComponent<RigidBodyObjectComponent>();
	rigidBody->RigidBodyParams.Type = PhysicsSystemObjectType::RigidStatic;
	rigidBody->RigidBodyParams.CustomGeometryCallbacks = std::make_shared<physx::PxCustomGeometryExt::CylinderCallbacks>(2.0f * halfHeight, radius);
	rigidBody->RigidBodyParams.Geometry = std::make_shared<physx::PxCustomGeometry>(*rigidBody->RigidBodyParams.CustomGeometryCallbacks.get());
	rigidBody->RigidBodyParams.Material = material;

	StaticMeshObjectComponent* const staticMesh = gameObject->CreateComponent<StaticMeshObjectComponent>();
	staticMesh->mModel = Model::CreateCylinder(device, radius, halfHeight);
	staticMesh->mColor = glm::vec3(0.8f, 0.3f, 0.3f);

	world.AddObject(gameObject);
}

void SpawnBall(
	GameWorld& world, Device& device,
	const mage::Transform& transform,
	const PhysicsSystemMaterialPtr& material,
	float radius)
{
	const glm::mat4 matrix = transform.Matrix();
	const glm::vec3 forward = matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	std::shared_ptr<TransformableObject> gameObject = std::make_shared<TransformableObject>();
	gameObject->Transform = transform;
	
	RigidBodyObjectComponent* const rigidBody = gameObject->CreateComponent<RigidBodyObjectComponent>();
	rigidBody->RigidBodyParams.Type = PhysicsSystemObjectType::RigidDynamic;
	rigidBody->RigidBodyParams.Geometry = std::make_shared<physx::PxSphereGeometry>(radius);
	rigidBody->RigidBodyParams.Material = material;
	rigidBody->LinearVelocity = 10.0f * reinterpret_cast<const physx::PxVec3&>(forward);

	StaticMeshObjectComponent* const staticMesh = gameObject->CreateComponent<StaticMeshObjectComponent>();
	staticMesh->mModel = Model::CreateSphere(device, radius);
	staticMesh->mColor = glm::vec3(0.3f, 0.3f, 1.0f);

	KillZComponent* const killZComponent = gameObject->CreateComponent<KillZComponent>();
	killZComponent->KillZ = -10.0f;

	world.AddObject(gameObject);
}

int main()
{
	Window window{ gWindowWidth, gWindowHeight, "Window" };
	Device device{ window };
	Renderer renderer{ window, device };

	GameWorld world(
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

	while (!window.ShouldClose())
	{
		Window::PollEvents();

		const std::chrono::steady_clock::time_point newTime = std::chrono::high_resolution_clock::now();
		const float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		RotateCameraByInput(window, viewRotation, frameTime, camera->mTransform);
		MoveByInput(window, frameTime, camera->mTransform);
		camera->SetPerspectiveParams(0.1f, 1000.0f, glm::radians(90.0f), renderer.GetAspectRatio());

		if (window.TEMP_mPendingFire)
		{
			SpawnBall(world, device, camera->mTransform, material, 1.0f);
			window.TEMP_mPendingFire = false;
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
