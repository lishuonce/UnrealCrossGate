// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGWorldGM.generated.h"

/**
 * 
 */
UCLASS()
class UNREALCROSSGATE_API ACGWorldGM : public AGameModeBase
{
	GENERATED_BODY()
    DECLARE_LOG_CATEGORY_CLASS(CGWorldGM, Log, All);
    
public:
    
    ACGWorldGM();
    
    virtual void BeginPlay() override;
    
protected:
    
private:
};
