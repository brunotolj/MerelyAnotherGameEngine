#include "Game/CameraComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Game/InputSystem.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Vulkan/Renderer.h"

GameWorld::GameWorld(
	std::unique_ptr<InputSystem>&& inputSystem,
	std::unique_ptr<PhysicsSystem>&& physicsSystem,
	std::unique_ptr<MeshRenderSystem>&& meshRenderSystem,
	std::unique_ptr<SpriteRenderSystem>&& spriteRenderSystem) :
	mInputSystem(std::move(inputSystem)),
	mPhysicsSystem(std::move(physicsSystem)),
	mMeshRenderSystem(std::move(meshRenderSystem)),
	mSpriteRenderSystem(std::move(spriteRenderSystem))
{
}

void GameWorld::Update(f32 deltaTime)
{
	mIsCurrentlyUpdatingObjects = true;
	for (const std::shared_ptr<GameObject>& object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePrePhysics(deltaTime);
	}

	mIsCurrentlyUpdatingObjects = false;
	for (std::shared_ptr<GameObject>& newObject : mNewObjects)
	{
		newObject->UpdatePrePhysics(deltaTime);
		mObjects.push_back(std::move(newObject));
	}
	mNewObjects.clear();

	mPhysicsSystem->Update(deltaTime);

	mIsCurrentlyUpdatingObjects = true;
	for (const std::shared_ptr<GameObject>& object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePostPhysics(deltaTime);
	}

	mIsCurrentlyUpdatingObjects = false;
	for (std::shared_ptr<GameObject>& newObject : mNewObjects)
	{
		newObject->UpdatePostPhysics(deltaTime);
		mObjects.push_back(std::move(newObject));
	}
	mNewObjects.clear();

	u64 currentObject = 0;
	u64 destroyedObjectCount = 0;
	const u64 totalObjectCount = mObjects.size();

	while (currentObject + destroyedObjectCount < mObjects.size())
	{
		if (mObjects[currentObject]->mIsDestoryed)
		{
			std::swap(mObjects[currentObject], mObjects[totalObjectCount - destroyedObjectCount - 1]);
			destroyedObjectCount++;

			RemoveObject(mObjects[totalObjectCount - destroyedObjectCount]);
		}
		else
		{
			currentObject++;
		}
	}

	mObjects.resize(totalObjectCount - destroyedObjectCount);
}

glm::mat4 CalcProjectionTransform(f32 nearPlane, f32 farPlane, f32 horizontalFOV, f32 aspectRatio)
{
	mage_check(nearPlane >= 0.0f && farPlane > nearPlane);
	mage_check(horizontalFOV > 0.0f && glm::degrees(horizontalFOV) < 180.0f);

	const f32 fovFactor = 1.0f / glm::tan(horizontalFOV / 2.0f);
	const f32 planeDelta = farPlane - nearPlane;

	return
	{
		{ fovFactor, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, farPlane / planeDelta, 1.0f },
		{ 0.0f, -fovFactor * aspectRatio, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -farPlane * nearPlane / planeDelta, 0.0f }
	};
}

void GameWorld::Render(Vulkan::Renderer& renderer) const
{
	SceneRenderData sceneData;

	sceneData.LightDirection = glm::vec3(3.0f, 2.0f, -2.5f);
	sceneData.AmbientLightIntensity = 0.05f;

	bool foundCamera = false;
	for (std::shared_ptr<GameObject> const& object : mObjects)
	{
		for (std::shared_ptr<StaticMeshObjectComponent> const& staticMeshComp : object->GetComponentsOfClass<StaticMeshObjectComponent>())
		{
			sceneData.Meshes.emplace_back(staticMeshComp->GetModel().get(),
				staticMeshComp->GetTransform().Matrix(),
				staticMeshComp->GetColor(),
				0);
		}

		if (!foundCamera)
			for (std::shared_ptr<CameraComponent> const& cameraComp : object->GetComponentsOfClass<CameraComponent>())
			{
				sceneData.ViewTransform = cameraComp->GetViewTransform();
				foundCamera = true;
				break;
			}
	}

	std::vector<SpriteRenderData> spriteData;

	{
		SpriteRenderData data;
		data.ScreenCoordsMin = { 50.0f, 50.0f };
		data.ScreenCoordsMax = { 150.0f, 150.0f };
		data.TextureCoordsMin = { 0.0f, 0.0f };
		data.TextureCoordsMax = { 1.0f, 1.0f };
		data.TextureIndex = 0;

		spriteData.push_back(data);
	}

	renderer.RenderFrame([this, &sceneData, &spriteData](Vulkan::RenderFrameData const& inFrameData)
		{
			f32 aspectRatio = f32(inFrameData.Extent.width) / f32(inFrameData.Extent.height);
			sceneData.ProjectionTransform = CalcProjectionTransform(0.1f, 1000.0f, glm::radians(90.0f), aspectRatio);

			mMeshRenderSystem->RenderMeshes(inFrameData, sceneData);
			mSpriteRenderSystem->RenderSprites(inFrameData, spriteData);
		});
}

void GameWorld::AddObject(const std::shared_ptr<GameObject>& object)
{
	if (mIsCurrentlyUpdatingObjects)
		mNewObjects.push_back(object);
	else
		mObjects.push_back(object);

	object->OnAddedToWorld(*this);
}

void GameWorld::RemoveObject(const std::shared_ptr<GameObject>& object)
{
	object->OnRemovedFromWorld(*this);
}
