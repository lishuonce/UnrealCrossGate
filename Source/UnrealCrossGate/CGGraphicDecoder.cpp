

#include "CGGraphicDecoder.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "ImageUtils.h"
#include "PaperTileSet.h"
#include "PaperTileMap.h"
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
        LoadAnimeInfo();
		LoadGraphicInfo();
        LoadPaletData();
        InitGraphicData();
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

UTexture2D * FCGGraphicDecoder::GetTexture2D(uint32 GraphicId, FString PaletType)
{
    SetColorBuff(GraphicId, PaletType);
    
	// create Texture2D
    UTexture2D *Tex2d = UTexture2D::CreateTransient(CGData.gWidth, CGData.gHeight, PF_B8G8R8A8);
    FTexture2DMipMap& Mip = Tex2d->PlatformData->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(Data, ColorBuff, CGData.gLength * sizeof(FColor));
    Mip.BulkData.Unlock();
    Tex2d->UpdateResource();
    
    // gc ColorBuff
    delete[] ColorBuff;
    ColorBuff = nullptr;
    
    return Tex2d;
}

void FCGGraphicDecoder::SaveToPng(uint32 GraphicId, FString PaletType)
{
	SetColorBuff(GraphicId, PaletType);

	// create png array
	TArray<FColor> SrcData;
	SrcData.Append(ColorBuff, CGData.gLength);
	TArray<uint8> DstData;
	FImageUtils::CompressImageArray(CGData.gWidth, CGData.gHeight, SrcData, DstData);

	// save png file
	FString fsTexturePath = FPaths::ProjectContentDir() + "CGRawDecode/MapTiles/";
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

void FCGGraphicDecoder::SaveTileToPng(uint32 GraphicId, FString PaletType)
{
	if (CGTileInfo.Contains(GraphicId))
	{
		SetColorBuff(GraphicId, PaletType);

		// create png array
		TArray<FColor> SrcData;
		SrcData.Append(ColorBuff, CGData.gLength);
		TArray<uint8> DstData;
		FImageUtils::CompressImageArray(CGData.gWidth, CGData.gHeight, SrcData, DstData);

		// save png file
		FString fsTexturePath = FPaths::ProjectContentDir() + "CGRawDecode/MapTilesNonStd/";
		if (CGInfo[GraphicId].gWidth == 64 && CGInfo[GraphicId].gHeight == 47)
		{
			fsTexturePath = FPaths::ProjectContentDir() + "CGRawDecode/MapTiles/";
		}
		FString fsTmpPngPath = fsTexturePath + FString::FromInt(CGInfo[GraphicId].gId) + ".png";// Filename : gid
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
}

void FCGGraphicDecoder::CreateTileMap(uint32 MapId)
{
    // create package
    FString PackageName = FString::FromInt(MapId);
    FString PackagePath = "/Game/Maps/" + PackageName;
    UPackage * Package = CreatePackage(nullptr, *PackagePath);
    EObjectFlags Flags = RF_Public|RF_Standalone|RF_Transactional;
    UPaperTileMap* TileMap = NewObject<UPaperTileMap>(Package, *PackageName, Flags);
    
    // set map data - base
    SetMapData(MapId);
    TileMap->MapWidth = CGMap.mWeight;
    TileMap->MapHeight = CGMap.mHeight;
    TileMap->TileWidth = 64;
    TileMap->TileHeight = 47;
    TileMap->ProjectionMode = ETileMapProjectionMode::IsometricDiamond;
    TileMap->TileLayers.Empty();
    
	// set map data - mTerrainLayer



    // save package
	Package->MarkPackageDirty();
    FString FilePath = FPaths::ProjectContentDir() + "Maps/";
    FString FileName = FilePath + PackageName + FPackageName::GetAssetPackageExtension();
    bool bSuccess = UPackage::SavePackage(Package, TileMap, Flags, *FileName);

}

void FCGGraphicDecoder::SetResPath()
{
    // todo : check launch dir, if not, let player define
    fsResPath = FPaths::ProjectContentDir() + "CGRaw/";// CG Raw Assets paths : Content/CGRaw
    // fsResPath = FPaths::LaunchDir();// CG Raw Assets paths : game launch pach
    fsGraphicInfoPath = fsResPath + "bin/GraphicInfo_20.bin";
    fsGraphicDataPath = fsResPath + "bin/Graphic_20.bin";
    fsAnimeInfoPath = fsResPath + "bin/AnimeInfo_3.bin";
    fsAnimeDataPath = fsResPath + "bin/Anime_3.bin";
}

bool FCGGraphicDecoder::IsResVerified()
{
    // todo : check file exists
    // todo : check md5
    
    if (FPaths::FileExists(fsGraphicInfoPath)
        && FPaths::FileExists(fsGraphicDataPath)
		)
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
	FString fsMapPath = fsResPath + "map/0/";
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
		MapList.Add(MapId);
	}
}

void FCGGraphicDecoder::SetMapData(uint32 MapId)
{
	// Set fsMapPath
	FString fsMapPath = fsResPath + "map/0/";
	FString fsMapExten = ".dat";
	FString fsMapName = FString::FromInt(MapId);
	FString fsMapFile = fsMapPath + fsMapName + fsMapExten;

	// Load *.dat to CGMap
	IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*fsMapFile);
	if (fileHandleTmp)
	{
		// Load GraphicMap Header : mHeader[3], mBlank[9], mWeight, mHeight
		fileHandleTmp->Read((uint8 *)&CGMap, 20);

		uint32 LayerLength = CGMap.mWeight * CGMap.mHeight;
        uint32 LayerSize = LayerLength * 2;

		// Load GraphicMap Layet : mTerrainLayer
		CGMap.mTerrainLayer = new uint16[LayerLength];
		fileHandleTmp->Read((uint8 *)CGMap.mTerrainLayer, LayerSize);

		// Load GraphicMap Layet : mArtifactLayer
		CGMap.mArtifactLayer = new uint16[LayerLength];
		fileHandleTmp->Read((uint8 *)CGMap.mArtifactLayer, LayerSize);

		// Load GraphicMap Layet : mFlagLayer
		CGMap.mFlagLayer = new MapFlag[LayerLength];
		fileHandleTmp->Read((uint8 *)CGMap.mFlagLayer, LayerSize);

		delete fileHandleTmp;
	}
}

void FCGGraphicDecoder::LoadAnimeInfo()
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*fsAnimeInfoPath);
    if (fileHandleTmp)
    {
        uint32 iFileSize = fileHandleTmp->Size();
        uint32 iRecordNum = iFileSize / sizeof(AnimeInfo);
        CGAnimeInfo = new AnimeInfo[iRecordNum];
        fileHandleTmp->Read((uint8 *)CGAnimeInfo, iFileSize);
        delete fileHandleTmp;
    }
}

void FCGGraphicDecoder::SetAnimeData(uint32 AnimeId)
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*fsAnimeDataPath);
    if (fileHandleTmp)
    {
        // Load Anime_*.bin Header
        FileHandle->Seek(CGAnimeInfo[AnimeId].aAddr);
        FileHandle->Read((uint8 *)&CGAnimeData, 12);
        
        // Load Anime_*.bin FrameData
        CGAnimeData.FrameData = new AnimeFrame[CGAnimeData.aFrameNum];
        uint32 DataSize = CGAnimeData.aFrameNum * 10;
        FileHandle->Read((uint8 *)CGAnimeData.FrameData, DataSize);
        delete fileHandleTmp;
    }
}

void FCGGraphicDecoder::LoadGraphicInfo()
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*fsGraphicInfoPath);
    if (fileHandleTmp)
    {
		// load CGInfo
        uint32 iFileSize = fileHandleTmp->Size();
        uint32 iRecordNum = iFileSize / sizeof(GraphicInfo);
        CGInfo = new GraphicInfo[iRecordNum];
        fileHandleTmp->Read((uint8 *)CGInfo, iFileSize);
        delete fileHandleTmp;
        
		// set CGTileInfo
        for (uint32 i=0; i<iRecordNum; i++)
        {
            if (CGInfo[i].gTileId != 0)
            {
                CGTileInfo.Emplace(CGInfo[i].gTileId, CGInfo[i].gId);
            }
        }
        CGTileInfo.KeySort([](uint32 A, uint32 B) {return A < B;});
    }

	// load tilesets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.ClassNames.Add(UPaperTileSet::StaticClass()->GetFName());
	Filter.PackagePaths.Add("/Game/Maps/TileSets");
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	// set CGTileInfoStd
	uint32 gIdStd = 0;
	for (auto& Elem : CGTileInfo)
	{
		if (CGInfo[Elem.Value].gWidth == 64 && CGInfo[Elem.Value].gHeight == 47)
		{
			UPaperTileSet *FirstTileSet = (UPaperTileSet *)AssetData[0].GetAsset();
			uint32 TileSetCount = FirstTileSet->GetTileCount();


			uint32 AssetIndex = FMath::DivideAndRoundDown(gIdStd, TileSetCount);
			uint32 AssetCount = gIdStd % TileSetCount;

			struct FPaperTileInfo TileInfo;
			TileInfo.TileSet = (UPaperTileSet *)AssetData[AssetIndex].GetAsset();
			TileInfo.PackedTileIndex = AssetCount;

			CGTileInfoStd.Emplace(Elem.Key, TileInfo);

			gIdStd++;
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
    FString fsPaletPath = fsResPath + "bin/pal/";
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

void FCGGraphicDecoder::InitGraphicData()
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    FileHandle = PlatFormFile.OpenRead(*fsGraphicDataPath);
}

void FCGGraphicDecoder::SetGraphicData(uint32 GraphicId)
{
	if (FileHandle)
	{
		// Load Graphic_*.bin Header
		FileHandle->Seek(CGInfo[GraphicId].gAddr);
		FileHandle->Read((uint8 *)&CGData, 16);

		// Load Graphic_*.bin gData
		CGData.gLength -= 16;
		CGData.gData = new uint8[CGData.gLength];
		FileHandle->Read(CGData.gData, CGData.gLength);
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
	uint8 *BufferDecoded = new uint8[CGData.gHeight*CGData.gWidth];

	while (BufferEncodedCursor < CGData.gLength)
	{
		uint8 High = CGData.gData[BufferEncodedCursor] >> 4;
		uint8 Low = CGData.gData[BufferEncodedCursor] & 0x0f;
		uint32 RLESize = 0;
		uint8 *RepeatBuffer;

		switch (High)
		{
		case 0x0:
			RLESize = Low;
			ERLEFlags = RLE_READ;
			break;
		case 0x1:
			RLESize = Low * 0x100 + CGData.gData[BufferEncodedCursor + 1];
			ERLEFlags = RLE_READ;
			break;
		case 0x2:
			RLESize = Low * 0x10000 + CGData.gData[BufferEncodedCursor + 1] * 0x100 + CGData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_READ;
			break;
		case 0x8:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0x9:
			RLESize = Low * 0x100 + CGData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xa:
			RLESize = Low * 0x10000 + CGData.gData[BufferEncodedCursor + 2] * 0x100 + CGData.gData[BufferEncodedCursor + 3];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xc:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xd:
			RLESize = Low * 0x100 + CGData.gData[BufferEncodedCursor + 1];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xe:
			RLESize = Low * 0x10000 + CGData.gData[BufferEncodedCursor + 1] * 0x100 + CGData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		default:
			UE_LOG(CGGraphicDecoder, Error, TEXT("JSS-RLE Decode Error - switch(High) Default: %x"), CGData.gData[BufferEncodedCursor]);
			break;
		}// while switch(High) END

		switch (ERLEFlags)
		{
		case RLE_READ:
			FMemory::Memcpy(&BufferDecoded[BufferDecodedCursor], &CGData.gData[BufferEncodedCursor + High + 1], RLESize);
			BufferDecodedCursor += RLESize;
			BufferEncodedCursor += High + 1 + RLESize;
			break;
		case RLE_REPEAT_BACKGROUND:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, CGData.gData[BufferEncodedCursor + 1], RLESize);
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
			UE_LOG(CGGraphicDecoder, Error, TEXT("JSS-RLE Decode Error - switch(ERLEFlags) Default: %x"), CGData.gData[BufferEncodedCursor]);
			break;
		}// switch(ERLEFlags) END

		ERLEFlags = RLE_NONE;

	}// while CurrentDecodePosition < SizeOfBuffer END

	// update SGData
	CGData.gIscompressed = 0;
	CGData.gLength = BufferDecodedCursor;
	delete[] CGData.gData;
	CGData.gData = BufferDecoded;
	BufferDecoded = nullptr;
}

void FCGGraphicDecoder::FormatGraphicData()
{
    uint8 *BufferFormated = new uint8[CGData.gLength];
    uint32 Cursor;
    uint32 CursorFormated;
    uint32 LineFormated;
    
    for (uint32 Line = 0; Line < CGData.gHeight; Line ++)
    {
        LineFormated = CGData.gHeight - Line - 1;
        Cursor = Line * CGData.gWidth;
        CursorFormated = LineFormated * (CGData.gWidth);
        FMemory::Memcpy(&BufferFormated[CursorFormated], &CGData.gData[Cursor], CGData.gWidth);
    }
    
    delete[] CGData.gData;
    CGData.gData = BufferFormated;
    BufferFormated = nullptr;
}

void FCGGraphicDecoder::SetColorBuff(uint32 GraphicId, FString PaletType)
{
    SetGraphicData(GraphicId);
    
    if (CGData.gIscompressed)
    {
        DecodeGraphicData();
    }
    
    FormatGraphicData();
    
	ColorBuff = new FColor[CGData.gLength];
	for (uint32 i = 0; i < CGData.gLength; i++)
	{
		ColorBuff[i].B = CGPalet[PaletType].ColorData[CGData.gData[i]].Blue;
		ColorBuff[i].G = CGPalet[PaletType].ColorData[CGData.gData[i]].Green;
		ColorBuff[i].R = CGPalet[PaletType].ColorData[CGData.gData[i]].Red;
		if (CGData.gData[i] == alpha_index)
		{
			ColorBuff[i].A = alpha_level;
		}
        else
        {
            ColorBuff[i].A = 0xff;
        }
	}
    
    // gc SGData.gData
    delete[] CGData.gData;
    CGData.gData = nullptr;
}

void FCGGraphicDecoder::Test()
{
//    TArray<uint32> tileidarr;
//    CGTileInfo.GenerateKeyArray(tileidarr);
//    uint32 tilenum = CGTileInfo.Num();
//    UE_LOG(LogTemp, Warning, TEXT("num:%d, max:%d"), tilenum, tileidarr[tilenum-1]);
    
//    for (auto &Key : MapList)
//    {
//        SetMapData(Key);
//        uint32 Length = CGMap.mWeight * CGMap.mHeight;
//        uint32 countstd = 0;
//        uint32 countnonstd = 0;
//        uint32 count0 = 0;
//        uint32 count2 = 0;
//        for (uint32 i = 0; i < Length; i++)
//        {
//            uint16 TileId = CGMap.mArtifactLayer[i];
//            if (CGTileInfo.Contains(TileId))
//            {
//                uint32 gId = CGTileInfo[TileId];
//                if (CGInfo[gId].gIsNonStd)
//                {
//                    //UE_LOG(LogTemp, Warning, TEXT("w:%d h:%d x:%d y:%d"), CGInfo[gId].gWidth, CGInfo[gId].gHeight, CGInfo[gId].gOffsetX, CGInfo[gId].gOffsetY);
//                    countnonstd++;
//                }
//                else
//                {
//                    countstd++;
//                }
//            }
//            else if (TileId == 0)
//            {
//                count0++;
//            }
//            else if (TileId == 2)
//            {
//                count2++;
//            }
//        }
//        UE_LOG(LogTemp, Warning, TEXT("map:%d std:%d n:%d 0:%d 2:%d l:%d"), Key, countstd, countnonstd, count0, count2, Length);
//        delete[] CGMap.mTerrainLayer;
//        delete[] CGMap.mArtifactLayer;
//        delete[] CGMap.mFlagLayer;
//    }
    
//    for (auto &Key : MapList)
//    {
//        SetMapData(Key);
//        uint32 Length = CGMap.mWeight * CGMap.mHeight;
//        uint32 counttransit = 0;
//        uint32 countnotransit = 0;
//        uint32 countpass = 0;
//        uint32 countnopass = 0;
//        uint32 countnomap = 0;
//        uint32 countoutt = 0;
//        uint32 countoutp = 0;
//        for (uint32 i = 0; i < Length; i++)
//        {
//            MapFlag Flag = CGMap.mFlagLayer[i];
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
//        delete[] CGMap.mTerrainLayer;
//        delete[] CGMap.mArtifactLayer;
//        delete[] CGMap.mFlagLayer;
//    }
    
    //CreateTileMap(1000);
    
}
