#include "Assets/WorldSetupFactory.h"
#include "Engine/Engine.h"

#include <iostream>

AssetHandle<WorldSetup> Factory<WorldSetup>::FromFile(mage::StringView inPath)
{
	mage::Array<u8> fileData = mage::ReadFile(inPath);
	u8* reader = fileData.GetData();

	auto readToken = [&reader](mage::String& outToken)
	{
		outToken.Empty();

		bool includeSpaces = false;

		while (true)
		{
			if (*reader == '\"') { includeSpaces = !includeSpaces; ++reader; continue; }
			if (*reader == ' ' && !includeSpaces) break;
			if (*reader == '\t') break;
			if (*reader == '\n') break;
			if (*reader == '\r') break;
			if (*reader == '\0') break;
			outToken += *(reader++);
		}
	};

	auto skipWhiteSpace = [&reader]()
	{
		while (true)
		{
			if (*reader == ' ') { reader++; continue; }
			if (*reader == '\t') { reader++; continue; }
			if (*reader == '\n') { reader++; continue; }
			if (*reader == '\r') { reader++; continue; }
			break;
		}
	};

	struct ObjectData
	{
		mage::String Name;
		mage::String Type;
		PropertyContainer Properties;
		u32 ParentChainDepth = 0;
	};

	auto parseObject = [](mage::StringView inToken, ObjectData& outObject)
	{
		outObject.Name.Empty();
		outObject.Type.Empty();
		outObject.Properties.clear();

		cstr reader = inToken.GetCString();
		while (reader - inToken.GetCString() < (i32)inToken.GetLength())
		{
			outObject.Name += *(reader++);
			if (*reader == ':') { reader++; break; }
		}
		while (reader - inToken.GetCString() < (i32)inToken.GetLength())
		{
			outObject.Type += *(reader++);
		}
	};

	auto parseProperty = [](mage::StringView inToken, ObjectData& outObject)
	{
		mage::String name, value;

		char const* reader = inToken.GetCString();
		while (reader - inToken.GetCString() < (i32)inToken.GetLength())
		{
			name += *(reader++);
			if (*reader == '=') { reader++; break; }
		}
		while (reader - inToken.GetCString() < (i32)inToken.GetLength())
		{
			value += *(reader++);
		}

		if (value.GetLength() > 0)
			outObject.Properties[name] = value;
	};

	auto parseDepth = [](mage::StringView inToken, u32& outDepth)
	{
		char const* reader = inToken.GetCString();

		while (reader - inToken.GetCString() < (i32)inToken.GetLength())
		{
			if (*reader == '\0') break;
			if (*(reader++) != '>') return false;
		}

		outDepth = inToken.GetLength();
		return true;
	};

	enum State
	{
		Root,
		ContainerEntry,
		Container,
		Properties
	} state = Root;

	mage::Array<ObjectData> assets, components, entities;
	mage::Array<ObjectData>* currentContainer = nullptr;
	ObjectData* lastObject = nullptr;
	u32 currentDepth;

	mage::String token;

	bool hasError = false;

	while (*reader && !hasError)
	{
		skipWhiteSpace();
		readToken(token);

		if (token.GetLength() == 0)
			break;

		switch (state)
		{
			case Root:
				if (token == "Assets") { state = ContainerEntry; currentContainer = &assets; break; }
				if (token == "Components") { state = ContainerEntry; currentContainer = &components; break; }
				if (token == "Entities") { state = ContainerEntry; currentContainer = &entities; break; }
				hasError = true; mage_ensure(false);
				break;

			case ContainerEntry:
				if (token == "{") { state = Container; break; }
				hasError = true; mage_ensure(false);
				break;

			case Container:
				if (token == "{") { state = Properties; break; }
				if (token == "}") { state = Root; break; }
				if (parseDepth(token, currentDepth)) break;

				currentContainer->AddDefault();
				lastObject = &currentContainer->GetLast();
				parseObject(token, *lastObject);
				lastObject->ParentChainDepth = currentDepth;
				currentDepth = 0;
				break;

			case Properties:
				if (token == "}") { state = Container; break; }
				if (lastObject == nullptr) { hasError = true; mage_ensure(false); break; }
				parseProperty(token, *lastObject);
				break;
		}
	}

	if (hasError)
		return nullptr;

	WorldSetup* result = new WorldSetup();

	for (ObjectData const& assetData : assets)
	{
		mage::String assetName = inPath;
		assetName.Append(':');

		if (gAssetFactoryFunctions.contains(assetData.Type) == false)
			continue;

		AssetHandleBase assetHandle = gAssetFactoryFunctions[assetData.Type](assetName.Append(assetData.Name), assetData.Properties);
		if (assetHandle.IsSet())
		{
			result->mAssetHandles.Add(assetHandle);
		}
	}

	for (ObjectData const& componentData : components)
	{
		result->mComponentSetups.AddConstruct(componentData.Name, componentData.Properties);
	}

	for (ObjectData const& entityData : entities)
	{
		result->mEntitySetups.AddConstruct(entityData.Name, entityData.Properties, entityData.ParentChainDepth);
	}

	return gEngine->mAssetManager.Register(result, inPath);
}
