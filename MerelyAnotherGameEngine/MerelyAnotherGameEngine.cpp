#include "Core/Asserts.h"
#include "MV/MV_Camera.h"
#include "MV/MV_Device.h"
#include "MV/MV_Object.h"
#include "MV/MV_Renderer.h"
#include "MV/MV_TestRenderSystem.h"
#include "MV/MV_Window.h"

#include <array>
#include <chrono>
#include <memory>

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

int main()
{
	MV::Window window{ gWindowWidth, gWindowHeight, "Window" };
	MV::Device device{ window };
	MV::Renderer renderer{ window, device };

	std::vector<std::shared_ptr<MV::Object>> objects;

	// this model is not in source control, sorry
	std::shared_ptr<MV::Model> model = MV::Model::CreateFromFile(device, "../models/heart.obj");

	std::shared_ptr<MV::Object> object = std::make_shared<MV::Object>();
	object->mModel = model;
	object->mColor = { 1.0f, 0.0f, 1.0f };
	objects.push_back(std::move(object));

	std::shared_ptr<MV::Camera> camera = std::make_shared<MV::Camera>();

	MV::TestRenderSystem renderSystem(device, renderer.GetSwapchainRenderPass());
	renderSystem.SetCamera(camera);

	camera->mTransformComponent.mTransform.Position = glm::vec3(0.0f, -5.0f, 0.0f);

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

			objects[0]->mTransformComponent.mTransform.Rotation = mage::Rotor::FromAxisAndAngle({ 0.0f, 0.0f, 1.0f }, rotation);

			RotateCameraByInput(window, viewRotation, frameTime, camera->mTransformComponent);
			MoveByInput(window, frameTime, camera->mTransformComponent);
			camera->SetPerspectiveParams(0.1f, 100.0f, glm::radians(90.0f), renderer.GetAspectRatio());

			renderSystem.RenderObjects(commandBuffer, objects);

			renderer.EndSwapChainRenderPass(commandBuffer);
			renderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(device.GetDevice());

	return 0;
}
