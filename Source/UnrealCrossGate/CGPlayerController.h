

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UNREALCROSSGATE_API ACGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    // Override BeginPlay()
    virtual void BeginPlay() override;
    
};
