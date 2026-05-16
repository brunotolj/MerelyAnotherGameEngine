#include "Game/SpriteObjectComponent.h"

SpriteObjectComponent::SpriteObjectComponent(GameObject& owner, const ComponentTemplate<SpriteObjectComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mScreenCoordsMin(creationTemplate.ScreenCoordsMin),
	mScreenCoordsMax(creationTemplate.ScreenCoordsMax),
	mTextureCoordsMin(creationTemplate.TextureCoordsMin),
	mTextureCoordsMax(creationTemplate.TextureCoordsMax),
	mTexture(creationTemplate.Texture)
{
}
