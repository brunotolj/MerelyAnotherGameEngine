#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

WorldComponentFactoryFunction TransformTreeFactoryFunction("TransformTree", [](GameWorld& inWorld, PropertyContainer const& inProperties)
{
	inWorld.CreateComponent<TransformTree>();
});

TransformTreeEntryId TransformTree::AddEntry(mage::Transform inInitialTransform, TransformTreeEntryId inParentId)
{
	TransformTreeEntryId newEntryId = mEntries.AddDefault();

	mEntries[newEntryId].Flags.Set(TransformTreeEntryFlags::EntryExists, TransformTreeEntryFlags::GlobalDirty);
	mEntries[newEntryId].LocalTransform = inInitialTransform;

	if (inParentId != mage::InvalidIndex)
	{
		bool doAttach = true;

		doAttach &= mage_ensure(inParentId < mEntries.GetSize() - 1);
		doAttach &= mage_ensure(mEntries[inParentId].Flags.HasAny(TransformTreeEntryFlags::EntryExists));

		if (doAttach)
			AttachInternal(newEntryId, inParentId);
	}

	return newEntryId;
}

bool TransformTree::RemoveEntry(TransformTreeEntryId inEntryId)
{
	if (!mage_ensure(inEntryId < mEntries.GetSize()))
		return false;

	if (!mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return false;

	if (!mage_ensure(mEntries[inEntryId].ChildIds.GetSize() == 0))
		return false;

	mEntries[inEntryId].Flags.Reset(TransformTreeEntryFlags::EntryExists);

	return true;
}

bool TransformTree::AttachToParent(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId)
{
	if (!mage_ensure(inParentId != inEntryId))
		return false;

	if (!mage_ensure(inEntryId < mEntries.GetSize()))
		return false;

	if (!mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return false;

	if (!mage_ensure(inParentId < mEntries.GetSize()))
		return false;
	
	if (!mage_ensure(mEntries[inParentId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return false;

	if (mEntries[inEntryId].ParentId != mage::InvalidIndex)
	{
		if (mEntries[inEntryId].ParentId == inParentId)
			return true;

		DetachInternal(inEntryId, mEntries[inEntryId].ParentId);
	}

	AttachInternal(inEntryId, inParentId);
	return true;
}

bool TransformTree::DetachFromParent(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId)
{
	if (!mage_ensure(inParentId != inEntryId))
		return false;

	if (!mage_ensure(inEntryId < mEntries.GetSize()))
		return false;

	if (!mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return false;

	if (!mage_ensure(inParentId < mEntries.GetSize()))
		return false;

	if (!mage_ensure(mEntries[inParentId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return false;

	DetachInternal(inEntryId, inParentId);
	return true;
}

u32 TransformTree::DetachAllChildren(TransformTreeEntryId inEntryId)
{
	if (!mage_ensure(inEntryId < mEntries.GetSize()))
		return 0;

	if (!mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return 0;

	u32 childrenCount = mEntries[inEntryId].ChildIds.GetSize();

	while (mEntries[inEntryId].ChildIds.GetSize())
		DetachInternal(mEntries[inEntryId].ChildIds.GetFirst(), inEntryId);

	return childrenCount;
}

mage::Transform TransformTree::GetLocalTransform(TransformTreeEntryId inEntryId) const
{
	if (!mage_ensure(inEntryId < mEntries.GetSize()))
		return mage::Transform();

	if (!mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return mage::Transform();

	return mEntries[inEntryId].LocalTransform;
}

void TransformTree::SetLocalTransform(TransformTreeEntryId inEntryId, mage::Transform inTransform)
{
	mage_ensure(inEntryId < mEntries.GetSize());
	mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists));

	mEntries[inEntryId].LocalTransform = inTransform;
	MarkGlobalDirty(inEntryId);
}

mage::Transform TransformTree::GetGlobalTransform(TransformTreeEntryId inEntryId) const
{
	if (!mage_ensure(inEntryId < mEntries.GetSize()))
		return mage::Transform();

	if (!mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists)))
		return mage::Transform();

	if (mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::GlobalDirty))
		CacheGlobalTransform(inEntryId);

	return mEntries[inEntryId].GlobalTransform;
}

void TransformTree::SetGlobalTransform(TransformTreeEntryId inEntryId, mage::Transform inTransform)
{
	mage_ensure(inEntryId < mEntries.GetSize());
	mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists));

	if (mEntries[inEntryId].ParentId != mage::InvalidIndex)
	{
		mage::Transform parentGlobal = GetGlobalTransform(mEntries[inEntryId].ParentId);
		mEntries[inEntryId].LocalTransform = parentGlobal.Inverse() * inTransform;
	}
	else
	{
		mEntries[inEntryId].LocalTransform = inTransform;
	}

	MarkGlobalDirty(inEntryId);

	mEntries[inEntryId].GlobalTransform = inTransform;
	mEntries[inEntryId].Flags.Reset(TransformTreeEntryFlags::GlobalDirty);
}

void TransformTree::AttachInternal(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId)
{
	if (!mage_ensure(mEntries[inEntryId].ParentId == mage::InvalidIndex))
		return;

	mage_ensure(!mEntries[inParentId].ChildIds.Contains(inEntryId));

	TransformTreeEntryId parentChain = mEntries[inParentId].ParentId;
	while (parentChain != mage::InvalidIndex)
	{
		if (!mage_ensure(parentChain != inEntryId))
			return;

		parentChain = mEntries[parentChain].ParentId;
	}

	mEntries[inEntryId].ParentId = inParentId;
	mEntries[inParentId].ChildIds.Add(inEntryId);

	mage::Transform parentGlobal = GetGlobalTransform(inParentId);
	mEntries[inEntryId].LocalTransform = parentGlobal.Inverse() * mEntries[inEntryId].LocalTransform;
}

void TransformTree::DetachInternal(TransformTreeEntryId inEntryId, TransformTreeEntryId inParentId)
{
	if (!mage_ensure(mEntries[inEntryId].ParentId == inParentId))
		return;

	mage_ensure(mEntries[inParentId].ChildIds.Contains(inEntryId));

	mEntries[inEntryId].LocalTransform = GetGlobalTransform(inParentId);

	mEntries[inEntryId].ParentId = mage::InvalidIndex;
	mEntries[inParentId].ChildIds.RemoveSwap(inEntryId);
}

void TransformTree::MarkGlobalDirty(TransformTreeEntryId inEntryId)
{
	mage_ensure(inEntryId < mEntries.GetSize());
	mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists));

	if (mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::GlobalDirty))
		return;

	mEntries[inEntryId].Flags.Set(TransformTreeEntryFlags::GlobalDirty);
	for (TransformTreeEntryId childId : mEntries[inEntryId].ChildIds)
		MarkGlobalDirty(inEntryId);
}

void TransformTree::CacheGlobalTransform(TransformTreeEntryId inEntryId) const
{
	mage_ensure(inEntryId < mEntries.GetSize());
	mage_ensure(mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::EntryExists));

	if (!mEntries[inEntryId].Flags.HasAny(TransformTreeEntryFlags::GlobalDirty))
		return;

	if (mEntries[inEntryId].ParentId == mage::InvalidIndex)
	{
		mEntries[inEntryId].GlobalTransform = mEntries[inEntryId].LocalTransform;
	}
	else
	{
		mage::Transform parentGlobal = GetGlobalTransform(mEntries[inEntryId].ParentId);
		mEntries[inEntryId].GlobalTransform = parentGlobal * mEntries[inEntryId].LocalTransform;
	}

	mEntries[inEntryId].Flags.Reset(TransformTreeEntryFlags::GlobalDirty);
}
