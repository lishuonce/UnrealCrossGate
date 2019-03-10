

#include "CGWorldGM.h"
#include "CGPlayerController.h"
#include "CGGraphicDecoder.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"
#include "PaperSpriteActor.h"
#include "ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(CGWorldGM, Log, All);

ACGWorldGM::ACGWorldGM()
{
    PlayerControllerClass = ACGPlayerController::StaticClass();
}

void ACGWorldGM::BeginPlay()
{
    Super::BeginPlay();
    
    FCGGraphicDecoder &CGGraphicDecoderSingle = FCGGraphicDecoder::Get();
    
	/*for (uint32 i = 0; i <= 7425; i++) {
        CGGraphicDecoderSingle.SaveToPng(i, "00");
    }*/

	/*for (uint32 i = 0; i <= 7425; i++) {
		CGGraphicDecoderSingle.SaveMapToPng(i);
	}*/

    /*TArray<FString> &PaletMapKey = CGGraphicDecoderSingle.PaletTypes;
    for (auto Key = PaletMapKey.CreateConstIterator(); Key; ++Key)
    {
        CGGraphicDecoderSingle.GetDecodePngData(19713, *Key);
    }*/

    /*UWorld* const World = GetWorld();
    if (World) {

        //UPaperSprite *MapSprite = LoadObject<UPaperSprite>(NULL, TEXT("/Game/Textures/MapTiles/19713_00_Sprite"));
        UPaperSprite *MapSprite = NewObject<UPaperSprite>();

        UTexture2D *MapTex2d = CGGraphicDecoderSingle.GetTexture2D(19713, "00");
		MapTex2d->Source.Init(MapTex2d->GetSizeX(), MapTex2d->GetSizeY(), 0, 0, ETextureSourceFormat::TSF_Invalid);

        FSpriteAssetInitParameters Para;
        Para.SetTextureAndFill(MapTex2d);
        MapSprite->InitializeSprite(Para);

        APaperSpriteActor *MapSpriteActor = World->SpawnActor<APaperSpriteActor>();
        UPaperSpriteComponent *RenderComponent = MapSpriteActor->GetRenderComponent();

        RenderComponent->SetMobility(EComponentMobility::Stationary);
        RenderComponent->SetSprite(MapSprite);
    }*/
}
