#include "Game/TextObjectComponent.h"

TextObjectComponent::TextObjectComponent(GameObject& owner, const ComponentTemplate<TextObjectComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mText(creationTemplate.Text),
	mColor(creationTemplate.Color),
	mScreenPosition(creationTemplate.ScreenPosition),
	mScale(creationTemplate.Scale),
	mFont(creationTemplate.Font)
{
}
