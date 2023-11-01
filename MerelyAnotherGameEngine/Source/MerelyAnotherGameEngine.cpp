#include "Core/Asserts.h"
#include "Rendering/MV_Camera.h"
#include "Rendering/MV_Device.h"
#include "Rendering/MV_Object.h"
#include "Rendering/MV_Renderer.h"
#include "Rendering/MV_TestRenderSystem.h"
#include "Rendering/MV_Window.h"

#include <array>
#include <chrono>
#include <memory>

#include <PxPhysicsAPI.h>

class TestPhysicsSystem
{
public:
	TestPhysicsSystem()
	{
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);

		mPvd = physx::PxCreatePvd(*mFoundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), true, mPvd);

		physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -9.81f);
		mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = mDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		mScene = mPhysics->createScene(sceneDesc);

		mDefaultMaterial = mPhysics->createMaterial(0.1f, 0.05f, 0.5f);

		physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
	}

	~TestPhysicsSystem()
	{
		for (physx::PxActor* actor : staticActors)
		{
			PX_RELEASE(actor);
		}

		for (physx::PxActor* actor : dynamicActors)
		{
			PX_RELEASE(actor);
		}

		PX_RELEASE(mDefaultMaterial);

		PX_RELEASE(mScene);
		PX_RELEASE(mDispatcher);
		PX_RELEASE(mPhysics);

		if (mPvd)
		{
			physx::PxPvdTransport* transport = mPvd->getTransport();
			mPvd->release();
			mPvd = NULL;
			PX_RELEASE(transport);
		}

		PX_RELEASE(mFoundation);
	}

	void Tick(float deltaTime)
	{
		mScene->simulate(deltaTime);
		mScene->fetchResults(true);

		for (size_t i = 0; i < dynamicActors.size(); i++)
		{
			physx::PxTransform pTransform = dynamicActors[i]->getGlobalPose();
			transformComponents[i]->mTransform.Position = reinterpret_cast<const glm::vec3&>(pTransform.p);
			transformComponents[i]->mTransform.Rotation.S = pTransform.q.w;
			transformComponents[i]->mTransform.Rotation.XY = -pTransform.q.z;
			transformComponents[i]->mTransform.Rotation.YZ = -pTransform.q.y;
			transformComponents[i]->mTransform.Rotation.ZX = -pTransform.q.x;
		}
	}

	void AddStaticObject(const mage::Transform& transform, const physx::PxGeometry& geometry)
	{
		physx::PxTransform pTransform;
		pTransform.p = reinterpret_cast<const physx::PxVec3&>(transform.Position);
		pTransform.q.x = -transform.Rotation.YZ;
		pTransform.q.y = -transform.Rotation.ZX;
		pTransform.q.z = -transform.Rotation.XY;
		pTransform.q.w = transform.Rotation.S;

		physx::PxShape* shape = mPhysics->createShape(geometry, *mDefaultMaterial, true);

		physx::PxRigidStatic* actor = mPhysics->createRigidStatic(pTransform);
		actor->attachShape(*shape);
		mScene->addActor(*actor);

		shape->release();

		staticActors.push_back(actor);
	}

	void AddDynamicObject(MV::TransformComponent& transformComponent, const physx::PxGeometry& geometry, glm::vec3 velocity)
	{
		physx::PxTransform transform;
		transform.p = reinterpret_cast<const physx::PxVec3&>(transformComponent.mTransform.Position);
		transform.q.x = -transformComponent.mTransform.Rotation.YZ;
		transform.q.y = -transformComponent.mTransform.Rotation.ZX;
		transform.q.z = -transformComponent.mTransform.Rotation.XY;
		transform.q.w = transformComponent.mTransform.Rotation.S;

		physx::PxShape* shape = mPhysics->createShape(geometry, *mDefaultMaterial, true);

		physx::PxRigidDynamic* actor = mPhysics->createRigidDynamic(transform);
		actor->attachShape(*shape);
		actor->setLinearVelocity(reinterpret_cast<const physx::PxVec3&>(velocity));
		mScene->addActor(*actor);

		shape->release();

		dynamicActors.push_back(actor);
		transformComponents.push_back(&transformComponent);
	}

private:
	physx::PxDefaultAllocator mAllocator;
	physx::PxDefaultErrorCallback mErrorCallback;
	physx::PxFoundation* mFoundation = nullptr;
	physx::PxPhysics* mPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
	physx::PxScene* mScene = nullptr;
	physx::PxMaterial* mDefaultMaterial = nullptr;
	physx::PxPvd* mPvd = nullptr;

	std::vector<physx::PxRigidStatic*> staticActors;
	std::vector<physx::PxRigidDynamic*> dynamicActors;
	std::vector<MV::TransformComponent*> transformComponents;
};

static constexpr int gWindowWidth = 1280;
static constexpr int gWindowHeight = 720;

void MoveByInput(MV::Window& window, float deltaTime, MV::TransformComponent& transformComponent)
{
	glm::vec3 movement(0.0f);

	if (window.GetKeyState(GLFW_KEY_D) == GLFW_PRESS) movement.x += 1.0f;
	if (window.GetKeyState(GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1.0f;

	if (window.GetKeyState(GLFW_KEY_W) == GLFW_PRESS) movement.y += 1.0f;
	if (window.GetKeyState(GLFW_KEY_S) == GLFW_PRESS) movement.y -= 1.0f;

	movement = transformComponent.mTransform.Matrix() * glm::vec4(movement, 0.0f);

	if (window.GetKeyState(GLFW_KEY_E) == GLFW_PRESS) movement.z += 1.0f;
	if (window.GetKeyState(GLFW_KEY_Q) == GLFW_PRESS) movement.z -= 1.0f;

	transformComponent.mTransform.Position += 5.0f * deltaTime * movement;
}

void RotateCameraByInput(MV::Window& window, glm::vec2& accumulatedRotation, float deltaTime, MV::TransformComponent& transformComponent)
{
	accumulatedRotation += 0.01f * window.ConsumeMovement();
	accumulatedRotation.y = glm::clamp(accumulatedRotation.y, -glm::radians(80.0f), glm::radians(80.0f));

	transformComponent.mTransform.Rotation = mage::Rotor::Combine(
		mage::Rotor::FromAxisAndAngle({ 0.0f, 0.0f, 1.0f }, accumulatedRotation.x),
		mage::Rotor::FromAxisAndAngle({ 1.0f, 0.0f, 0.0f }, accumulatedRotation.y));
}

void SpawnBall(std::vector<std::shared_ptr<MV::Object>>& objects, std::shared_ptr<MV::Model>& model, TestPhysicsSystem& physicsSystem, const mage::Transform& transform)
{
	std::shared_ptr<MV::Object> object = std::make_shared<MV::Object>();
	object->mModel = model;
	object->mColor = glm::vec3(0.3f, 0.3f, 1.0f);

	glm::mat4 matrix = transform.Matrix();
	glm::vec3 forward = matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

	object->mTransformComponent.mTransform = transform;

	physicsSystem.AddDynamicObject(object->mTransformComponent, physx::PxSphereGeometry(1.0f), 50.0f * forward);

	objects.push_back(std::move(object));
}

int main()
{
	MV::Window window{ gWindowWidth, gWindowHeight, "Window" };
	MV::Device device{ window };
	MV::Renderer renderer{ window, device };

	TestPhysicsSystem physicsSystem;

	std::vector<std::shared_ptr<MV::Object>> objects;

	// these models are not in source control, sorry
	std::shared_ptr<MV::Model> ballModel = MV::Model::CreateFromFile(device, "Models/ball.obj");
	std::shared_ptr<MV::Model> boxModel = MV::Model::CreateFromFile(device, "Models/box.obj");

	for (int32_t i = 0; i < 3; i++)
	{
		for (int32_t j = 0; j < 3; j++)
		{
			std::shared_ptr<MV::Object> object = std::make_shared<MV::Object>();
			object->mModel = ballModel;
			object->mColor = glm::vec3(0.3f, 0.3f, 1.0f);
			object->mTransformComponent.mTransform.Position = glm::vec3(10.0f * i - 10.0f, 10.0f * j - 10.0f, 5.0f);

			physicsSystem.AddDynamicObject(object->mTransformComponent, physx::PxSphereGeometry(1.0f), glm::vec3(15.0f, 10.0f, 0.0f));

			objects.push_back(std::move(object));
		}
	}

	// floor
	{
		std::shared_ptr<MV::Object> object = std::make_shared<MV::Object>();
		object->mModel = boxModel;
		object->mColor = glm::vec3(0.5f, 0.5f, 0.5f);
		object->mTransformComponent.mTransform.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		object->mTransformComponent.mTransform.Scale = glm::vec3(20.0f, 20.0f, 1.0f);

		physicsSystem.AddStaticObject(object->mTransformComponent.mTransform, physx::PxBoxGeometry(20.0f, 20.0f, 1.0f));

		objects.push_back(std::move(object));
	}

	// back
	{
		std::shared_ptr<MV::Object> object = std::make_shared<MV::Object>();
		object->mModel = boxModel;
		object->mColor = glm::vec3(0.5f, 0.5f, 0.5f);
		object->mTransformComponent.mTransform.Position = glm::vec3(0.0f, 19.0f, 4.0f);
		object->mTransformComponent.mTransform.Scale = glm::vec3(20.0f, 1.0f, 5.0f);

		physicsSystem.AddStaticObject(object->mTransformComponent.mTransform, physx::PxBoxGeometry(20.0f, 1.0f, 5.0f));

		objects.push_back(std::move(object));
	}

	// left
	{
		std::shared_ptr<MV::Object> object = std::make_shared<MV::Object>();
		object->mModel = boxModel;
		object->mColor = glm::vec3(0.5f, 0.5f, 0.5f);
		object->mTransformComponent.mTransform.Position = glm::vec3(-19.0f, 0.0f, 4.0f);
		object->mTransformComponent.mTransform.Scale = glm::vec3(1.0f, 20.0f, 5.0f);

		physicsSystem.AddStaticObject(object->mTransformComponent.mTransform, physx::PxBoxGeometry(1.0f, 20.0f, 5.0f));

		objects.push_back(std::move(object));
	}

	// right
	{
		std::shared_ptr<MV::Object> object = std::make_shared<MV::Object>();
		object->mModel = boxModel;
		object->mColor = glm::vec3(0.5f, 0.5f, 0.5f);
		object->mTransformComponent.mTransform.Position = glm::vec3(19.0f, 0.0f, 4.0f);
		object->mTransformComponent.mTransform.Scale = glm::vec3(1.0f, 20.0f, 5.0f);

		physicsSystem.AddStaticObject(object->mTransformComponent.mTransform, physx::PxBoxGeometry(1.0f, 20.0f, 5.0f));

		objects.push_back(std::move(object));
	}

	std::shared_ptr<MV::Camera> camera = std::make_shared<MV::Camera>();

	MV::TestRenderSystem renderSystem(device, renderer.GetSwapchainRenderPass());
	renderSystem.SetCamera(camera);

	camera->mTransformComponent.mTransform.Position = glm::vec3(0.0f, -30.0f, 10.0f);

	glm::vec2 viewRotation = glm::vec2(0.0f);

	std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

	float accTime = 0.0f;
	constexpr float period = 1.5f;

	while (!window.ShouldClose())
	{
		MV::Window::PollEvents();

		const std::chrono::steady_clock::time_point newTime = std::chrono::high_resolution_clock::now();
		const float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		RotateCameraByInput(window, viewRotation, frameTime, camera->mTransformComponent);
		MoveByInput(window, frameTime, camera->mTransformComponent);
		camera->SetPerspectiveParams(0.1f, 100.0f, glm::radians(90.0f), renderer.GetAspectRatio());

		if (window.TEMP_mPendingFire)
		{
			SpawnBall(objects, ballModel, physicsSystem, camera->mTransformComponent.mTransform);
			window.TEMP_mPendingFire = false;
		}

		physicsSystem.Tick(frameTime);

		accTime += frameTime;
		while (accTime >= period)
		{
			accTime -= period;
		}

		const float rotationFactor = 1.0f / (1.0f + glm::exp(-15.0f * (accTime / period - 0.5f)));
		const float rotation = rotationFactor * glm::radians(180.0f);

		if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
		{
			renderer.BeginSwapChainRenderPass(commandBuffer);

			renderSystem.RenderObjects(commandBuffer, objects);

			renderer.EndSwapChainRenderPass(commandBuffer);
			renderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(device.GetDevice());

	return 0;
}
