#include "Asserts.h"
#include "MV/MV_Device.h"
#include "MV/MV_Object.h"
#include "MV/MV_Renderer.h"
#include "MV/MV_TestRenderSystem.h"
#include "MV/MV_Window.h"

#include <array>
#include <memory>

static constexpr int WIDTH = 1280;
static constexpr int HEIGHT = 720;

int main()
{
	MV::Window window{ WIDTH, HEIGHT, "Window" };
	MV::Device device{ window };
	MV::Renderer renderer{ window, device };

	std::vector<std::shared_ptr<MV::Object>> objects;

	std::vector<MV::Model::Vertex> vertices {
		{{ -0.4, -0.4 }},
		{ { 0.4, 0.4 } },
		{ { -0.4, 0.4 } }};

	std::shared_ptr<MV::Model> model = std::make_shared<MV::Model>(device, vertices);

	std::shared_ptr<MV::Object> triangle = std::make_shared<MV::Object>();
	triangle->pubModel = model;
	triangle->pubColor = { 0.0f, 0.8f, 0.0f };

	objects.push_back(std::move(triangle));

	MV::TestRenderSystem renderSystem(device, renderer.GetSwapchainRenderPass());

	while (!window.ShouldClose())
	{
		MV::Window::PollEvents();

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
