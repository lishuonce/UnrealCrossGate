

#include "CGWorldGM.h"
#include "CGGraphicDecoder.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"
#include "PaperSpriteActor.h"
#include "ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(CGWorldGM, Log, All);

ACGWorldGM::ACGWorldGM()
{
    
}

void ACGWorldGM::BeginPlay()
{
    FCGGraphicDecoder &CGGraphicDecoderSingle = FCGGraphicDecoder::Get();
    
	//CGGraphicDecoderSingle.GetDecodePngData(19713, "00");
    
	/*for (uint32 i = 0; i < 100; i++) {
        CGGraphicDecoderSingle.GetDecodePngData(i, "00");
    }*/

    /*TArray<FString> &PaletMapKey = CGGraphicDecoderSingle.PaletTypes;
    for (auto Key = PaletMapKey.CreateConstIterator(); Key; ++Key)
    {
        CGGraphicDecoderSingle.GetDecodePngData(19713, *Key);
    }
    for (auto Key = PaletMapKey.CreateConstIterator(); Key; ++Key)
    {
        CGGraphicDecoderSingle.GetDecodePngData(30, *Key);
    }*/

    UWorld* const World = GetWorld();
    if (World) {
        
        //UPaperSprite *MapSprite = LoadObject<UPaperSprite>(NULL, TEXT("/Game/Textures/MapTiles/19713_00_Sprite"));
		//UPaperSprite *MapSprite;

		UTexture2D *MapTex2d = CGGraphicDecoderSingle.GetTexture2D(19713, "00");

        /*FSpriteAssetInitParameters Para;
		FIntPoint Dimension;
		Dimension.X = 168;
		Dimension.Y = 152;
		
        Para.SetTextureAndFill(MapTex2d);
		Para.Dimension = Dimension;
        MapSprite->InitializeSprite(Para);
		
        APaperSpriteActor *MapSpriteActor = World->SpawnActor<APaperSpriteActor>();
        UPaperSpriteComponent *RenderComponent = MapSpriteActor->GetRenderComponent();

        RenderComponent->SetMobility(EComponentMobility::Stationary);
        RenderComponent->SetSprite(MapSprite);*/
    }
}
