#include "Core/Asserts.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/RigidBodyObjectComponent.h"
#include "Rendering/Camera.h"
#include "Rendering/Device.h"
#include "Rendering/Model.h"
#include "Rendering/Renderer.h"
#include "Rendering/Window.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/StaticMeshObjectComponent.h"

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

void SpawnBall(GameWorld& world, std::shared_ptr<Model>& model, const mage::Transform& transform)
{
	const glm::mat4 matrix = transform.Matrix();
	const glm::vec3 forward = matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	std::shared_ptr<GameObject> gameObject = std::make_shared<GameObject>();
	gameObject->mTransform = transform;
	
	RigidBodyObjectComponent* const rigidBody = gameObject->CreateComponent<RigidBodyObjectComponent>();
	rigidBody->mType = PhysicsSystemObjectType::RigidDynamic;
	rigidBody->mGeometry = std::make_unique<physx::PxSphereGeometry>(1.0f);
	rigidBody->mLinearVelocity = 50.0f * reinterpret_cast<const physx::PxVec3&>(forward);

	StaticMeshObjectComponent* const staticMesh = gameObject->CreateComponent<StaticMeshObjectComponent>();
	staticMesh->mModel = model;
	staticMesh->mColor = glm::vec3(0.3f, 0.3f, 1.0f);

	world.AddObject(gameObject);
}

int main()
{
	Window window{ gWindowWidth, gWindowHeight, "Window" };
	Device device{ window };
	Renderer renderer{ window, device };

	GameWorld world;
	world.mPhysicsSystem = std::make_unique<PhysicsSystem>();
	world.mRenderSystem = std::make_unique<RenderSystem>(device, renderer.GetSwapchainRenderPass());

	// these models are not in source control, sorry
	std::shared_ptr<Model> ballModel = Model::CreateFromFile(device, "Models/ball.obj");
	std::shared_ptr<Model> boxModel = Model::CreateFromFile(device, "Models/box.obj");

	// floor
	{
		std::shared_ptr<GameObject> gameObject = std::make_shared<GameObject>();
		gameObject->mTransform.Position = glm::vec3(0.0f, 0.0f, 1.0f);
		gameObject->mTransform.Scale = glm::vec3(20.0f, 20.0f, 1.0f);

		RigidBodyObjectComponent* const rigidBody = gameObject->CreateComponent<RigidBodyObjectComponent>();
		rigidBody->mType = PhysicsSystemObjectType::RigidStatic;
		rigidBody->mGeometry = std::make_unique<physx::PxBoxGeometry>(20.0f, 20.0f, 1.0f);

		StaticMeshObjectComponent* const staticMesh = gameObject->CreateComponent<StaticMeshObjectComponent>();
		staticMesh->mModel = boxModel;
		staticMesh->mColor = glm::vec3(0.5f, 0.5f, 0.5f);

		world.AddObject(gameObject);
	}

	std::shared_ptr<Camera> camera = std::make_shared<Camera>();
	world.mRenderSystem->SetCamera(camera);

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
			SpawnBall(world, ballModel, camera->mTransform);
			window.TEMP_mPendingFire = false;
		}

		world.Update(frameTime);

		size_t currentObject = 0;
		size_t destroyedObjectCount = 0;
		size_t objectCount = world.mObjects.size();
		while (currentObject + destroyedObjectCount < objectCount)
		{
			if (world.mObjects[currentObject]->mTransform.Position.z < -20.0f)
			{
				std::swap(world.mObjects[currentObject], world.mObjects[objectCount - destroyedObjectCount - 1]);
				destroyedObjectCount++;

				world.RemoveObject(world.mObjects[objectCount - destroyedObjectCount]);
			}
			else
			{
				currentObject++;
			}
		}

		world.mObjects.resize(objectCount - destroyedObjectCount);

		if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
		{
			renderer.BeginSwapChainRenderPass(commandBuffer);

			world.mRenderSystem->RenderScene(commandBuffer);

			renderer.EndSwapChainRenderPass(commandBuffer);
			renderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(device.GetDevice());

	return 0;
}
