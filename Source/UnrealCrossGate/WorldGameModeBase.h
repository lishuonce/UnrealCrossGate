// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WorldGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class UNREALCROSSGATE_API AWorldGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	DECLARE_LOG_CATEGORY_CLASS(WorldGameModeBase, Log, All);

public:

	AWorldGameModeBase();

	virtual void BeginPlay() override;

protected:

private:
};
