

#include "CGPlayerController.h"
#include "CGUserWidgets/UWDevTools.h"


void ACGPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    UUWDevTools *UserWidget = nullptr;
    
    TSubclassOf<UUWDevTools> WidgetClass = LoadClass<UUWDevTools>(this, TEXT("WidgetBlueprint'/Game/UI/BPWidgets/WB_DevTools.WB_DevTools_C'"));
    
    if (WidgetClass != nullptr)
    {
        UserWidget = CreateWidget<UUWDevTools>(GetWorld(), WidgetClass);
        if (UserWidget != nullptr)
        {
            UserWidget->AddToViewport();
            FInputModeUIOnly Mode;
            SetInputMode(Mode);
            bShowMouseCursor = true;
        }
    }
    
}
