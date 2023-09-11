#include "Core/Asserts.h"
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
		{ { -0.25f, -0.25f, 0.25f } },
		{ {  0.25f, -0.25f, 0.25f } },
		{ {  0.25f,  0.25f, 0.25f } },
		{ { -0.25f, -0.25f, 0.25f } },
		{ {  0.25f,  0.25f, 0.25f } },
		{ { -0.25f,  0.25f, 0.25f } }};

	std::shared_ptr<MV::Model> model = std::make_shared<MV::Model>(device, vertices);

	glm::vec3 colors[6] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{1.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 1.0f}};
	
	constexpr float pi = glm::pi<float>();

	mage::Rotor rotors[6] = {
		mage::Rotor::FromAxisAndAngle({0.0f, 1.0f, 0.0f}, 0.0f),
		mage::Rotor::FromAxisAndAngle({0.0f, 1.0f, 0.0f}, 0.5f * pi),
		mage::Rotor::FromAxisAndAngle({0.0f, 1.0f, 0.0f}, pi),
		mage::Rotor::FromAxisAndAngle({0.0f, 1.0f, 0.0f}, -0.5f * pi),
		mage::Rotor::FromAxisAndAngle({1.0f, 0.0f, 0.0f}, 0.5f * pi),
		mage::Rotor::FromAxisAndAngle({1.0f, 0.0f, 0.0f}, -0.5f * pi)};

	for (int32_t i = 0; i < 6; i++)
	{
		std::shared_ptr<MV::Object> square = std::make_shared<MV::Object>();
		
		square->mModel = model;
		square->mColor = colors[i];
		square->mTransform.mPosition = {0.0f, 0.0f, 0.5f};
		
		objects.push_back(std::move(square));

		rotors[i] = mage::Rotor::Combine(mage::Rotor::FromAxisAndAngle({ 1.0f, 0.0f, 0.0f }, 0.2f * pi), rotors[i]);
	}

	MV::TestRenderSystem renderSystem(device, renderer.GetSwapchainRenderPass());

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
				objects[i]->mTransform.mRotation = mage::Rotor::Combine(mage::Rotor::FromAxisAndAngle({0.0f, 1.0f, 0.0f}, kok), rotors[i]);
			}

			float invAspectRatio = float(window.GetExtent().height) / window.GetExtent().width;

			glm::mat4 viewTransform = {
				{ invAspectRatio, 0.0f, 0.0f, 0.0f},
				{ 0.0f, 1.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f, 1.0f }
			};

			renderSystem.RenderObjects(commandBuffer, objects, viewTransform);

			renderer.EndSwapChainRenderPass(commandBuffer);
			renderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(device.GetDevice());

	return 0;
}
