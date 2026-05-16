#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Assets/Texture.h"

template<>
struct ComponentTemplate<class SpriteObjectComponent>
{
	glm::vec2 ScreenCoordsMin;
	glm::vec2 ScreenCoordsMax;
	glm::vec2 TextureCoordsMin;
	glm::vec2 TextureCoordsMax;
	AssetHandle<Texture> Texture;
};

class SpriteObjectComponent : public GameObjectComponent<GameObject>
{
public:
	SpriteObjectComponent(GameObject& owner, const ComponentTemplate<SpriteObjectComponent>& creationTemplate);

	glm::vec2 GetScreenCoordsMin() const { return mScreenCoordsMin; }
	glm::vec2 GetScreenCoordsMax() const { return mScreenCoordsMax; }
	glm::vec2 GetTextureCoordsMin() const { return mTextureCoordsMin; }
	glm::vec2 GetTextureCoordsMax() const { return mTextureCoordsMax; }
	AssetHandle<Texture> GetTexture() const { return mTexture; }

private:
	glm::vec2 mScreenCoordsMin;
	glm::vec2 mScreenCoordsMax;
	glm::vec2 mTextureCoordsMin;
	glm::vec2 mTextureCoordsMax;
	AssetHandle<Texture> mTexture;
};
