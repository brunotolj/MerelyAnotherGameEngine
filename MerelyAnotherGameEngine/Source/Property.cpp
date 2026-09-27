#include "Property.h"

void PropertyNode::Setup(mage::StringView inName, u32 inMemoryOffset, PropertyNode* inParentNode)
{
	mName = inName;
	mOffset = inMemoryOffset;
	mParent = inParentNode;

	if (inParentNode)
	{
		if (inParentNode->mFirstChild)
		{
			PropertyNode* previousSibling = inParentNode->mFirstChild;
			while (previousSibling->mNextSibling)
				previousSibling = previousSibling->mNextSibling;
			previousSibling->mNextSibling = this;
		}
		else
		{
			inParentNode->mFirstChild = this;
		}
	}
}

void PropertyNode::SetValue(void* inMemory, mage::StringView inValue) const
{
	mage_ensure(false);
}

void PropertyNode::GetValue(void* inMemory, mage::String& outValue) const
{
	mage_ensure(false);
}

void PropertyTree::ApplyPropertyValues(PropertyValueMap const& inPropertyValues) const
{
	ApplyPropertyValuesRecursive(inPropertyValues, &mProperties.GetFirst(), (u8*)mMemory, "");
}

void PropertyTree::ApplyPropertyValuesRecursive(PropertyValueMap const& inPropertyValues, PropertyNode const* inCurrentNode, u8* inMemory, mage::StringView inParentName) const
{
	auto getValue = [&inPropertyValues](mage::StringView inPropertyName) { return inPropertyValues.contains(inPropertyName) ? inPropertyValues.at(inPropertyName) : ""; };

	mage::String name = inParentName;
	if (inParentName.GetLength()) name.Append('.');
	name.Append(inCurrentNode->mName);

	mage::String value = getValue(name);
	if (value.GetLength() > 0)
		inCurrentNode->SetValue(inMemory + inCurrentNode->mOffset, value);

	if (inCurrentNode->mFirstChild)
	{
		ApplyPropertyValuesRecursive(inPropertyValues, inCurrentNode->mFirstChild, inMemory + inCurrentNode->mOffset, name);
	}
	
	if (inCurrentNode->mNextSibling)
	{
		ApplyPropertyValuesRecursive(inPropertyValues, inCurrentNode->mNextSibling, inMemory, inParentName);
	}
}
