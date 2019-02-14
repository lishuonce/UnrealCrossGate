// Fill out your copyright notice in the Description page of Project Settings.

#include "CGGraphicDecoder.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"

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
		GraphicData SGData;
		fileHandle->Seek(SGInfo[GraphicId].gAddr);
		fileHandle->Read((uint8 *)&SGData, 16);
        
        //DEBUG LOG - GraphicData
		UE_LOG(LogTemp, Warning, TEXT("gRD:%c%c"), SGData.gRD[0], SGData.gRD[1]);
		UE_LOG(LogTemp, Warning, TEXT("gIscompressed:%i"), SGData.gIscompressed);
		UE_LOG(LogTemp, Warning, TEXT("gWidth:%i"), SGData.gWidth);
		UE_LOG(LogTemp, Warning, TEXT("gHeight:%i"), SGData.gHeight);
		UE_LOG(LogTemp, Warning, TEXT("gLength:%i"), SGData.gLength);
		
		uint32 GDataLength = SGInfo[GraphicId].gLength - 16;
		SGData.gData = new uint8[GDataLength];
		fileHandle->Read(SGData.gData, GDataLength);

        if (SGData.gIscompressed)
        {
            //JSSRLEDecode
            uint8 *BufferDecoded = JSSRLEDecode(SGData.gData, GDataLength, SGData.gWidth, SGData.gHeight);
            delete[] SGData.gData;
            SGData.gData = BufferDecoded;
            BufferDecoded = nullptr;
            GDataLength = SGData.gWidth * SGData.gHeight;
        }
        
        //PNGEncode
        uint8 *PNGBuffer = PNGEncode(SGData.gData, GDataLength, SGData.gWidth, SGData.gHeight);
        
		return PNGBuffer;
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
}

void CGGraphicDecoder::InitGraphicData()
{
	IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
	fileHandle = platFormFile.OpenRead(*fsGraphicDataPath);
}

uint8 * CGGraphicDecoder::JSSRLEDecode(uint8 *Buffer, uint32 SizeOfBuffer, uint32 PicWidth, uint32 PicHeight)
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
    uint32 BufferCursor = 0;
    uint32 BufferDecodedLength = PicWidth * PicHeight;
    uint8 *BufferDecoded = new uint8[BufferDecodedLength];
    
    while (BufferCursor < SizeOfBuffer)
	{
		uint8 High = Buffer[BufferCursor] >> 4;
		uint8 Low = Buffer[BufferCursor] & 0x0f;
		uint32 RLESize;
		uint8 *RepeatBuffer;
        
		switch (High)
		{
		case 0x0:
			RLESize = Low;
			ERLEFlags = RLE_READ;
			break;
		case 0x1:
			RLESize = Low * 0x100 + Buffer[BufferCursor + 1];
			ERLEFlags = RLE_READ;
			break;
		case 0x2:
			RLESize = Low * 0x10000 + Buffer[BufferCursor + 1] * 0x100 + Buffer[BufferCursor + 2];
			ERLEFlags = RLE_READ;
			break;
		case 0x8:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0x9:
			RLESize = Low * 0x100 + Buffer[BufferCursor + 2];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xa:
			RLESize = Low * 0x10000 + Buffer[BufferCursor + 2] * 0x100 + Buffer[BufferCursor + 3];
			ERLEFlags = RLE_REPEAT_BACKGROUND;
			break;
		case 0xc:
			RLESize = Low;
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xd:
			RLESize = Low * 0x100 + Buffer[BufferCursor + 1];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		case 0xe:
			RLESize = Low * 0x10000 + Buffer[BufferCursor + 1] * 0x100 + Buffer[BufferCursor + 2];
			ERLEFlags = RLE_REPEAT_TRANSPARENT;
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("switch(High) Default: %x"), Buffer[BufferCursor]);
			break;
		}//while switch(High) END

		switch (ERLEFlags)
		{
		case RLE_READ:
            memcpy(&BufferDecoded[BufferDecodedCursor], &Buffer[BufferCursor + High + 1], RLESize);
            BufferDecodedCursor += RLESize;
			BufferCursor += High + 1 + RLESize;
			UE_LOG(LogTemp, Warning, TEXT("RLE_READ: %d"), RLESize);
			break;
		case RLE_REPEAT_BACKGROUND:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, Buffer[BufferCursor + 1], RLESize);
            memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
            BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferCursor += (High % 4) + 2;
			UE_LOG(LogTemp, Warning, TEXT("RLE_REPEAT_BACKGROUND: %d"), RLESize);
			break;
		case RLE_REPEAT_TRANSPARENT:
			RepeatBuffer = new uint8[RLESize];
			memset(RepeatBuffer, 0x00, RLESize);//transparent color
            memcpy(&BufferDecoded[BufferDecodedCursor], RepeatBuffer, RLESize);
            BufferDecodedCursor += RLESize;
			delete[] RepeatBuffer;
			BufferCursor += (High % 4) + 1;
			UE_LOG(LogTemp, Warning, TEXT("RLE_REPEAT_TRANSPARENT: %d"), RLESize);
			break;
		default:
            UE_LOG(LogTemp, Error, TEXT("switch(ERLEFlags) Default: %x"), Buffer[BufferCursor]);
			break;
		}//switch(ERLEFlags) END
        
        ERLEFlags = RLE_NONE;
        
	}//while CurrentDecodePosition < SizeOfBuffer END
    UE_LOG(LogTemp, Warning, TEXT("Decoded Data size : %d"), BufferDecodedLength);
    
    //PNG : Line header with 0 & row from down to up
    uint8 *BufferDecoded2 = new uint8[BufferDecodedLength + PicHeight];
    uint32 BufferDecodedCursor2;
    uint32 Line2;
    for (uint32 Line = 0; Line < PicHeight; Line += 1)
    {
        Line2 = PicHeight - 1 - Line;
        BufferDecodedCursor = Line * PicWidth;
        BufferDecodedCursor2 = Line2 * (PicWidth + 1);
        BufferDecoded2[BufferDecodedCursor2] = 0x00;
        memcpy(&BufferDecoded2[BufferDecodedCursor2 + 1], &BufferDecoded[BufferDecodedCursor], PicWidth);
    }
    delete[] BufferDecoded;
    
	return BufferDecoded2;
}

uint8 * CGGraphicDecoder::PNGEncode(uint8 *Buffer, uint32 SizeOfBuffer, uint32 PicWidth, uint32 PicHeight)
{
    //PNG init : PNGLength , PNGBuffer
    int32 CompressedLength = FCompression::CompressMemoryBound(COMPRESS_ZLIB, SizeOfBuffer);
    uint32 PNGLength = 1105 + CompressedLength;
    uint8 *PNGBuffer = new uint8[PNGLength];
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
    uint32 PicWidthBigendian = bIsLittleEndian ? htonl(PicWidth) : PicWidth;
    uint32 PicHeightBigendian = bIsLittleEndian ? htonl(PicHeight) : PicHeight;
    Chunk_IHDR sChunk_IHDR = {PicWidthBigendian, PicHeightBigendian, 8, 3, 0, 0, 0};
    AppendChunk(PNGBuffer, PNGBufferCursor, 13, "IHDR", &sChunk_IHDR);
    
    //Chunk_PLTE
    AppendChunk(PNGBuffer, PNGBufferCursor, 768, "PLTE", sPalet_00);
    
    //Chunk_tRNS
    PaletColor Chunk_tRNS[256] = {{0x00, 0x00, 0x00}};//set transparent color
    memset(Chunk_tRNS + 1, (0xff, 0xff, 0xff), 255);
    AppendChunk(PNGBuffer, PNGBufferCursor, 256, "tRNS", Chunk_tRNS);
    
    //Chunk_IDAT
    //int32 CompressedLength = FCompression::CompressMemoryBound(COMPRESS_ZLIB, SizeOfBuffer);
    uint8 *CompressedBuffer = new uint8[CompressedLength];
    FCompression::CompressMemory(COMPRESS_ZLIB, CompressedBuffer, CompressedLength, Buffer, SizeOfBuffer);
    UE_LOG(LogTemp, Warning, TEXT("FCompression::CompressMemory CompressedLength: %d"), CompressedLength);
    AppendChunk(PNGBuffer, PNGBufferCursor, CompressedLength, "IDAT", CompressedBuffer);
    delete[] CompressedBuffer;
    
    //Chunk_IEND
    AppendChunk(PNGBuffer, PNGBufferCursor, 0, "IEND", nullptr);
    
    //Save PNGBuffer to random file
    FString fsTmpPngPath = FPaths::CreateTempFilename(*fsResPath, TEXT("tmp"), TEXT(".png"));
    IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = platFormFile.OpenWrite(*fsTmpPngPath);
    if (fileHandleTmp)
    {
        fileHandleTmp->Write(PNGBuffer, PNGBufferCursor);
        delete fileHandleTmp;
    }
    
    return PNGBuffer;
}

void CGGraphicDecoder::AppendChunk(uint8 *PNGBuffer, uint32 &PNGBufferCursor, uint32 ChunkLength, FString ChunkTypeCode, void *ChunkData)
{
    //ChunkLength
    bool bIsLittleEndian = FGenericPlatformProperties::IsLittleEndian();
    uint32 ChunkLengthBigendian = bIsLittleEndian ? htonl(ChunkLength) : ChunkLength;
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
    uint32 ChunkCRCBigendian = bIsLittleEndian ? htonl(ChunkCRC) : ChunkCRC;
    memcpy(&PNGBuffer[PNGBufferCursor], &ChunkCRCBigendian, 4);
    PNGBufferCursor += 4;
    
    UE_LOG(LogTemp, Warning, TEXT("PNG_%s + %d : , BufferCursor : %d"), *ChunkTypeCode, ChunkLength + 12, PNGBufferCursor);
}
