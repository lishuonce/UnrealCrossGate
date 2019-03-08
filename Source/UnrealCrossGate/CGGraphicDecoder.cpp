

#include "CGGraphicDecoder.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "ImageUtils.h"

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
}

FCGGraphicDecoder::~FCGGraphicDecoder()
{
    // static Singleton will not excute deconstructor
    // dont implement
}

UTexture2D * FCGGraphicDecoder::GetTexture2D(uint32 GraphicId, FString PaletType)
{
    LoadGraphicData(GraphicId);
    
    if (SGData.gIscompressed)
    {
        DecodeGraphicData();
    }
    
    FormatGraphicData();
    
    SetColorBuff(PaletType);
    
    UTexture2D *Tex2d = UTexture2D::CreateTransient(SGData.gWidth, SGData.gHeight, PF_B8G8R8A8);
    FTexture2DMipMap& Mip = Tex2d->PlatformData->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    //uint8* Data = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
    FMemory::Memcpy(Data, ColorBuff, SGData.gLength * sizeof(FColor));
    Mip.BulkData.Unlock();
    Tex2d->UpdateResource();
    
    // gc SGData.gData , ColorBuff
    delete[] SGData.gData;
    SGData.gData = nullptr;
    
    delete[] ColorBuff;
    ColorBuff = nullptr;
    
    
    return Tex2d;
}

void FCGGraphicDecoder::SetResPath()
{
    // todo : check launch dir, if not, let player define
    fsResPath = FPaths::ProjectContentDir() + "CGRaw/";// CG Raw Assets paths : Content/CGRaw
    // fsResPath = FPaths::LaunchDir();// CG Raw Assets paths : game launch pach
    fsGraphicInfoPath = fsResPath + "bin/GraphicInfo_20.bin";
    fsGraphicDataPath = fsResPath + "bin/Graphic_20.bin";
    fsPaletDataPath = fsResPath + "bin/pal/palet_00.cgp";
}

bool FCGGraphicDecoder::IsResVerified()
{
    // todo : check file exists
    // todo : check md5
    
    if (FPaths::FileExists(fsGraphicInfoPath)
        && FPaths::FileExists(fsGraphicDataPath)
        && FPaths::FileExists(fsPaletDataPath))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void FCGGraphicDecoder::LoadGraphicInfo()
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*fsGraphicInfoPath);
    if (fileHandleTmp)
    {
        uint32 iFileSize = fileHandleTmp->Size();
        uint32 iRecordNum = iFileSize / 40;
        SGInfo = new GraphicInfo[iRecordNum];
        fileHandleTmp->Read((uint8 *)SGInfo, iFileSize);
        delete fileHandleTmp;
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
    FString PaletType;
    PlatFormFile.FindFiles(FileList, *fsPaletPath, *fsPaletExten);

	// Load Palet_*.cgp to PaletMap.sPalet
	for (int32 i = 0; i < FileList.Num(); i++)
    {
        Palet sPalet;
        IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*FileList[i]);
        if (fileHandleTmp)
        {
            // Palet Color 16 ~ 239 (Palet_*.cgp)
            fileHandleTmp->Read((uint8 *)(sPalet.sPalet + 16), 672);
            delete fileHandleTmp;
        }
        
        // Set Palet (common)
        FMemory::Memcpy(sPalet.sPalet, CommonPaletHead, 48);
        FMemory::Memcpy(sPalet.sPalet + 240, CommonPaletEnd, 48);
        
        // Add to PaletMAP
        PaletType = FPaths::GetBaseFilename(FileList[i]).RightChop(6);
        PaletMap.Emplace(PaletType, sPalet);
        UE_LOG(CGGraphicDecoder, Log, TEXT("PaletMap[%s] : %s"), *PaletType, *BytesToHex((uint8 *)&PaletMap[PaletType].sPalet, 768));
    }
    
	// Set loaded PaletTypes
    PaletMap.GenerateKeyArray(PaletTypes);
}

void FCGGraphicDecoder::InitGraphicData()
{
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    FileHandle = PlatFormFile.OpenRead(*fsGraphicDataPath);
}

void FCGGraphicDecoder::LoadGraphicData(uint32 GraphicId)
{
	UE_LOG(CGGraphicDecoder, Log, TEXT("gId:%i"), SGInfo[GraphicId].gId);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gAddr:%i"), SGInfo[GraphicId].gAddr);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gLength:%i"), SGInfo[GraphicId].gLength);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gOffsetX:%i"), SGInfo[GraphicId].gOffsetX);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gOffsetY:%i"), SGInfo[GraphicId].gOffsetY);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gWidth:%i"), SGInfo[GraphicId].gWidth);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gHeight:%i"), SGInfo[GraphicId].gHeight);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gEast:%i"), SGInfo[GraphicId].gEast);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gSouth:%i"), SGInfo[GraphicId].gSouth);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gIsFloor:%i"), SGInfo[GraphicId].gIsFloor);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gIsNonStd:%i"), SGInfo[GraphicId].gIsNonStd);
	UE_LOG(CGGraphicDecoder, Log, TEXT("gMapId:%i"), SGInfo[GraphicId].gMapId);

	if (FileHandle)
	{
		// Load Graphic_*.bin Header
		FileHandle->Seek(SGInfo[GraphicId].gAddr);
		FileHandle->Read((uint8 *)&SGData, 16);
		UE_LOG(CGGraphicDecoder, Log, TEXT("gRD:%c%c"), SGData.gHeader[0], SGData.gHeader[1]);
		UE_LOG(CGGraphicDecoder, Log, TEXT("gIscompressed:%d"), SGData.gIscompressed);
		UE_LOG(CGGraphicDecoder, Log, TEXT("unknown:%x"), SGData.unknown);
		UE_LOG(CGGraphicDecoder, Log, TEXT("gWidth:%d"), SGData.gWidth);
		UE_LOG(CGGraphicDecoder, Log, TEXT("gHeight:%d"), SGData.gHeight);
		UE_LOG(CGGraphicDecoder, Log, TEXT("gLength:%d"), SGData.gLength);

		// Load Graphic_*.bin gData
		SGData.gLength -= 16;
		SGData.gData = new uint8[SGData.gLength];
		FileHandle->Read(SGData.gData, SGData.gLength);
		UE_LOG(CGGraphicDecoder, Log, TEXT("%s"), *BytesToHex(SGData.gData, SGData.gLength));
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
	uint8 *BufferDecoded = new uint8[SGData.gHeight*SGData.gWidth];

	while (BufferEncodedCursor < SGData.gLength)
	{
		uint8 High = SGData.gData[BufferEncodedCursor] >> 4;
		uint8 Low = SGData.gData[BufferEncodedCursor] & 0x0f;
		uint32 RLESize = 0;
		uint8 *RepeatBuffer;

		switch (High)
		{
		case 0x0:
			RLESize = Low;
			ERLEFlags = RLE_READ;
			break;
		case 0x1:
			RLESize = Low * 0x100 + SGData.gData[BufferEncodedCursor + 1];
			ERLEFlags = RLE_READ;
			break;
		case 0x2:
			RLESize = Low * 0x10000 + SGData.gData[BufferEncodedCursor + 1] * 0x100 + SGData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_READ;
			break;
		case 0x8:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0x9:
			RLESize = Low * 0x100 + SGData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xa:
			RLESize = Low * 0x10000 + SGData.gData[BufferEncodedCursor + 2] * 0x100 + SGData.gData[BufferEncodedCursor + 3];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xc:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xd:
			RLESize = Low * 0x100 + SGData.gData[BufferEncodedCursor + 1];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xe:
			RLESize = Low * 0x10000 + SGData.gData[BufferEncodedCursor + 1] * 0x100 + SGData.gData[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		default:
			UE_LOG(CGGraphicDecoder, Error, TEXT("JSSRLEDecode - switch(High) Default: %x"), SGData.gData[BufferEncodedCursor]);
			break;
		}// while switch(High) END

		switch (ERLEFlags)
		{
		case RLE_READ:
			FMemory::Memcpy(&BufferDecoded[BufferDecodedCursor], &SGData.gData[BufferEncodedCursor + High + 1], RLESize);
			BufferDecodedCursor += RLESize;
			BufferEncodedCursor += High + 1 + RLESize;
			UE_LOG(CGGraphicDecoder, Log, TEXT("RLE_READ: %d"), RLESize);
			break;
		case RLE_REPEAT_BACKGROUND:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, SGData.gData[BufferEncodedCursor + 1], RLESize);
			FMemory::Memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
			BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 2;
			UE_LOG(CGGraphicDecoder, Log, TEXT("RLE_REPEAT_BACKGROUND: %d"), RLESize);
			break;
		case RLE_REPEAT_TRANSPARENT:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, alpha_index, RLESize);
			FMemory::Memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
			BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 1;
			UE_LOG(CGGraphicDecoder, Log, TEXT("RLE_REPEAT_TRANSPARENT: %d"), RLESize);
			break;
		default:
			UE_LOG(CGGraphicDecoder, Error, TEXT("JSSRLEDecode - switch(ERLEFlags) Default: %x"), SGData.gData[BufferEncodedCursor]);
			break;
		}// switch(ERLEFlags) END

		ERLEFlags = RLE_NONE;

	}// while CurrentDecodePosition < SizeOfBuffer END

	// update SGData
	SGData.gIscompressed = 0;
	SGData.gLength = BufferDecodedCursor;
	delete[] SGData.gData;
	SGData.gData = BufferDecoded;
	BufferDecoded = nullptr;

	UE_LOG(CGGraphicDecoder, Log, TEXT("JSSRLEDecoded Data size : %d"), BufferDecodedCursor);
	for (uint32 Line = 0; Line < SGData.gHeight; Line++)
	{
		UE_LOG(CGGraphicDecoder, Log, TEXT("%s"), *BytesToHex(&SGData.gData[SGData.gWidth * Line], SGData.gWidth));
	}
}

void FCGGraphicDecoder::FormatGraphicData()
{
    uint8 *BufferFormated = new uint8[SGData.gLength];
    uint32 Cursor;
    uint32 CursorFormated;
    uint32 LineFormated;
    
    for (uint32 Line = 0; Line < SGData.gHeight; Line ++)
    {
        LineFormated = SGData.gHeight - Line - 1;
        Cursor = Line * SGData.gWidth;
        CursorFormated = LineFormated * (SGData.gWidth);
        FMemory::Memcpy(&BufferFormated[CursorFormated], &SGData.gData[Cursor], SGData.gWidth);
    }
    
    delete[] SGData.gData;
    SGData.gData = BufferFormated;
    BufferFormated = nullptr;
}

void FCGGraphicDecoder::SetColorBuff(FString PaletType)
{
	ColorBuff = new FColor[SGData.gLength];
	for (uint32 i = 0; i < SGData.gLength; i++)
	{
		ColorBuff[i].B = PaletMap[PaletType].sPalet[SGData.gData[i]].Blue;
		ColorBuff[i].G = PaletMap[PaletType].sPalet[SGData.gData[i]].Green;
		ColorBuff[i].R = PaletMap[PaletType].sPalet[SGData.gData[i]].Red;
		if (SGData.gData[i] == alpha_index)
		{
			ColorBuff[i].A = alpha_level;
		}
        else
        {
            ColorBuff[i].A = 0xff;
        }
	}
    
	UE_LOG(CGGraphicDecoder, Log, TEXT("%s"), *BytesToHex((uint8 *)ColorBuff, SGData.gLength));
}
