

#include "CGGraphicDecoder.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "ImageUtils.h"
#include "PaperTileSet.h"
#include "PaperTileMap.h"
#include "PaperTileLayer.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(CGGraphicDecoder, Warning, Warning);

FCGGraphicDecoder & FCGGraphicDecoder::Get()
{
    static FCGGraphicDecoder Singleton;
    return Singleton;
}

FCGGraphicDecoder::FCGGraphicDecoder()
{
    SetResPath();
    if (IsResVerified())
    {
		LoadMapList();
        LoadAnimeTable(V_0);
        LoadAnimeTable(V_Ex);
		LoadGraphicInfo(V_0);
		LoadGraphicInfo(V_Ex);
        SetTileInfo(V_0);
        SetTileInfo(V_Ex);
        LoadPaletData();
        InitGraphicData(V_0);
		InitGraphicData(V_Ex);
    }
    else
    {
        
    }
	// set transparent color
	alpha_index = 0x00;
	alpha_level = 0x00;
    
    //
    Test();
}

FCGGraphicDecoder::~FCGGraphicDecoder()
{
    // static Singleton will not excute deconstructor
    // dont implement
}

UTexture2D * FCGGraphicDecoder::GetTexture2D(uint32 GraphicId, FString PaletType, AssetVer Ver)
{
    SetColorBuff(GraphicId, PaletType, Ver);
    
	// create Texture2D
    UTexture2D *Tex2d = UTexture2D::CreateTransient(CurGraphicData.gWidth, CurGraphicData.gHeight, PF_B8G8R8A8);
    FTexture2DMipMap& Mip = Tex2d->PlatformData->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(Data, ColorBuff, CurGraphicData.gLength * sizeof(FColor));
    Mip.BulkData.Unlock();
    Tex2d->UpdateResource();
    
    // gc ColorBuff
    delete[] ColorBuff;
    ColorBuff = nullptr;
    
    return Tex2d;
}

void FCGGraphicDecoder::SaveToPng(uint32 GraphicId, FString PaletType, AssetVer Ver)
{
	SetColorBuff(GraphicId, PaletType, Ver);

	// create png array
	TArray<FColor> SrcData;
	SrcData.Append(ColorBuff, CurGraphicData.gLength);
	TArray<uint8> DstData;
	FImageUtils::CompressImageArray(CurGraphicData.gWidth, CurGraphicData.gHeight, SrcData, DstData);

	// save png file
	FString fsTexturePath = FPaths::ProjectContentDir() + "CGRawDecode/";
	// FString fsTmpPngPath = FPaths::CreateTempFilename(*fsTexturePath, TEXT("tmp"), TEXT(".png"));// Filename : random
	FString fsTmpPngPath = fsTexturePath + FString::FromInt(GraphicId) + "_" + PaletType + ".png";// Filename : GraphicID + PaletType
	IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle *fileHandleTmp = platFormFile.OpenWrite(*fsTmpPngPath);
	if (fileHandleTmp)
	{
		fileHandleTmp->Write(DstData.GetData(), DstData.Num());
		delete fileHandleTmp;
	}

	// gc ColorBuff
	delete[] ColorBuff;
	ColorBuff = nullptr;
}

void FCGGraphicDecoder::ExportTileMapPng()
{
    for (auto& TileElem :TileInfo)
    {
		// decode Graphic
		SetColorBuff(TileElem.Value.gId, "00", TileElem.Value.Ver);

		// create png array
		TArray<FColor> SrcData;
		SrcData.Append(ColorBuff, CurGraphicData.gLength);
		TArray<uint8> DstData;
		FImageUtils::CompressImageArray(CurGraphicData.gWidth, CurGraphicData.gHeight, SrcData, DstData);

		FString fsTexturePath = FPaths::ProjectContentDir() + "CGRawDecode/Map/NonTiles/";
		if (TileElem.Value.bIsTile)
		{
			fsTexturePath = FPaths::ProjectContentDir() + "CGRawDecode/Map/Tiles/";
		}

		FString fsTmpPngPath = fsTexturePath + FString::FromInt(TileElem.Key) + ".png";// Filename : gTileId

		// save png file
		IPlatformFile& platFormFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileHandle* fileHandleTmp = platFormFile.OpenWrite(*fsTmpPngPath);
		if (fileHandleTmp)
		{
			fileHandleTmp->Write(DstData.GetData(), DstData.Num());
			delete fileHandleTmp;
		}

		// gc ColorBuff
		delete[] ColorBuff;
		ColorBuff = nullptr;
    }
}

void FCGGraphicDecoder::SetResPath()
{
	FString fsAssetPath = FPaths::ProjectContentDir() + "CGRaw/bin/";

	AssetInfo Asset_V0;
	CGAsset.Emplace(V_0, Asset_V0);

	CGAsset[V_0].Path.Emplace(T_GraphicInfo, fsAssetPath + "GraphicInfo_20.bin");
	CGAsset[V_0].Path.Emplace(T_GraphicData, fsAssetPath + "Graphic_20.bin");
	CGAsset[V_0].Path.Emplace(T_AnimeInfo, fsAssetPath + "AnimeInfo_3.bin");
	CGAsset[V_0].Path.Emplace(T_AnimeData, fsAssetPath + "Anime_3.bin");

	AssetInfo Asset_Ex;
	CGAsset.Emplace(V_Ex, Asset_Ex);

	CGAsset[V_Ex].Path.Emplace(T_GraphicInfo, fsAssetPath + "GraphicInfoEx_4.bin");
	CGAsset[V_Ex].Path.Emplace(T_GraphicData, fsAssetPath + "GraphicEx_4.bin");
	CGAsset[V_Ex].Path.Emplace(T_AnimeInfo, fsAssetPath + "AnimeInfoEx_1.Bin");
	CGAsset[V_Ex].Path.Emplace(T_AnimeData, fsAssetPath + "AnimeEx_1.Bin");
}

bool FCGGraphicDecoder::IsResVerified()
{
    // todo : check file exists
    // todo : check md5
    
    if (true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void FCGGraphicDecoder::LoadMapList()
{
	// Set fsMapPath
	FString fsMapPath = FPaths::ProjectContentDir() + "CGRaw/map/0/";
	FString fsMapExten = ".dat";

	// Get *.dat to FileList
	IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> FileList;
	PlatFormFile.FindFiles(FileList, *fsMapPath, *fsMapExten);
	uint32 MapId;

	// to MapList
	for (int32 i = 0; i < FileList.Num(); i++)
	{
		MapId = FCString::Atoi(*FPaths::GetBaseFilename(FileList[i]));
        if (MapId < 40000 && MapId != 18564 && MapId != 9999)
        {
            MapList.Add(MapId);
        }
	}
}

void FCGGraphicDecoder::SetMapData(uint32 MapId)
{
	// Set fsMapPath
	FString fsMapPath = FPaths::ProjectContentDir() + "CGRaw/map/0/";
	FString fsMapExten = ".dat";
	FString fsMapName = FString::FromInt(MapId);
	FString fsMapFile = fsMapPath + fsMapName + fsMapExten;

	// Load *.dat to CurMap
	IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*fsMapFile);
	if (fileHandleTmp)
	{
		// Load GraphicMap Header : mHeader[3], mBlank[9], mWidth, mHeight
		fileHandleTmp->Read((uint8 *)&CurMap, 20);

		uint32 LayerLength = CurMap.mWidth * CurMap.mHeight;
        uint32 LayerSize = LayerLength * 2;

		// Load GraphicMap Layet : mTerrainLayer
		CurMap.mTerrainLayer = new uint16[LayerLength];
		fileHandleTmp->Read((uint8 *)CurMap.mTerrainLayer, LayerSize);

		// Load GraphicMap Layet : mArtifactLayer
		CurMap.mArtifactLayer = new uint16[LayerLength];
		fileHandleTmp->Read((uint8 *)CurMap.mArtifactLayer, LayerSize);

		// Load GraphicMap Layet : mFlagLayer
		CurMap.mFlagLayer = new MapFlag[LayerLength];
		fileHandleTmp->Read((uint8 *)CurMap.mFlagLayer, LayerSize);

		delete fileHandleTmp;
	}
}

void FCGGraphicDecoder::CreateTileMap(uint32 MapId)
{
    // load tilesets
	TArray<UPaperTileSet*> TileSets;
	FString fsTileSetPrefix = "/Game/Maps/TileSets/Tiles";
	for (uint8 i = 0; i <= 5; i++)
	{
		FString fsTileSetName = fsTileSetPrefix + FString::FromInt(i);
		TileSets.Emplace(LoadObject<UPaperTileSet>(NULL, *fsTileSetName));
	}
    
    // create package
    FString PackageName = FString::FromInt(MapId);
    FString PackagePath = "/Game/Maps/" + PackageName;
    UPackage * Package = CreatePackage(nullptr, *PackagePath);
    EObjectFlags Flags = RF_Public|RF_Standalone|RF_Transactional;
    UPaperTileMap* TileMap = NewObject<UPaperTileMap>(Package, *PackageName, Flags);
    
    // set map data - base
    SetMapData(MapId);
    TileMap->MapWidth = CurMap.mHeight;
    TileMap->MapHeight = CurMap.mWidth;
    TileMap->TileWidth = 64;
    TileMap->TileHeight = 47;
    TileMap->ProjectionMode = ETileMapProjectionMode::IsometricDiamond;
    TileMap->TileLayers.Empty();
    
    // set map data - mTerrainLayer
    UPaperTileLayer *pTerrainLayer = TileMap->AddNewLayer();
    pTerrainLayer->LayerName = FText::FromString(TEXT("Terrain"));
    
	uint32 LayerCursor = 0;
	for (uint32 CurX = 0; CurX <= CurMap.mHeight - 1; CurX++)
	{
		for (int32 CurY = CurMap.mWidth - 1; CurY >= 0; CurY--)
		{
			uint32 CurTileId = CurMap.mTerrainLayer[LayerCursor];
			LayerCursor++;
			
			if (CurTileId >= 20000)
			{
				CurTileId += 200000;
			}

			if (CurTileId != 0 && TileInfo.Contains(CurTileId))
			{
				FPaperTileInfo fCellInfo;
				uint32 TileSetId = TileInfo[CurTileId].TileSetId;
				fCellInfo.TileSet = TileSets[TileSetId];
				fCellInfo.PackedTileIndex = TileInfo[CurTileId].TileSetIndex;
				pTerrainLayer->SetCell(CurX, CurY, fCellInfo);

				//UE_LOG(LogTemp, Warning, TEXT("%d,%d,%d,%d,%d,%d"), CurX, CurY, LayerCursor, CurTileId, TileSetId, TileInfo[CurTileId].TileSetIndex);
			}
		}
	}
    
    // save package
    Package->MarkPackageDirty();
    FString FilePath = FPaths::ProjectContentDir() + "Maps/";
    FString FileName = FilePath + PackageName + FPackageName::GetAssetPackageExtension();
    bool bSuccess = UPackage::SavePackage(Package, TileMap, Flags, *FileName);
    
}

void FCGGraphicDecoder::LoadAnimeTable(AssetVer Ver)
{
	AnimeInfo *pAnimeInfo;
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();

    IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*CGAsset[Ver].Path[T_AnimeInfo]);
    if (fileHandleTmp)
    {
        uint32 iFileSize = fileHandleTmp->Size();
        uint32 iRecordNum = iFileSize / sizeof(AnimeInfo);
		pAnimeInfo = new AnimeInfo[iRecordNum];
        fileHandleTmp->Read((uint8 *)pAnimeInfo, iFileSize);
        delete fileHandleTmp;

		IFileHandle *fileHandleTmp2 = PlatFormFile.OpenRead(*CGAsset[Ver].Path[T_AnimeData]);
		if (fileHandleTmp2)
		{
			uint32 StartIndex = 2375;
			switch (Ver)
			{
			case FCGGraphicDecoder::V_0:
				StartIndex = 2375;
				break;
			case FCGGraphicDecoder::V_Ex:
				StartIndex = 0;
				break;
			case FCGGraphicDecoder::V_unknown:
				UE_LOG(CGGraphicDecoder, Error, TEXT("FCGGraphicDecoder::LoadAnimeTable switch (Ver) V_unknown"));
				break;
			default:
				UE_LOG(CGGraphicDecoder, Error, TEXT("FCGGraphicDecoder::LoadAnimeTable switch (Ver) default"));
				break;
			}
			CGAsset[Ver].AnimeNum = iRecordNum - StartIndex;
			CGAsset[Ver].AnimeTable = new AnimeTable[CGAsset[Ver].AnimeNum];
            

			for (uint32 i = StartIndex; i < iRecordNum; i++)
			{
				uint32 ActionId = i - StartIndex;
				
				// Set AnimeTable
				CGAsset[Ver].AnimeTable[ActionId].aId = pAnimeInfo[i].aId;
				CGAsset[Ver].AnimeTable[ActionId].aNum = pAnimeInfo[i].aNum;
				CGAsset[Ver].AnimeTable[ActionId].aMovement = new Movement[CGAsset[Ver].AnimeTable[ActionId].aNum];

				for (uint32 j = 0; j < CGAsset[Ver].AnimeTable[ActionId].aNum; j++)//j: movement num
				{
					// Load Anime_*.bin Header
					fileHandleTmp2->Read((uint8 *)&CGAsset[Ver].AnimeTable[ActionId].aMovement[j], 12);
					CGAsset[Ver].AnimeTable[ActionId].aMovement[j].gId = new uint32[CGAsset[Ver].AnimeTable[ActionId].aMovement[j].aFrameNum];

					for (uint32 k = 0; k < CGAsset[Ver].AnimeTable[ActionId].aMovement[j].aFrameNum; k++)//k: frame num
					{
						// Load Anime_*.bin FrameData
						fileHandleTmp2->Read((uint8 *)&CGAsset[Ver].AnimeTable[ActionId].aMovement[j].gId[k], 4);
                        uint8 unknown[6];
                        fileHandleTmp2->Read((uint8 *)&unknown, 6);
					}
				}
			}
			delete fileHandleTmp2;
		}
		delete[] pAnimeInfo;
	}
}

void FCGGraphicDecoder::LoadGraphicInfo(AssetVer Ver)
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*CGAsset[Ver].Path[T_GraphicInfo]);
    if (fileHandleTmp)
    {
		// load CGAsset[Ver].GraphicInfo
        uint32 iFileSize = fileHandleTmp->Size();
        uint32 iRecordNum = iFileSize / sizeof(GraphicInfo);
        
        CGAsset[Ver].GraphicInfo = new GraphicInfo[iRecordNum];
        CGAsset[Ver].GraphicNum = iRecordNum;
        
        fileHandleTmp->Read((uint8 *)CGAsset[Ver].GraphicInfo, iFileSize);
        delete fileHandleTmp;
    }
}

void FCGGraphicDecoder::SetTileInfo(AssetVer Ver)
{
    // set Ver, gId
    for (uint32 i=0; i<CGAsset[Ver].GraphicNum; i++)
    {
        if (CGAsset[Ver].GraphicInfo[i].gTileId != 0)
        {
            TileData TileData = { Ver, CGAsset[Ver].GraphicInfo[i].gId, 0, 0, 0, 0, 0, 1 };
            TileInfo.Emplace(CGAsset[Ver].GraphicInfo[i].gTileId, TileData);
        }
    }
    
    
    if (Ver == V_Ex)
    {   
        // set CountTerrain, CountArtifact, bHasTileId, Map
        TileData TileData = { V_unknown, 0, 0, 0, 0, 0, 0, 0 };
        for (auto &Key : MapList)
        {
            SetMapData(Key);
            uint32 Length = CurMap.mWidth * CurMap.mHeight;
            for (uint32 i = 0; i < Length; i++)
            {
                uint32 TileId = CurMap.mTerrainLayer[i];
                if (TileId >= 20000)
                {
                    TileId += 200000;
                }
                if (TileInfo.Contains(TileId))
                {
                    TileInfo[TileId].CountTerrain++;
                    TileInfo[TileId].Map.Add(Key);
                }
                else if (TileId != 0)
                {
                    TileInfo.Emplace(TileId, TileData);
                    TileInfo[TileId].Map.Add(Key);
                }
                
                uint32 TileId2 = CurMap.mArtifactLayer[i];
                if (TileId2 >= 20000)
                {
                    TileId2 += 200000;
                }
                if (TileInfo.Contains(TileId2))
                {
                    TileInfo[TileId2].CountArtifact++;
                    TileInfo[TileId2].Map.Add(Key);
                }
                else if (TileId2 != 0 && TileId2 != 2)
                {
                    TileInfo.Emplace(TileId2, TileData);
                    TileInfo[TileId2].Map.Add(Key);
                }
            }
            delete[] CurMap.mTerrainLayer;
            delete[] CurMap.mArtifactLayer;
            delete[] CurMap.mFlagLayer;
        }
        
        // print tileinfo
        /*for (auto& TileUnit :TileInfo)
        {
            TileUnit.Value.Map.Sort([](uint32 A, uint32 B) {return A > B;});
            FString mapSet;
            for (auto& Elem :TileUnit.Value.Map)
            {
                mapSet += FString::FromInt(Elem) + TEXT("|");
            }
            UE_LOG(LogTemp, Warning, TEXT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%s"), TileUnit.Key, TileUnit.Value.Ver, TileUnit.Value.gId, TileUnit.Value.bIsTile, TileUnit.Value.TileSetId, TileUnit.Value.TileSetIndex, TileUnit.Value.bHasTileId, TileUnit.Value.CountTerrain, TileUnit.Value.CountArtifact, *mapSet);
        }*/

		// remove unused tileinfo
		TArray<uint32> TileInfoKeyArr;
		TileInfo.GenerateKeyArray(TileInfoKeyArr);
		for (auto& Elem : TileInfoKeyArr)
		{
			if (TileInfo[Elem].Ver == V_unknown
				|| TileInfo[Elem].bHasTileId == 0
				|| (TileInfo[Elem].CountTerrain == 0 && TileInfo[Elem].CountArtifact == 0)
				|| (Elem <= 265 && Elem >= 200))
			{
				TileInfo.Remove(Elem);
			}
		}

		// Set bIsTile, TileSetId, TileSetIndex
		const uint32 TileSetCount = 1376;//tile num per tileset
		uint32 IsTiledIndex = 0;

		TileInfo.KeySort([](uint32 A, uint32 B) {return A < B; });

		for (auto& TileElem : TileInfo)
		{
			if (CGAsset[TileElem.Value.Ver].GraphicInfo[TileElem.Value.gId].gWidth == 64 && CGAsset[TileElem.Value.Ver].GraphicInfo[TileElem.Value.gId].gHeight == 47)
			{
				TileElem.Value.bIsTile = 1;
				TileElem.Value.TileSetId = FMath::DivideAndRoundDown(IsTiledIndex, TileSetCount);
				TileElem.Value.TileSetIndex = IsTiledIndex % TileSetCount;
				IsTiledIndex++;
			}
		}
    }
}

void FCGGraphicDecoder::LoadPaletData()
{
    // Palet Color 0 ~ 15 (common)
    uint8 CommonPaletHead[48] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80,
        0x00, 0x80, 0x80, 0x80, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0xDC, 0xC0, 0xF0, 0xCA, 0xA6, 0x00, 0x00,
        0xDE, 0x00, 0x5F, 0xFF, 0xA0, 0xFF, 0xFF, 0xD2, 0x5F, 0x00, 0xFF, 0xD2, 0x50, 0x28, 0xE1, 0x28};
    
    // Palet Color 240 ~ 255 (common) - fixed by decompile sec-cg-viewer
    uint8 CommonPaletEnd[48] = {
        0x96, 0xC3, 0xF5, 0x5F, 0xA0, 0xE1, 0x46, 0x7D, 0xC3, 0x1E, 0x55, 0x90, 0x37, 0x41, 0x46, 0x1E,
        0x23, 0x28, 0xF0, 0xFB, 0xFF, 0xA4, 0xA0, 0xA0, 0x80, 0x80, 0x80, 0x00, 0x00, 0xFF, 0x00, 0xFF,
        0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF};
    
    // Set fsPaletPath
    FString fsPaletPath = FPaths::ProjectContentDir() + "CGRaw/bin/pal/";
    FString fsPaletExten = ".cgp";

	// Get Palet_*.cgp to FileList
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    TArray<FString> FileList;
    PlatFormFile.FindFiles(FileList, *fsPaletPath, *fsPaletExten);
	FString PaletType;

	// Load Palet_*.cgp to CGPalet
	for (int32 i = 0; i < FileList.Num(); i++)
    {
        PaletData PrePalet;
        IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*FileList[i]);
        if (fileHandleTmp)
        {
            // Palet Color 16 ~ 239 (Palet_*.cgp)
            fileHandleTmp->Read((uint8 *)(PrePalet.ColorData + 16), 672);
            delete fileHandleTmp;
        }
        
        // Set Palet (common)
        FMemory::Memcpy(PrePalet.ColorData, CommonPaletHead, 48);
        FMemory::Memcpy(PrePalet.ColorData + 240, CommonPaletEnd, 48);
        
        // Add to CGPalet
        PaletType = FPaths::GetBaseFilename(FileList[i]).RightChop(6);
        CGPalet.Emplace(PaletType, PrePalet);
    }
    
	// Set loaded PaletTypes
    CGPalet.GenerateKeyArray(PaletTypes);
}

void FCGGraphicDecoder::InitGraphicData(AssetVer Ver)
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
	CGAsset[Ver].FileHandle = PlatFormFile.OpenRead(*CGAsset[Ver].Path[T_GraphicData]);
}

void FCGGraphicDecoder::SetGraphicData(uint32 GraphicId, AssetVer Ver)
{
	if (CGAsset[Ver].FileHandle)
	{
		// Load Graphic_*.bin Header
		CGAsset[Ver].FileHandle->Seek(CGAsset[Ver].GraphicInfo[GraphicId].gAddr);
		CGAsset[Ver].FileHandle->Read((uint8 *)&CurGraphicData, 16);

		// Load Graphic_*.bin gData
		CurGraphicData.gLength -= 16;
		CurGraphicData.gData = new uint8[CurGraphicData.gLength];
		CGAsset[Ver].FileHandle->Read(CurGraphicData.gData, CurGraphicData.gLength);
	}
}

void FCGGraphicDecoder::DecodeGraphicData()
{
	// RLEFlags for RLE Decode
	enum
	{
		RLE_NONE,
		RLE_READ,
		RLE_REPEAT_BACKGROUND,
		RLE_REPEAT_TRANSPARENT
	}ERLEFlags = RLE_NONE;

	uint32 BufferDecodedCursor = 0;
	uint32 BufferEncodedCursor = 0;
	uint8 *BufferDecoded = new uint8[CurGraphicData.gHeight*CurGraphicData.gWidth];

	while (BufferEncodedCursor < CurGraphicData.gLength)
	{
		uint8 High = CurGraphicData.gData[BufferEncodedCursor] >> 4;
		uint8 Low = CurGraphicData.gData[BufferEncodedCursor] & 0x0f;
		uint32 RLESize = 0;
		uint8 *RepeatBuffer;

		switch (High)
		{
		case 0x0:
			RLESize = Low;
			ERLEFlags = RLE_READ;
			break;
		case 0x1:
			RLESize = Low * 0x100 + CurGraphicData.gData[BufferEncodedCursor + 1];
			ERLEFlags = RLE_READ;
			break;
		case 0x2:
			RLESize = Low * 0x10000 + CurGraphicData.gData[BufferEncodedCursor + 1] * 0x100 + CurGraphicData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_READ;
			break;
		case 0x8:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0x9:
			RLESize = Low * 0x100 + CurGraphicData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xa:
			RLESize = Low * 0x10000 + CurGraphicData.gData[BufferEncodedCursor + 2] * 0x100 + CurGraphicData.gData[BufferEncodedCursor + 3];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xc:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xd:
			RLESize = Low * 0x100 + CurGraphicData.gData[BufferEncodedCursor + 1];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xe:
			RLESize = Low * 0x10000 + CurGraphicData.gData[BufferEncodedCursor + 1] * 0x100 + CurGraphicData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		default:
			UE_LOG(CGGraphicDecoder, Error, TEXT("JSS-RLE Decode Error - switch(High) Default: %x"), CurGraphicData.gData[BufferEncodedCursor]);
			break;
		}// while switch(High) END

		switch (ERLEFlags)
		{
		case RLE_READ:
			FMemory::Memcpy(&BufferDecoded[BufferDecodedCursor], &CurGraphicData.gData[BufferEncodedCursor + High + 1], RLESize);
			BufferDecodedCursor += RLESize;
			BufferEncodedCursor += High + 1 + RLESize;
			break;
		case RLE_REPEAT_BACKGROUND:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, CurGraphicData.gData[BufferEncodedCursor + 1], RLESize);
			FMemory::Memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
			BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 2;
			break;
		case RLE_REPEAT_TRANSPARENT:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, alpha_index, RLESize);
			FMemory::Memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
			BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 1;
			break;
		default:
			UE_LOG(CGGraphicDecoder, Error, TEXT("JSS-RLE Decode Error - switch(ERLEFlags) Default: %x"), CurGraphicData.gData[BufferEncodedCursor]);
			break;
		}// switch(ERLEFlags) END

		ERLEFlags = RLE_NONE;

	}// while CurrentDecodePosition < SizeOfBuffer END

	// update SGData
	CurGraphicData.gIscompressed = 0;
	CurGraphicData.gLength = BufferDecodedCursor;
	delete[] CurGraphicData.gData;
	CurGraphicData.gData = BufferDecoded;
	BufferDecoded = nullptr;
}

void FCGGraphicDecoder::FormatGraphicData()
{
    uint8 *BufferFormated = new uint8[CurGraphicData.gLength];
    uint32 Cursor;
    uint32 CursorFormated;
    uint32 LineFormated;
    
    for (uint32 Line = 0; Line < CurGraphicData.gHeight; Line ++)
    {
        LineFormated = CurGraphicData.gHeight - Line - 1;
        Cursor = Line * CurGraphicData.gWidth;
        CursorFormated = LineFormated * (CurGraphicData.gWidth);
        FMemory::Memcpy(&BufferFormated[CursorFormated], &CurGraphicData.gData[Cursor], CurGraphicData.gWidth);
    }
    
    delete[] CurGraphicData.gData;
    CurGraphicData.gData = BufferFormated;
    BufferFormated = nullptr;
}

void FCGGraphicDecoder::SetColorBuff(uint32 GraphicId, FString PaletType, AssetVer Ver)
{
    SetGraphicData(GraphicId, Ver);
    
    if (CurGraphicData.gIscompressed)
    {
        DecodeGraphicData();
    }
    
    FormatGraphicData();
    
	ColorBuff = new FColor[CurGraphicData.gLength];
	for (uint32 i = 0; i < CurGraphicData.gLength; i++)
	{
		ColorBuff[i].B = CGPalet[PaletType].ColorData[CurGraphicData.gData[i]].Blue;
		ColorBuff[i].G = CGPalet[PaletType].ColorData[CurGraphicData.gData[i]].Green;
		ColorBuff[i].R = CGPalet[PaletType].ColorData[CurGraphicData.gData[i]].Red;
		if (CurGraphicData.gData[i] == alpha_index)
		{
			ColorBuff[i].A = alpha_level;
		}
        else
        {
            ColorBuff[i].A = 0xff;
        }
	}
    
    // gc SGData.gData
    delete[] CurGraphicData.gData;
    CurGraphicData.gData = nullptr;
}

void FCGGraphicDecoder::Test()
{
//    TArray<uint32> tileidarr;
//    CGTileInfo.GenerateKeyArray(tileidarr);
//    uint32 tilenum = CGTileInfo.Num();
//    UE_LOG(LogTemp, Warning, TEXT("num:%d, max:%d"), tilenum, tileidarr[tilenum-1]);
    
    
    

    
//    FString mapSetBlack;
//    TSet<uint32> MapBlackList;
//    for (auto& TileUnit :TileInfo)
//    {
//        if (TileUnit.Value.bHasTileId == 0)
//        {
//            MapBlackList.Append(TileUnit.Value.Map);
//        }
//    }
//    MapBlackList.Sort([](uint32 A, uint32 B) {return A > B;});
//    for (auto& Elem :MapBlackList)
//    {
//        mapSetBlack += FString::FromInt(Elem) + TEXT("|");
//    }
//    UE_LOG(LogTemp, Warning, TEXT("%s"), *mapSetBlack);

//    for (auto &Key : MapList)
//    {
//        SetMapData(Key);
//        uint32 Length = CurMap.mWidth * CurMap.mHeight;
//        uint32 counttransit = 0;
//        uint32 countnotransit = 0;
//        uint32 countpass = 0;
//        uint32 countnopass = 0;
//        uint32 countnomap = 0;
//        uint32 countoutt = 0;
//        uint32 countoutp = 0;
//        for (uint32 i = 0; i < Length; i++)
//        {
//            MapFlag Flag = CurMap.mFlagLayer[i];
//
//            switch (Flag.fTransit)
//            {
//                case 0:
//                    countnotransit++;
//                    break;
//                case 10:
//                    counttransit++;
//                    break;
//                default:
//                    countoutt++;
//                    UE_LOG(LogTemp, Warning, TEXT("%d"), Flag.fTransit);
//                    break;
//            }
//
//            switch (Flag.fPass)
//            {
//                case 0:
//                    countnomap++;
//                    break;
//                case 192:
//                    countpass++;
//                    break;
//                case 193:
//                    countnopass++;
//                    break;
//                default:
//                    countoutp++;
//                    break;
//            }
//        }
//        //UE_LOG(LogTemp, Warning, TEXT("map:%d notransit:%d transit:%d o:%d nomap:%d pass:%d nopass:%d o:%d"), Key, countnotransit, counttransit, countoutt, countnomap, countpass, countnopass, countoutp);
//        delete[] CurMap.mTerrainLayer;
//        delete[] CurMap.mArtifactLayer;
//        delete[] CurMap.mFlagLayer;
//    }
    
    //ExportTileMapPng();
    //CreateTileMap(2000);
}
