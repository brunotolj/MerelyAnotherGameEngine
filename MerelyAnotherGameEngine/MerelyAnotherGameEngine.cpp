#include "Asserts.h"
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
		{ { -0.4f, -0.4f, -0.3f } },
		{ { 0.4f, 0.4f, 0.2f } },
		{ { -0.4f, 0.4f, 0.5f } }};

	std::shared_ptr<MV::Model> model = std::make_shared<MV::Model>(device, vertices);

	std::shared_ptr<MV::Object> triangle = std::make_shared<MV::Object>();
	triangle->mModel = model;
	triangle->mColor = { 0.5f, 0.2f, 0.8f };

	objects.push_back(std::move(triangle));

	MV::TestRenderSystem renderSystem(device, renderer.GetSwapchainRenderPass());

	while (!window.ShouldClose())
	{
		MV::Window::PollEvents();

		if (VkCommandBuffer commandBuffer = renderer.BeginFrame())
		{
			renderer.BeginSwapChainRenderPass(commandBuffer);

			objects[0]->mTransform.mRotation += 0.002f;

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
