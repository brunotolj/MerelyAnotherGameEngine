#include "Asserts.h"
#include "MV/MV_Device.h"
#include "MV/MV_Object.h"
#include "MV/MV_Renderer.h"
#include "MV/MV_TestRenderSystem.h"
#include "MV/MV_Window.h"

#include <array>
#include <memory>

struct PushConstantData
{
	alignas(64) glm::mat4 Transform{1.0f};
	alignas(16) glm::vec3 Color{0.0f};
};

class App : public NonCopyableClass
{
public:
	static constexpr int WIDTH = 1280;
	static constexpr int HEIGHT = 720;

public:
	App()
	{
		LoadObjects();
	}

	~App()
	{
	}

	void Run()
	{
		MV::TestRenderSystem renderSystem(mDevice, mRenderer.GetSwapchainRenderPass());

		while (!mWindow.ShouldClose())
		{
			MV::Window::PollEvents();

			if (VkCommandBuffer commandBuffer = mRenderer.BeginFrame())
			{
				mRenderer.BeginSwapChainRenderPass(commandBuffer);

				renderSystem.RenderObjects(commandBuffer, mObjects);
				
				mRenderer.EndSwapChainRenderPass(commandBuffer);
				mRenderer.EndFrame();
			}
		}

		vkDeviceWaitIdle(mDevice.GetDevice());
	}

private:
	MV::Window mWindow{ WIDTH, HEIGHT, "Window" };
	MV::Device mDevice{ mWindow };
	MV::Renderer mRenderer{ mWindow, mDevice };

	std::vector<std::shared_ptr<MV::Object>> mObjects;

private:
	void LoadObjects()
	{
		std::vector<MV::Model::Vertex> vertices {
			{{ -0.4, -0.4 }},
			{{ 0.4, 0.4 }},
			{{ -0.4, 0.4 }}};

		std::shared_ptr<MV::Model> model = std::make_shared<MV::Model>(mDevice, vertices);

		std::shared_ptr<MV::Object> triangle = std::make_shared<MV::Object>();
		triangle->model = model;
		triangle->color = { 0.0f, 0.8f, 0.0f };

		mObjects.push_back(std::move(triangle));
	}
};

int main()
{
	App a;
	a.Run();
	return 0;
}
