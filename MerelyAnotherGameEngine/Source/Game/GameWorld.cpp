#include "Game/CameraComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Game/SpriteObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Game/TextObjectComponent.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Rendering/Systems/TextRenderSystem.h"
#include "Vulkan/Renderer.h"

void GameWorld::Update(f32 deltaTime)
{
	mIsCurrentlyUpdatingObjects = true;
	for (GameObject* object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePrePhysics(deltaTime);
	}

	mIsCurrentlyUpdatingObjects = false;
	for (GameObject* newObject : mNewObjects)
	{
		newObject->UpdatePrePhysics(deltaTime);
		mObjects.Add(newObject);
	}
	mNewObjects.Empty();

	mPhysicsSystem.Update(deltaTime);

	mIsCurrentlyUpdatingObjects = true;
	for (GameObject* object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePostPhysics(deltaTime);
	}

	mIsCurrentlyUpdatingObjects = false;
	for (GameObject* newObject : mNewObjects)
	{
		newObject->UpdatePostPhysics(deltaTime);
		mObjects.Add(newObject);
	}
	mNewObjects.Empty();

	for (u32 i = 0; i < mObjects.GetSize(); ++i)
	{
		if (!mObjects[i]->mIsDestoryed)
			continue;

		mObjects[i]->OnRemovedFromWorld(*this);
		mObjects.RemoveAtSwap(i--);
	}
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
	mage::Array<SpriteRenderData> spriteData;
	mage::Array<TextRenderData> textData;

	sceneData.LightDirection = glm::vec3(-3.0f, 2.0f, -2.5f);
	sceneData.AmbientLightIntensity = 0.05f;

	bool foundCamera = false;
	for (GameObject const* object : mObjects)
	{
		for (StaticMeshObjectComponent const* staticMeshComp : object->GetComponentsOfClass<StaticMeshObjectComponent>())
			sceneData.Meshes.AddConstruct(
				staticMeshComp->GetTransform().Matrix(),
				staticMeshComp->GetMesh(),
				staticMeshComp->GetTexture());

		for (SpriteObjectComponent const* spriteComp : object->GetComponentsOfClass<SpriteObjectComponent>())
			spriteData.AddConstruct(
				spriteComp->GetScreenCoordsMin(),
				spriteComp->GetScreenCoordsMax(),
				spriteComp->GetTextureCoordsMin(),
				spriteComp->GetTextureCoordsMax(),
				spriteComp->GetTexture());

		for (TextObjectComponent const* textComp : object->GetComponentsOfClass<TextObjectComponent>())
			textData.AddConstruct(
				textComp->GetText(),
				textComp->GetColor(),
				textComp->GetScreenPosition(),
				textComp->GetScale(),
				textComp->GetFont());

		if (!foundCamera)
			for (CameraComponent const* cameraComp : object->GetComponentsOfClass<CameraComponent>())
			{
				sceneData.ViewTransform = cameraComp->GetViewTransform();
				foundCamera = true;
				break;
			}
	}

	renderer.RenderFrame([this, &sceneData, &spriteData, &textData](Vulkan::RenderFrameData const& inFrameData)
		{
			f32 aspectRatio = f32(inFrameData.Extent.width) / f32(inFrameData.Extent.height);
			sceneData.ProjectionTransform = CalcProjectionTransform(0.1f, 1000.0f, glm::radians(90.0f), aspectRatio);

			mMeshRenderSystem.RenderMeshes(inFrameData, sceneData);
			mSpriteRenderSystem.RenderSprites(inFrameData, spriteData);
			mTextRenderSystem.RenderText(inFrameData, textData);
		});
}
