// Fill out your copyright notice in the Description page of Project Settings.

#include "CGGraphicDecoder.h"

CGGraphicDecoder & CGGraphicDecoder::Get()
{
	static CGGraphicDecoder Singleton;
	return Singleton;
}

CGGraphicDecoder::CGGraphicDecoder()
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
}

CGGraphicDecoder::~CGGraphicDecoder()
{
	//static Singleton will not excute deconstructor
    //dont implement
}

uint8 * CGGraphicDecoder::GetDecodePngData(uint32 GraphicId, FString PaletType)
{
	//DEBUG LOG - GraphicInfo
//    UE_LOG(LogTemp, Warning, TEXT("gId:%i"), SGInfo[GraphicId].gId);
//    UE_LOG(LogTemp, Warning, TEXT("gAddr:%i"), SGInfo[GraphicId].gAddr);
//    UE_LOG(LogTemp, Warning, TEXT("gLength:%i"), SGInfo[GraphicId].gLength);
//    UE_LOG(LogTemp, Warning, TEXT("gOffsetX:%i"), SGInfo[GraphicId].gOffsetX);
//    UE_LOG(LogTemp, Warning, TEXT("gOffsetY:%i"), SGInfo[GraphicId].gOffsetY);
//    UE_LOG(LogTemp, Warning, TEXT("gWidth:%i"), SGInfo[GraphicId].gWidth);
//    UE_LOG(LogTemp, Warning, TEXT("gHeight:%i"), SGInfo[GraphicId].gHeight);
//    UE_LOG(LogTemp, Warning, TEXT("gEast:%i"), SGInfo[GraphicId].gEast);
//    UE_LOG(LogTemp, Warning, TEXT("gSouth:%i"), SGInfo[GraphicId].gSouth);
//    UE_LOG(LogTemp, Warning, TEXT("gIsFloor:%i"), SGInfo[GraphicId].gIsFloor);
//    UE_LOG(LogTemp, Warning, TEXT("gMapId:%i"), SGInfo[GraphicId].gMapId);
    
    //Load GraphicData
	if (fileHandle)
	{
		//Load Header
        GraphicData SGData;
		fileHandle->Seek(SGInfo[GraphicId].gAddr);
		fileHandle->Read((uint8 *)&SGData, 16);
        
        //DEBUG LOG - GraphicData
//        UE_LOG(LogTemp, Warning, TEXT("gRD:%c%c"), SGData.gHeader[0], SGData.gHeader[1]);
//        UE_LOG(LogTemp, Warning, TEXT("gIscompressed:%d"), SGData.gIscompressed);
//        UE_LOG(LogTemp, Warning, TEXT("unknown:%x"), SGData.unknown);
//        UE_LOG(LogTemp, Warning, TEXT("gWidth:%d"), SGData.gWidth);
//        UE_LOG(LogTemp, Warning, TEXT("gHeight:%d"), SGData.gHeight);
//        UE_LOG(LogTemp, Warning, TEXT("gLength:%d"), SGData.gLength);
		
        //Load Data
		uint32 GDataLength = SGInfo[GraphicId].gLength - 16;
		SGData.gData = new uint8[GDataLength];
		fileHandle->Read(SGData.gData, GDataLength);
        //UE_LOG(LogTemp, Warning, TEXT("%s"), *BytesToHex(SGData.gData, GDataLength));
        
        uint8 *Buffer = SGData.gData;
        uint32 BufferSize = GDataLength;
        
        //JSSRLEDecode
        if (SGData.gIscompressed)
        {
            BufferSize = SGData.gWidth * SGData.gHeight;
            Buffer = new uint8[BufferSize];
            JSSRLEDecode(SGData.gData, GDataLength, Buffer, BufferSize);
            delete [] SGData.gData;
            SGData.gData = Buffer;
            Buffer = nullptr;
            GDataLength = BufferSize;
        }
//        for (uint32 Line = 0; Line < SGData.gHeight; Line++)
//        {
//            UE_LOG(LogTemp, Log, TEXT("%s"), *BytesToHex(&SGData.gData[SGData.gWidth * Line], SGData.gWidth));
//        }
        
        //PNG: Encode
        BufferSize = 1105 + FCompression::CompressMemoryBound(COMPRESS_ZLIB, GDataLength + SGData.gHeight);
        Buffer = new uint8[BufferSize];
        PNGEncode(SGData.gData, GDataLength, Buffer, BufferSize, SGData.gWidth, SGData.gHeight, PaletType);
        delete [] SGData.gData;
        SGData.gData = Buffer;
        Buffer = nullptr;
        GDataLength = BufferSize;
        
        //Save PNGBuffer to Files
        FString fsTexturePath = FPaths::ProjectContentDir() + "Textures/MapTiles/";
        //Filename : random
        //FString fsTmpPngPath = FPaths::CreateTempFilename(*fsTexturePath, TEXT("tmp"), TEXT(".png"));
        //Filename : GraphicID + PaletType
        FString fsTmpPngPath = fsTexturePath + FString::FromInt(GraphicId) + "_" + PaletType + ".png";
        IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
        IFileHandle *fileHandleTmp = platFormFile.OpenWrite(*fsTmpPngPath);
        if (fileHandleTmp)
        {
            fileHandleTmp->Write(SGData.gData, GDataLength);
            delete fileHandleTmp;
        }
        
        //return Data
        return SGData.gData;
	}
	return nullptr;
}

void CGGraphicDecoder::SetResPath()
{
	fsResPath = FPaths::ProjectContentDir() + "CGRaw/";
	//fsResPath = FPaths::LaunchDir();
	fsGraphicInfoPath = fsResPath + "bin/GraphicInfo_20.bin";
	fsGraphicDataPath = fsResPath + "bin/Graphic_20.bin";
	fsPaletDataPath = fsResPath + "bin/pal/palet_00.cgp";
}

bool CGGraphicDecoder::IsResVerified()
{
	//check file exists
	//check md5

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

void CGGraphicDecoder::LoadGraphicInfo()
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

void CGGraphicDecoder::LoadPaletData()
{
    //Palet Color 0 ~ 15 (common)
    uint8 CommonPaletHead[48] = {
        0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0x80,
        0x00,0x80,0x00,0x80,0x80,0xc0,0xc0,0xc0,0xc0,0xdc,0xc0,0xa6,0xca,0xf0,0xde,0x00,
        0x00,0xff,0x5f,0x00,0xff,0xff,0xa0,0x00,0x5f,0xd2,0x50,0xd2,0xff,0x28,0xe1,0x28};
    //Palet Color 240 ~ 255 (common)
    uint8 CommonPaletEnd[48] = {
        0xf5,0xc3,0x96,0xe1,0xa0,0x5f,0xc3,0x7d,0x46,0x90,0x55,0x1e,0x46,0x41,0x37,0x28,
        0x23,0x1e,0xff,0xfb,0xf0,0xa0,0xa0,0xa4,0x80,0x80,0x80,0xff,0x00,0x00,0x00,0xff,
        0x00,0xff,0xff,0x00,0x00,0x00,0xff,0xff,0x80,0xff,0x00,0xff,0xff,0xff,0xff,0xff};
    //Load Palet_*.cgp to PaletMap.sPalet
    FString fsPaletPath = fsResPath + "bin/pal/";
    FString fsPaletExten = ".cgp";
    IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
    TArray<FString> FileList;
    FString PaletType;
    PlatFormFile.FindFiles(FileList, *fsPaletPath, *fsPaletExten);
    for (uint32 i = 0; i < FileList.Num(); i++)
    {
        Palet sPalet;
        IFileHandle *fileHandleTmp = PlatFormFile.OpenRead(*FileList[i]);
        if (fileHandleTmp)
        {
            //Palet Color 16 ~ 239 (Palet_*.cgp)
            fileHandleTmp->Read((uint8 *)(sPalet.sPalet + 16), 672);
            delete fileHandleTmp;
        }
        //Convert BGR to RGB
        for (uint8 j = 16; j < 239; j++)
        {
            Swap(sPalet.sPalet[j].Blue, sPalet.sPalet[j].Red);
        }
        //Set Palet (common)
        memcpy(sPalet.sPalet, CommonPaletHead, 48);
        memcpy(sPalet.sPalet + 240, CommonPaletEnd, 48);
        //Add to PaletMAP
        PaletType = FPaths::GetBaseFilename(FileList[i]).RightChop(6);
        PaletMap.Emplace(PaletType, sPalet);
        //UE_LOG(LogTemp, Warning, TEXT("PaletMap[%s] : %s"), *PaletType, *BytesToHex((uint8 *)&PaletMap[PaletType].sPalet, 768));
    }
}

void CGGraphicDecoder::InitGraphicData()
{
	IPlatformFile &PlatFormFile = FPlatformFileManager::Get().GetPlatformFile();
	fileHandle = PlatFormFile.OpenRead(*fsGraphicDataPath);
}

void CGGraphicDecoder::JSSRLEDecode(uint8 *BufferEncoded, uint32 SizeOfBufferEncoded, uint8 *BufferDecoded, uint32 SizeOfBufferDecoded)
{
	//RLEFlags for RLE Decode
    enum
    {
        RLE_NONE,
        RLE_READ,
        RLE_REPEAT_BACKGROUND,
        RLE_REPEAT_TRANSPARENT
    }ERLEFlags = RLE_NONE;
    
    uint32 BufferDecodedCursor = 0;
    uint32 BufferEncodedCursor = 0;
    
    while (BufferEncodedCursor < SizeOfBufferEncoded)
	{
		uint8 High = BufferEncoded[BufferEncodedCursor] >> 4;
		uint8 Low = BufferEncoded[BufferEncodedCursor] & 0x0f;
		uint32 RLESize;
		uint8 *RepeatBuffer;
        
		switch (High)
		{
		case 0x0:
			RLESize = Low;
			ERLEFlags = RLE_READ;
			break;
		case 0x1:
			RLESize = Low * 0x100 + BufferEncoded[BufferEncodedCursor + 1];
			ERLEFlags = RLE_READ;
			break;
		case 0x2:
			RLESize = Low * 0x10000 + BufferEncoded[BufferEncodedCursor + 1] * 0x100 + BufferEncoded[BufferEncodedCursor + 2];
			ERLEFlags = RLE_READ;
			break;
		case 0x8:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0x9:
			RLESize = Low * 0x100 + BufferEncoded[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xa:
			RLESize = Low * 0x10000 + BufferEncoded[BufferEncodedCursor + 2] * 0x100 + BufferEncoded[BufferEncodedCursor + 3];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xc:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xd:
			RLESize = Low * 0x100 + BufferEncoded[BufferEncodedCursor + 1];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xe:
			RLESize = Low * 0x10000 + BufferEncoded[BufferEncodedCursor + 1] * 0x100 + BufferEncoded[BufferEncodedCursor + 2];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("switch(High) Default: %x"), BufferEncoded[BufferEncodedCursor]);
			break;
		}//while switch(High) END

		switch (ERLEFlags)
		{
		case RLE_READ:
            memcpy(&BufferDecoded[BufferDecodedCursor], &BufferEncoded[BufferEncodedCursor + High + 1], RLESize);
            BufferDecodedCursor += RLESize;
			BufferEncodedCursor += High + 1 + RLESize;
			//UE_LOG(LogTemp, Warning, TEXT("RLE_READ: %d"), RLESize);
			break;
		case RLE_REPEAT_BACKGROUND:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, BufferEncoded[BufferEncodedCursor + 1], RLESize);
            memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
            BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 2;
			//UE_LOG(LogTemp, Warning, TEXT("RLE_REPEAT_BACKGROUND: %d"), RLESize);
			break;
		case RLE_REPEAT_TRANSPARENT:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, 0x00, RLESize);//set transparent color
            memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
            BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 1;
			//UE_LOG(LogTemp, Warning, TEXT("RLE_REPEAT_TRANSPARENT: %d"), RLESize);
			break;
		default:
            UE_LOG(LogTemp, Error, TEXT("switch(ERLEFlags) Default: %x"), BufferEncoded[BufferEncodedCursor]);
			break;
		}//switch(ERLEFlags) END
        
        ERLEFlags = RLE_NONE;
        
	}//while CurrentDecodePosition < SizeOfBuffer END
    //UE_LOG(LogTemp, Warning, TEXT("Decoded Data size : %d"), BufferDecodedCursor);
}

void CGGraphicDecoder::PNGEncode(uint8 *Buffer, uint32 SizeOfBuffer, uint8 *PNGBuffer, uint32 &SizeOfPNGBuffer, uint32 PicWidth, uint32 PicHeight, FString PaletType)
{
    //Line Format
    uint32 SizeOfBufferFormated = SizeOfBuffer + PicHeight;
    uint8 *BufferFormated = new uint8[SizeOfBufferFormated];
    uint32 Cursor;
    uint32 CursorFormated;
    uint32 LineFormated;
    //UE_LOG(LogTemp, Warning, TEXT("BufferFormated size in Function : %d"), SizeOfBufferFormated);
    for (uint32 Line = 0; Line < PicHeight; Line += 1)
    {
        LineFormated = PicHeight - 1 - Line;
        Cursor = Line * PicWidth;
        CursorFormated = LineFormated * (PicWidth + 1);
        BufferFormated[CursorFormated] = 0x00;
        memcpy(&BufferFormated[CursorFormated + 1], &Buffer[Cursor], PicWidth);
        //UE_LOG(LogTemp, Log, TEXT("%s"), *BytesToHex(&BufferFormated[CursorFormated], PicWidth + 1));
    }
    
    //PNGBuffer Cursor reset
    uint32 PNGBufferCursor = 0;
    
    //PNG_Header
    uint8 PNG_Title[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    memcpy(PNGBuffer, &PNG_Title, 8);
    PNGBufferCursor += 8;
    //UE_LOG(LogTemp, Warning, TEXT("PNG_Title : %d"), PNGBufferCursor);
    
    //Chunk_IHDR
    struct Chunk_IHDR
    {
        uint32 Width;
        uint32 Height;
        const uint8 BitDepth = 8;
        uint8 ColorType = 3;
        uint8 CompressionMethod = 0;
        uint8 FilterMethod = 0;
        uint8 InterlaceMethod = 0;
    };
    bool bIsLittleEndian = FGenericPlatformProperties::IsLittleEndian();
    uint32 PicWidthBigendian = bIsLittleEndian ? BYTESWAP_ORDER32_unsigned(PicWidth) : PicWidth;
    uint32 PicHeightBigendian = bIsLittleEndian ? BYTESWAP_ORDER32_unsigned(PicHeight) : PicHeight;
    Chunk_IHDR sChunk_IHDR = {PicWidthBigendian, PicHeightBigendian, 8, 3, 0, 0, 0};
    AppendChunk(PNGBuffer, PNGBufferCursor, 13, "IHDR", &sChunk_IHDR);
    
    //Chunk_PLTE
    AppendChunk(PNGBuffer, PNGBufferCursor, 768, "PLTE", PaletMap[PaletType].sPalet);
    
    //Chunk_tRNS
    uint8 Chunk_tRNS[256];
    memset(Chunk_tRNS, 0xff, 255);
    Chunk_tRNS[0] = 0x00;//set transparent color
    AppendChunk(PNGBuffer, PNGBufferCursor, 256, "tRNS", Chunk_tRNS);
    
    //Chunk_IDAT
    int32 CompressedLength = FCompression::CompressMemoryBound(COMPRESS_ZLIB, SizeOfBufferFormated);
    uint8 *CompressedBuffer = new uint8[CompressedLength];
    if (FCompression::CompressMemory(COMPRESS_ZLIB, CompressedBuffer, CompressedLength, BufferFormated, SizeOfBufferFormated))
    {
        //UE_LOG(LogTemp, Warning, TEXT("FCompression::CompressMemory CompressedLength: %d"), CompressedLength);
        AppendChunk(PNGBuffer, PNGBufferCursor, CompressedLength, "IDAT", CompressedBuffer);
        delete [] BufferFormated;
        BufferFormated = nullptr;
        delete [] CompressedBuffer;
        CompressedBuffer = nullptr;
    }
    
    //Chunk_IEND
    AppendChunk(PNGBuffer, PNGBufferCursor, 0, "IEND", nullptr);
    
    //Update SizeOfPNGBuffer
    SizeOfPNGBuffer = PNGBufferCursor;
}

void CGGraphicDecoder::AppendChunk(uint8 *PNGBuffer, uint32 &PNGBufferCursor, uint32 ChunkLength, FString ChunkTypeCode, void *ChunkData)
{
    //ChunkLength
    bool bIsLittleEndian = FGenericPlatformProperties::IsLittleEndian();
    uint32 ChunkLengthBigendian = bIsLittleEndian ? BYTESWAP_ORDER32_unsigned(ChunkLength) : ChunkLength;
    memcpy(&PNGBuffer[PNGBufferCursor], &ChunkLengthBigendian, 4);
    PNGBufferCursor += 4;
    
    //ChunkTypeCode
    memcpy(&PNGBuffer[PNGBufferCursor], (uint8 *)TCHAR_TO_ANSI(*ChunkTypeCode), 4);
    PNGBufferCursor += 4;
    
    //ChunkData
    if (ChunkLength > 0)
    {
        memcpy(&PNGBuffer[PNGBufferCursor], ChunkData, ChunkLength);
        PNGBufferCursor += ChunkLength;
    }
   
    //ChunkCRC
    uint32 CRCIndex = PNGBufferCursor - ChunkLength - 4;
    uint32 ChunkCRC = FCrc::MemCrc32(&PNGBuffer[CRCIndex], ChunkLength + 4);
    uint32 ChunkCRCBigendian = bIsLittleEndian ? BYTESWAP_ORDER32_unsigned(ChunkCRC) : ChunkCRC;
    memcpy(&PNGBuffer[PNGBufferCursor], &ChunkCRCBigendian, 4);
    PNGBufferCursor += 4;
    
    //UE_LOG(LogTemp, Warning, TEXT("PNG_%s + %d : , BufferCursor : %d"), *ChunkTypeCode, ChunkLength + 12, PNGBufferCursor);
}

void CGGraphicDecoder::test()
{
    IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = platFormFile.OpenRead(*fsGraphicDataPath);
    GraphicData SGData[253662];
    uint8 unknownarr[253662];
    if (fileHandleTmp)
    {
        for (uint32 i = 0; i < 253662; i++) {
            fileHandle->Seek(SGInfo[i].gAddr);
            fileHandle->Read((uint8 *)&SGData[i], 16);
            unknownarr[i] = SGData[i].unknown;
        }
        delete fileHandleTmp;
    }
    FString fsTmpPath = fsResPath + "test.bin";
    IFileHandle *fileHandleTmp2 = platFormFile.OpenWrite(*fsTmpPath);
    if (fileHandleTmp2) {
        fileHandleTmp2->Write(unknownarr, 253662);
        delete fileHandleTmp2;
    }
}

TArray<FString> CGGraphicDecoder::GetPaletTypeList()
{
    TArray<FString> PaletMapKey;
    PaletMap.GenerateKeyArray(PaletMapKey);
    return PaletMapKey;
}
