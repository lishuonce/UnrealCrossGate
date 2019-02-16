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

uint8 * CGGraphicDecoder::GetDecodePngData(uint32 GraphicId, uint8 PaletId)
{
	//DEBUG LOG - GraphicInfo
    UE_LOG(LogTemp, Warning, TEXT("gId:%i"), SGInfo[GraphicId].gId);
	UE_LOG(LogTemp, Warning, TEXT("gAddr:%i"), SGInfo[GraphicId].gAddr);
	UE_LOG(LogTemp, Warning, TEXT("gLength:%i"), SGInfo[GraphicId].gLength);
	UE_LOG(LogTemp, Warning, TEXT("gOffsetX:%i"), SGInfo[GraphicId].gOffsetX);
	UE_LOG(LogTemp, Warning, TEXT("gOffsetY:%i"), SGInfo[GraphicId].gOffsetY);
	UE_LOG(LogTemp, Warning, TEXT("gWidth:%i"), SGInfo[GraphicId].gWidth);
	UE_LOG(LogTemp, Warning, TEXT("gHeight:%i"), SGInfo[GraphicId].gHeight);
	UE_LOG(LogTemp, Warning, TEXT("gEast:%i"), SGInfo[GraphicId].gEast);
	UE_LOG(LogTemp, Warning, TEXT("gSouth:%i"), SGInfo[GraphicId].gSouth);
	UE_LOG(LogTemp, Warning, TEXT("gIsFloor:%i"), SGInfo[GraphicId].gIsFloor);
	UE_LOG(LogTemp, Warning, TEXT("gMapId:%i"), SGInfo[GraphicId].gMapId);
    
    //Load GraphicData
	if (fileHandle)
	{
		//Load Header
        GraphicData SGData;
		fileHandle->Seek(SGInfo[GraphicId].gAddr);
		fileHandle->Read((uint8 *)&SGData, 16);
        
        //DEBUG LOG - GraphicData
        UE_LOG(LogTemp, Warning, TEXT("gRD:%c%c"), SGData.gRD[0], SGData.gRD[1]);
		UE_LOG(LogTemp, Warning, TEXT("gIscompressed:%i"), SGData.gIscompressed);
		UE_LOG(LogTemp, Warning, TEXT("gWidth:%i"), SGData.gWidth);
		UE_LOG(LogTemp, Warning, TEXT("gHeight:%i"), SGData.gHeight);
		UE_LOG(LogTemp, Warning, TEXT("gLength:%i"), SGData.gLength);
		
        //Load Data
		uint32 GDataLength = SGInfo[GraphicId].gLength - 16;
		SGData.gData = new uint8[GDataLength];
		fileHandle->Read(SGData.gData, GDataLength);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *BytesToHex(SGData.gData, GDataLength));
        
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
        
        //PNG: Encode
        BufferSize = 1105 + FCompression::CompressMemoryBound(COMPRESS_ZLIB, GDataLength + SGData.gHeight);
        Buffer = new uint8[BufferSize];
        PNGEncode(SGData.gData, GDataLength, Buffer, BufferSize, SGData.gWidth, SGData.gHeight);
        delete [] SGData.gData;
        SGData.gData = Buffer;
        Buffer = nullptr;
        GDataLength = BufferSize;
        
        //Save PNGBuffer to random file
        FString fsTexturePath = FPaths::ProjectContentDir() + "Textures/MapTiles/";
        //FString fsTmpPngPath = FPaths::CreateTempFilename(*fsTexturePath, TEXT("tmp"), TEXT(".png"));
        //Save PNG files to name = GraphicID
        FString fsTmpPngPath = fsTexturePath + FString::FromInt(GraphicId) + ".png";
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
	IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle *fileHandleTmp = platFormFile.OpenRead(*fsGraphicInfoPath);
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
	IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle *fileHandleTmp = platFormFile.OpenRead(*fsPaletDataPath);
	if (fileHandleTmp)
	{
		//Palet Color 16 ~ 239
		fileHandleTmp->Read((uint8 *)(sPalet_00 + 16), 672);
		delete fileHandleTmp;
	}
    
    uint8 CommonPaletA[48] = {
        0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0x80,
        0x00,0x80,0x00,0x80,0x80,0xc0,0xc0,0xc0,0xc0,0xd0,0xc0,0xa6,0xca,0xf0,0xde,0x00,
        0x00,0xff,0x5f,0x00,0xff,0xff,0xa0,0x00,0x5f,0xd2,0x50,0xd2,0xff,0x28,0xe1,0x28};
    
    uint8 CommonPaletB[48] = {
        0xf5,0xc3,0x96,0x1e,0xa0,0x5f,0xc3,0x7d,0x46,0x9b,0x55,0x1e,0x46,0x41,0x37,0x28,
        0x23,0x1e,0xff,0xfb,0xf0,0x3a,0x6e,0xa5,0x80,0x80,0x80,0xff,0x00,0x00,0x00,0xff,
        0x00,0xff,0xff,0x00,0x00,0x00,0xff,0xff,0x80,0xff,0x00,0xff,0xff,0xff,0xff,0xff};

	memcpy(sPalet_00, CommonPaletA, 48);
	memcpy(sPalet_00 + 240, CommonPaletB, 48);
    
    for (uint8 i = 0; i < 255; i++)
    {
        Swap(sPalet_00[i].Blue, sPalet_00[i].Red);
    }
}

void CGGraphicDecoder::InitGraphicData()
{
	IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
	fileHandle = platFormFile.OpenRead(*fsGraphicDataPath);
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
			UE_LOG(LogTemp, Warning, TEXT("RLE_READ: %d"), RLESize);
			break;
		case RLE_REPEAT_BACKGROUND:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, BufferEncoded[BufferEncodedCursor + 1], RLESize);
            memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
            BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 2;
			UE_LOG(LogTemp, Warning, TEXT("RLE_REPEAT_BACKGROUND: %d"), RLESize);
			break;
		case RLE_REPEAT_TRANSPARENT:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, 0x00, RLESize);//transparent color
            memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
            BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferEncodedCursor += (High % 4) + 1;
			UE_LOG(LogTemp, Warning, TEXT("RLE_REPEAT_TRANSPARENT: %d"), RLESize);
			break;
		default:
            UE_LOG(LogTemp, Error, TEXT("switch(ERLEFlags) Default: %x"), BufferEncoded[BufferEncodedCursor]);
			break;
		}//switch(ERLEFlags) END
        
        ERLEFlags = RLE_NONE;
        
	}//while CurrentDecodePosition < SizeOfBuffer END
    UE_LOG(LogTemp, Warning, TEXT("Decoded Data size : %d"), BufferDecodedCursor);
    
    UE_LOG(LogTemp, Warning, TEXT("JSSRLEDecode size In Function: %d"), SizeOfBufferDecoded);
    for (uint32 Line = 0; Line < 47; Line += 1)
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *BytesToHex(&BufferDecoded[64 * Line], 64));
    }
}

void CGGraphicDecoder::PNGEncode(uint8 *Buffer, uint32 SizeOfBuffer, uint8 *PNGBuffer, uint32 &SizeOfPNGBuffer, uint32 PicWidth, uint32 PicHeight)
{
    //Line Format
    uint32 SizeOfBufferFormated = SizeOfBuffer + PicHeight;
    uint8 *BufferFormated = new uint8[SizeOfBufferFormated];
    uint32 Cursor;
    uint32 CursorFormated;
    uint32 LineFormated;
    UE_LOG(LogTemp, Warning, TEXT("BufferFormated size in Function : %d"), SizeOfBufferFormated);
    for (uint32 Line = 0; Line < PicHeight; Line += 1)
    {
        LineFormated = PicHeight - 1 - Line;
        Cursor = Line * PicWidth;
        CursorFormated = LineFormated * (PicWidth + 1);
        BufferFormated[CursorFormated] = 0x00;
        memcpy(&BufferFormated[CursorFormated + 1], &Buffer[Cursor], PicWidth);
        
        UE_LOG(LogTemp, Error, TEXT("%s"), *BytesToHex(&BufferFormated[CursorFormated], PicWidth + 1));
    }
    
    //PNGBuffer Cursor reset
    uint32 PNGBufferCursor = 0;
    
    //PNG_Header
    uint8 PNG_Title[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    memcpy(PNGBuffer, &PNG_Title, 8);
    PNGBufferCursor += 8;
    UE_LOG(LogTemp, Warning, TEXT("PNG_Title : %d"), PNGBufferCursor);
    
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
    AppendChunk(PNGBuffer, PNGBufferCursor, 768, "PLTE", sPalet_00);
    
    //Chunk_tRNS
    PaletColor Chunk_tRNS[256] = {{0x00, 0x00, 0x00}};//set transparent color
    memset(Chunk_tRNS + 1, (0xff, 0xff, 0xff), 255);
    AppendChunk(PNGBuffer, PNGBufferCursor, 256, "tRNS", Chunk_tRNS);
    
    //Chunk_IDAT
    int32 CompressedLength = FCompression::CompressMemoryBound(COMPRESS_ZLIB, SizeOfBufferFormated);
    uint8 *CompressedBuffer = new uint8[CompressedLength];
    if (FCompression::CompressMemory(COMPRESS_ZLIB, CompressedBuffer, CompressedLength, BufferFormated, SizeOfBufferFormated))
    {
        UE_LOG(LogTemp, Warning, TEXT("FCompression::CompressMemory CompressedLength: %d"), CompressedLength);
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
    
    UE_LOG(LogTemp, Warning, TEXT("PNG_%s + %d : , BufferCursor : %d"), *ChunkTypeCode, ChunkLength + 12, PNGBufferCursor);
}
