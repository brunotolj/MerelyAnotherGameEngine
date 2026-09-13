#include "Game/GameplaySystem.h"
#include "Engine/Engine.h"

GameplaySystem::GameplaySystem(GameWorld& inWorld, GameplaySystemSetup const& inSetup)
	: GameSystemWithPrerequisites(inWorld), mSetup(inSetup)
{
}

void GameplaySystem::Update(f32 inDeltaTime)
{
	for (u32 i = 0; i < 4; ++i)
	{
		PlayerData& playerData = mPlayerData[i];

		f32 input = 0.0f;
		if (gEngine->mInputHandler.IsKeyPressed(playerData.InputCodeNegative)) input -= 1.0f;
		if (gEngine->mInputHandler.IsKeyPressed(playerData.InputCodePositive)) input += 1.0f;

		f32 remainingTime = inDeltaTime;
		f32 movement = 0.0f;

		if (remainingTime > 0.0f && playerData.Speed != 0.0f && input * playerData.Speed <= 0.0f)
		{
			const f32 decelTime = std::fabsf(playerData.Speed) / mSetup.Deceleration;
			if (decelTime > remainingTime)
			{
				const f32 deltaSpeed = playerData.Speed / std::fabsf(playerData.Speed) * mSetup.Deceleration * remainingTime;
				movement += (playerData.Speed - 0.5f * deltaSpeed) * remainingTime;
				playerData.Speed -= deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * playerData.Speed * decelTime;
				playerData.Speed = 0.0f;
				remainingTime -= decelTime;
			}
		}

		if (remainingTime > 0.0f && ((playerData.Speed == 0.0f && input != 0.0f) || input * playerData.Speed > 0.0f))
		{
			const f32 accelTime = (mSetup.MaxSpeed - input * playerData.Speed) / mSetup.Acceleration;
			if (accelTime > remainingTime)
			{
				const f32 deltaSpeed = input * mSetup.Acceleration * remainingTime;
				movement += (playerData.Speed + 0.5f * deltaSpeed) * remainingTime;
				playerData.Speed += deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * (playerData.Speed + input * mSetup.MaxSpeed) * accelTime;
				playerData.Speed = input * mSetup.MaxSpeed;
				remainingTime -= accelTime;
			}
		}

		movement += playerData.Speed * remainingTime;

		playerData.Position += movement;
		if (playerData.Position > mSetup.HalfSpan)
		{
			playerData.Position = mSetup.HalfSpan;
			playerData.Speed = 0.0f;
		}
		else if (playerData.Position < -mSetup.HalfSpan)
		{
			playerData.Position = -mSetup.HalfSpan;
			playerData.Speed = 0.0f;
		}

		mage::Transform transform = playerData.OriginalTransform;
		transform.Position += playerData.Position * playerData.OriginalTransform.Rotation.Rotate({ 0.0f, -1.0f, 0.0f });
		Get<TransformTree>().SetGlobalTransform(playerData.TransformId, transform);
	}
}

void GameplaySystem::SetupPlayer(u32 inPlayerIndex, TransformTreeEntryId inTransformId, i32 inInputCodeNegative, i32 inInputCodePositive)
{
	mage_check(inPlayerIndex < 4);

	PlayerData& playerData = mPlayerData[inPlayerIndex];
	playerData.OriginalTransform = Get<TransformTree>().GetGlobalTransform(inTransformId);
	playerData.TransformId = inTransformId;
	playerData.InputCodeNegative = inInputCodeNegative;
	playerData.InputCodePositive = inInputCodePositive;
}
