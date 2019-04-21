

#include "CGWorldGM.h"
#include "CGPlayerController.h"
#include "CGGraphicDecoder.h"
#include "PaperTileMap.h"
#include "PaperTileMapActor.h"
#include "PaperTileMapComponent.h"
#include "PaperSprite.h"
#include "PaperSpriteActor.h"
#include "PaperSpriteComponent.h"

DEFINE_LOG_CATEGORY_STATIC(CGWorldGM, Log, All);

ACGWorldGM::ACGWorldGM()
{
    //PlayerControllerClass = ACGPlayerController::StaticClass();
}

void ACGWorldGM::BeginPlay()
{
    Super::BeginPlay();
    
    //FCGGraphicDecoder &CGGraphicDecoderSingle = FCGGraphicDecoder::Get();

	UWorld* const World = GetWorld();
	if (World) {

		APaperTileMapActor *tmActor = World->SpawnActor<APaperTileMapActor>();
		UPaperTileMapComponent *tmComponent = tmActor->GetRenderComponent();

		UPaperTileMap* tmObject = NewObject<UPaperTileMap>();
		tmObject->MapWidth = 3;
		tmObject->MapHeight = 4;
		tmObject->TileWidth = 64;
		tmObject->TileHeight = 47;
		tmObject->ProjectionMode = ETileMapProjectionMode::IsometricDiamond;
		tmObject->InitializeNewEmptyTileMap();

		//tmComponent->SetMobility(EComponentMobility::Stationary);
		tmComponent->SetTileMap(tmObject);

		FVector TileVector;
		uint32 X, Y;

		X = 0;
		Y = 0;
		TileVector = tmComponent->GetTileCornerPosition(X, Y);
		UE_LOG(LogTemp, Warning, TEXT("L:X=%d,Y=%d topleft x:%f,y:%f,z:%f"), X, Y, TileVector.X, TileVector.Y, TileVector.Z);

		TileVector = tmComponent->GetTileCenterPosition(X, Y);
		UE_LOG(LogTemp, Warning, TEXT("L:X=%d,Y=%d center x:%f,y:%f,z:%f"), X, Y, TileVector.X, TileVector.Y, TileVector.Z);

		X = 0;
		Y = 1;
		TileVector = tmComponent->GetTileCornerPosition(X, Y, 0, true);
		UE_LOG(LogTemp, Warning, TEXT("W:X=%d,Y=%d corner x:%f,y:%f,z:%f"), X, Y, TileVector.X, TileVector.Y, TileVector.Z);

		TileVector = tmComponent->GetTileCenterPosition(X, Y, 0, true);
		UE_LOG(LogTemp, Warning, TEXT("W:X=%d,Y=%d center x:%f,y:%f,z:%f"), X, Y, TileVector.X, TileVector.Y, TileVector.Z);

		UPaperSprite* Sprite = LoadObject<UPaperSprite>(NULL, TEXT("PaperSprite'/Game/Textures/Map/Frames/221247_2.221247_2'"));
		APaperSpriteActor* SpriteActor = World->SpawnActor<APaperSpriteActor>();
		UPaperSpriteComponent* SpriteComponent = SpriteActor->GetRenderComponent();
		SpriteComponent->SetMobility(EComponentMobility::Stationary);
		SpriteComponent->SetSprite(Sprite);
	}
    
	/*for (uint32 i = 0; i <= 7425; i++) {
        CGGraphicDecoderSingle.SaveToPng(i, "00");
    }*/

	/*for (uint32 i = 0; i <= 13371; i++) {
		CGGraphicDecoderSingle.SaveTileToPng(i);
	}*/

	/*for (uint32 i = 250496; i <= 256331; i++) {
		CGGraphicDecoderSingle.SaveTileToPng(i);
	}*/

    /*TArray<FString> &PaletMapKey = CGGraphicDecoderSingle.PaletTypes;
    for (auto Key = PaletMapKey.CreateConstIterator(); Key; ++Key)
    {
        CGGraphicDecoderSingle.GetDecodePngData(19713, *Key);
    }*/
}
