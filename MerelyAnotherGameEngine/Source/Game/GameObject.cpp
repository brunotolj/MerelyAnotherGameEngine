#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Framework/GameWorld.h"

GameObject::~GameObject()
{
	for (GameObjectComponentBase* component : mComponents)
	{
		delete component;
	}
}

void GameObject::OnAddedToWorld()
{
	for (GameObjectComponentBase* component : mComponents)
	{
		component->OnOwnerAddedToWorld();
	}
}

void GameObject::OnRemovedFromWorld()
{
	for (GameObjectComponentBase* component : mComponents)
	{
		component->OnOwnerRemovedFromWorld();
	}
}

void GameObject::Update(f32 deltaTime)
{
	for (GameObjectComponentBase* component : mComponents)
	{
		component->Update(deltaTime);
	}
}

mage::Transform TransformableObject::GetTransform() const
{
	return mWorld.GetComponent<TransformTree>()->GetGlobalTransform(mTransformId);
}

void TransformableObject::SetTransform(mage::Transform inTransform)
{
	mWorld.GetComponent<TransformTree>()->SetGlobalTransform(mTransformId, inTransform);
}

void TransformableObject::Destroy()
{
	GameObject::Destroy();

	mWorld.GetComponent<TransformTree>()->RemoveEntry(mTransformId);
	mTransformId = mage::InvalidIndex;
}

void TransformableObject::InitTransform(mage::Transform inInitialTransform)
{
	mTransformId = mWorld.GetComponent<TransformTree>()->AddEntry(inInitialTransform);
}
