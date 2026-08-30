#pragma once

#include "Framework/GameWorld.h"

enum class TransformTreeEntryFlags : u8
{
	EntryExists = 1 << 0,
	GlobalDirty = 1 << 1
};

using TransformTreeEntryId = u32;

struct TransformTreeEntry
{
	mage::Transform LocalTransform;
	mutable mage::Transform GlobalTransform;

	TransformTreeEntryId ParentId = mage::InvalidIndex;
	mage::Array<TransformTreeEntryId> ChildIds;

	mutable Flagset<TransformTreeEntryFlags> Flags;
};

class TransformTree : public GameUtility
{
public:
	TransformTree(GameWorld& inWorld) : GameUtility(inWorld) {}
	virtual ~TransformTree() {}

	TransformTreeEntryId AddEntry(mage::Transform inInitialTransform = mage::Transform(), TransformTreeEntryId inParentId = mage::InvalidIndex);
	bool RemoveEntry(TransformTreeEntryId inEntryId);

	bool AttachToParent(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId);
	bool DetachFromParent(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId);
	u32 DetachAllChildren(TransformTreeEntryId inEntryId);

	mage::Transform GetLocalTransform(TransformTreeEntryId inEntryId) const;
	void SetLocalTransform(TransformTreeEntryId inEntryId, mage::Transform inTransform);

	mage::Transform GetGlobalTransform(TransformTreeEntryId inEntryId) const;
	void SetGlobalTransform(TransformTreeEntryId inEntryId, mage::Transform inTransform);

private:
	void AttachInternal(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId);
	void DetachInternal(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId);

	void MarkGlobalDirty(TransformTreeEntryId inEntryId);
	void CacheGlobalTransform(TransformTreeEntryId inEntryId) const;

	mage::Array<TransformTreeEntry> mEntries;
};
