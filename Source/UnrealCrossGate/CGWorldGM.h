

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
    
public:
    
    ACGWorldGM();
    
    virtual void BeginPlay() override;
    
protected:
    
private:
};
