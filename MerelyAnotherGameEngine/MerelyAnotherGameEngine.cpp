#include "Core/Asserts.h"
#include "MV/MV_Camera.h"
#include "MV/MV_Device.h"
#include "MV/MV_Object.h"
#include "MV/MV_Renderer.h"
#include "MV/MV_TestRenderSystem.h"
#include "MV/MV_Window.h"

#include <array>
#include <memory>

static constexpr int gWindowWidth = 1280;
static constexpr int gWindowHeight = 720;

int main()
{
	MV::Window window{ gWindowWidth, gWindowHeight, "Window" };
	MV::Device device{ window };
	MV::Renderer renderer{ window, device };

	std::vector<std::shared_ptr<MV::Object>> objects;

	std::vector<MV::Model::Vertex> vertices {
		{{ -0.25f, -0.25f, 0.25f }},
		{{  0.25f, -0.25f, 0.25f }},
		{{  0.25f,  0.25f, 0.25f }},
		{{ -0.25f, -0.25f, 0.25f }},
		{{  0.25f,  0.25f, 0.25f }},
		{{ -0.25f,  0.25f, 0.25f }}};

	std::shared_ptr<MV::Model> model = std::make_shared<MV::Model>(device, vertices);

	glm::vec3 colors[6] = {
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f }};

	mage::Rotor rotors[6] = {
		mage::Rotor::FromAxisAndAngle({ 0.0f, 1.0f, 0.0f }, glm::radians(0.0f)),
		mage::Rotor::FromAxisAndAngle({ 0.0f, 1.0f, 0.0f }, glm::radians(90.0f)),
		mage::Rotor::FromAxisAndAngle({ 0.0f, 1.0f, 0.0f }, glm::radians(180.0f)),
		mage::Rotor::FromAxisAndAngle({ 0.0f, 1.0f, 0.0f }, glm::radians(-90.0f)),
		mage::Rotor::FromAxisAndAngle({ 1.0f, 0.0f, 0.0f }, glm::radians(90.0f)),
		mage::Rotor::FromAxisAndAngle({ 1.0f, 0.0f, 0.0f }, glm::radians(-90.0f))};

	mage::Rotor tilt = mage::Rotor::Combine(
		mage::Rotor::FromAxisAndAngle({ 1.0f, 0.0f, 0.0f }, glm::radians(-36.0f)),
		mage::Rotor::FromAxisAndAngle({ 0.0f, 1.0f, 0.0f }, glm::radians(45.0f)));

	for (int32_t i = 0; i < 6; i++)
	{
		std::shared_ptr<MV::Object> square = std::make_shared<MV::Object>();
		
		square->mModel = model;
		square->mColor = colors[i];
		square->mTransformComponent.mTransform.Position = { 0.0f, 0.0f, 2.0f };
		
		objects.push_back(std::move(square));

		rotors[i] = mage::Rotor::Combine(tilt, rotors[i]);
	}

	std::shared_ptr<MV::Camera> camera = std::make_shared<MV::Camera>();

	MV::TestRenderSystem renderSystem(device, renderer.GetSwapchainRenderPass());
	renderSystem.SetCamera(camera);

	camera->mTransformComponent.mTransform.Position = glm::vec3(0.0f, 1.0f, 0.0f);

	float kok = 0.0f;

	while (!window.ShouldClose())
	{
		MV::Window::PollEvents();

		kok += 0.002f;

		if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
		{
			renderer.BeginSwapChainRenderPass(commandBuffer);

			for (int32_t i = 0; i < 6; i++)
			{
				objects[i]->mTransformComponent.mTransform.Rotation = mage::Rotor::Combine(mage::Rotor::FromAxisAndAngle({ 0.0f, 1.0f, 0.0f }, kok), rotors[i]);
			}

			camera->mTransformComponent.mTransform.Rotation = mage::Rotor::FromAxisAndAngle(glm::vec3(0.0f, 0.0f, 1.0f), 2.0f * kok);
			camera->SetPerspectiveParams(0.1f, 100.0f, glm::radians(90.0f), renderer.GetAspectRatio());

			renderSystem.RenderObjects(commandBuffer, objects);

			renderer.EndSwapChainRenderPass(commandBuffer);
			renderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(device.GetDevice());

	return 0;
}
