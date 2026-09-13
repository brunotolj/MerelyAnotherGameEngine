#pragma once

#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

struct GameplaySystemSetup
{
	f32 HalfSpan = 0.0f;
	f32 MaxSpeed = 0.0f;
	f32 Acceleration = 0.0f;
	f32 Deceleration = 0.0f;
};

class GameplaySystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	GameplaySystem(GameWorld& inWorld, GameplaySystemSetup const& inSetup);

	virtual void Update(f32 inDeltaTime) override;

	void SetupPlayer(u32 inPlayerIndex, TransformTreeEntryId inTransformId, i32 inInputCodeNegative, i32 inInputCodePositive);

private:
	GameplaySystemSetup mSetup;

	struct PlayerData
	{
		mage::Transform OriginalTransform;
		TransformTreeEntryId TransformId = mage::InvalidIndex;

		i32 InputCodeNegative = 0;
		i32 InputCodePositive = 0;

		f32 Position = 0.0f;
		f32 Speed = 0.0f;
	};

	PlayerData mPlayerData[4];
};
