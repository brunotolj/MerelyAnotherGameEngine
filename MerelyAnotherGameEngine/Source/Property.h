#pragma once

struct PropertyNode
{
	friend class PropertyTree;

public:
	void Setup(mage::StringView inName, u32 inMemoryOffset, PropertyNode* inParentNode);

	virtual void SetValue(void* inMemory, mage::StringView inValue) const;
	virtual void GetValue(void* inMemory, mage::String& outValue) const;

protected:
	PropertyNode* mParent = nullptr;
	PropertyNode* mFirstChild = nullptr;
	PropertyNode* mNextSibling = nullptr;
	mage::String mName;
	u32 mOffset = 0;
};

template <typename>
struct Property;

using PropertyContainerOld = std::unordered_map<mage::String, mage::String>;

class PropertyTree : NonMovable
{
public:
	template <typename Type>
	PropertyTree(Type& inRoot) : mMemory(&inRoot)
	{
		mParentStack.Add(nullptr);
		AddProperty<Type>("", 0);
	}

	template <typename Type>
	void AddProperty(mage::StringView inName, u32 inMemoryOffset)
	{
		u32 index = mProperties.AddUninitialized();
		new (mProperties.GetData() + index) Property<Type>;
		mProperties.GetLast().Setup(inName, inMemoryOffset, mParentStack.GetLast());
		mParentStack.Add(&mProperties.GetLast());
		Property<Type>::GetChildProperties(*this);
		mParentStack.RemoveAt(mParentStack.GetSize() - 1);
	}

	void ApplyPropertyValues(PropertyContainerOld const& inPropertyValues) const;

private:
	mage::Array<PropertyNode> mProperties;
	mage::Array<PropertyNode*> mParentStack;
	void* mMemory;

	void ApplyPropertyValuesRecursive(PropertyContainerOld const& inPropertyValues, PropertyNode const* inCurrentNode, u8* inMemory, mage::StringView inParentName) const;
};

template <NumericType Type>
struct Property<Type> : public PropertyNode
{
	virtual void SetValue(void* inMemory, mage::StringView inValue) const override
	{
		*(Type*)(inMemory) = mage::ParseNumber<Type>(inValue);
	}

	virtual void GetValue(void* inMemory, mage::String& outValue) const override
	{
		outValue.FromNumber(*(Type*)(inMemory));
	}

	static void GetChildProperties(PropertyTree& inOutContainer) {}
};

#define AddProperty(Parent, Property) AddProperty<decltype(Parent::Property)>(#Property, offsetof(Parent, Property))

template <>
struct Property<glm::vec3> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(glm::vec3, x);
		inOutContainer.AddProperty(glm::vec3, y);
		inOutContainer.AddProperty(glm::vec3, z);
	}
};
