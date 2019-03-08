

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "UWDevTools.generated.h"

/**
 * 
 */
UCLASS()
class UNREALCROSSGATE_API UUWDevTools : public UUserWidget
{
	GENERATED_BODY()

public:
    
    UPROPERTY(Meta = (BindWidget))
    UButton *BtnLoad;
    
    UPROPERTY(Meta = (BindWidget))
    UEditableTextBox *TxtBoxGraphicId;
    
    UPROPERTY(Meta = (BindWidget))
    UEditableTextBox *TxtBoxPaletType;
    
    UPROPERTY(Meta = (BindWidget))
    UImage *ImageLoad;
    
    virtual void NativeConstruct() override;
    
    UFUNCTION()
    void UpdateImage();
};
