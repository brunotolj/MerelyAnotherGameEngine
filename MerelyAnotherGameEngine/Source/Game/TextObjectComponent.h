#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Assets/Font.h"

template<>
struct ComponentTemplate<class TextObjectComponent>
{
	mage::StringView Text;
	glm::vec4 Color;
	glm::vec2 ScreenPosition;
	f32 Scale;
	AssetHandle<Font> Font;
};

class TextObjectComponent : public GameObjectComponent<GameObject>
{
public:
	TextObjectComponent(GameObject& owner, const ComponentTemplate<TextObjectComponent>& creationTemplate);

	mage::StringView GetText() const { return mText; }
	glm::vec4 GetColor() const { return mColor; }
	glm::vec2 GetScreenPosition() const { return mScreenPosition; }
	f32 GetScale() const { return mScale; }
	AssetHandle<Font> GetFont() const { return mFont; }

private:
	mage::StringView mText;
	glm::vec4 mColor;
	glm::vec2 mScreenPosition;
	f32 mScale;
	AssetHandle<Font> mFont;
};
