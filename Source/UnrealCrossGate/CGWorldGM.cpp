// Fill out your copyright notice in the Description page of Project Settings.

#include "CGWorldGM.h"
#include "CGGraphicDecoder.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"
#include "PaperSpriteActor.h"
//#include "Engine/Texture2D.h"
#include "ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(CGWorldGM, Log, All);

ACGWorldGM::ACGWorldGM()
{
    
}

void ACGWorldGM::BeginPlay()
{
//    FCGGraphicDecoder &CGGraphicDecoderSingle = FCGGraphicDecoder::Get();
//    CGGraphicDecoderSingle.GetDecodePngData(19713, "00");
//    for (uint32 i = 0; i < 100; i++) {
//        CGGraphicDecoderSingle.GetDecodePngData(i, "00");
//    }
//    TArray<FString> &PaletMapKey = CGGraphicDecoderSingle.PaletTypes;
//    for (auto Key = PaletMapKey.CreateConstIterator(); Key; ++Key)
//    {
//        CGGraphicDecoderSingle.GetDecodePngData(19713, *Key);
//    }
//    for (auto Key = PaletMapKey.CreateConstIterator(); Key; ++Key)
//    {
//        CGGraphicDecoderSingle.GetDecodePngData(30, *Key);
//    }
    UWorld* const World = GetWorld();
    if (World) {
        
        //UPaperSprite *PaperSprite = LoadObject<UPaperSprite>(NULL, TEXT("/Game/Textures/NewPaperSprite"));
        UTexture2D *MapTex2d = LoadObject<UTexture2D>(NULL, TEXT("/Game/Textures/19713_00"));//crash
        
        FSpriteAssetInitParameters Para;
        Para.SetTextureAndFill(MapTex2d);
        UPaperSprite MapSprite;
        MapSprite.InitializeSprite(Para);
        
        APaperSpriteActor *MapSpriteActor = World->SpawnActor<APaperSpriteActor>();
        UPaperSpriteComponent *RenderComponent = MapSpriteActor->GetRenderComponent();

        RenderComponent->SetMobility(EComponentMobility::Stationary);
        RenderComponent->SetSprite(&MapSprite);
    }
    
}
