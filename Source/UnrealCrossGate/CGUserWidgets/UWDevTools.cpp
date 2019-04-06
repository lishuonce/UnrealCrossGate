

#include "UWDevTools.h"
#include "CGGraphicDecoder.h"

void UUWDevTools::NativeConstruct()
{
    // Call the Blueprint "Event Construct" node
    Super::NativeConstruct();
    
    // ItemTitle can be nullptr if we haven't created it in the
    // Blueprint subclass
    if (TxtBoxGraphicId)
    {
        TxtBoxGraphicId->SetText(FText::FromString(TEXT("19713")));
    }
    
    if (TxtBoxPaletType)
    {
        TxtBoxPaletType->SetText(FText::FromString(TEXT("00")));
    }
    
    BtnLoad->OnClicked.AddDynamic(this, &UUWDevTools::UpdateImage);
    
}

void UUWDevTools::UpdateImage()
{
    FCGGraphicDecoder &CGGraphicDecoderSingle = FCGGraphicDecoder::Get();
    uint32 GraphicId = FCString::Atoi(*TxtBoxGraphicId->GetText().ToString());
    FString PaletType = TxtBoxPaletType->GetText().ToString();
	UTexture2D *Tex2d = CGGraphicDecoderSingle.GetTexture2D(GraphicId, PaletType, FCGGraphicDecoder::AssetVer::V_0);
    FVector2D BrushSize;
    BrushSize.Set(Tex2d->GetSizeX(), Tex2d->GetSizeY());
    ImageLoad->SetBrushSize(BrushSize);
    ImageLoad->SetBrushFromTexture(Tex2d);
}
