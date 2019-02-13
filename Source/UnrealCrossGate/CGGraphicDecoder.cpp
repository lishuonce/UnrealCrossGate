// Fill out your copyright notice in the Description page of Project Settings.

#include "CGGraphicDecoder.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"

CGGraphicDecoder & CGGraphicDecoder::GetSingletonObject()
{
	static CGGraphicDecoder CGGraphDecoderSingletObj;
	return CGGraphDecoderSingletObj;
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
	UE_LOG(LogTemp, Warning, TEXT("gId:%i"), sGraphicInfo[GraphicId].gId);
	UE_LOG(LogTemp, Warning, TEXT("gAddr:%i"), sGraphicInfo[GraphicId].gAddr);
	UE_LOG(LogTemp, Warning, TEXT("gLength:%i"), sGraphicInfo[GraphicId].gLength);
	UE_LOG(LogTemp, Warning, TEXT("gOffsetX:%i"), sGraphicInfo[GraphicId].gOffsetX);
	UE_LOG(LogTemp, Warning, TEXT("gOffsetY:%i"), sGraphicInfo[GraphicId].gOffsetY);
	UE_LOG(LogTemp, Warning, TEXT("gWidth:%i"), sGraphicInfo[GraphicId].gWidth);
	UE_LOG(LogTemp, Warning, TEXT("gHeight:%i"), sGraphicInfo[GraphicId].gHeight);
	UE_LOG(LogTemp, Warning, TEXT("gEast:%i"), sGraphicInfo[GraphicId].gEast);
	UE_LOG(LogTemp, Warning, TEXT("gSouth:%i"), sGraphicInfo[GraphicId].gSouth);
	UE_LOG(LogTemp, Warning, TEXT("gIsFloor:%i"), sGraphicInfo[GraphicId].gIsFloor);
	UE_LOG(LogTemp, Warning, TEXT("gMapId:%i"), sGraphicInfo[GraphicId].gMapId);
	
	

	if (fileHandle)
	{
		GraphicData sGData;
		fileHandle->Seek(sGraphicInfo[GraphicId].gAddr);
		fileHandle->Read((uint8 *)&sGData, 16);
		UE_LOG(LogTemp, Warning, TEXT("gRD:%c%c"), sGData.gRD[0], sGData.gRD[1]);
		UE_LOG(LogTemp, Warning, TEXT("gIscompressed:%i"), sGData.gIscompressed);
		UE_LOG(LogTemp, Warning, TEXT("gWidth:%i"), sGData.gWidth);
		UE_LOG(LogTemp, Warning, TEXT("gHeight:%i"), sGData.gHeight);
		UE_LOG(LogTemp, Warning, TEXT("gLength:%i"), sGData.gLength);

		if (!BufferData)
		{
			delete[] BufferData;
		}
		
		BufferDataLenth = sGraphicInfo[GraphicId].gLength - 16;
		BufferData = new uint8[BufferDataLenth];
		fileHandle->Read(BufferData, BufferDataLenth);

		UE_LOG(LogTemp, Warning, TEXT("*gData: %x %x %x"), BufferData[0], BufferData[1], BufferData[2]);

        if (sGData.gIscompressed)
        {
            BufferData = JSSRLEDecode(BufferData, BufferDataLenth);
            BufferDataLenth = sGData.gWidth * sGData.gHeight;
        }
        
		return CreatePNG(BufferData, BufferDataLenth, sGraphicInfo[GraphicId].gWidth, sGraphicInfo[GraphicId].gHeight);
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
		sGraphicInfo = new GraphicInfo[iRecordNum];
		fileHandleTmp->Read((uint8 *)sGraphicInfo, iFileSize);
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

uint8 * CGGraphicDecoder::JSSRLEDecode(uint8 *Buffer, uint32 SizeOfBuffer)
{
	uint32 CurrentDecodePosition = 0;
	TArray<uint8> BufferDecoded;
    enum
    {
        none,
        read,
        repeat_background,
        repeat_transparent
    }rLengthAction = none;

	while (CurrentDecodePosition < SizeOfBuffer)
	{
		uint8 High = Buffer[CurrentDecodePosition] >> 4;
		uint8 Low = Buffer[CurrentDecodePosition] & 0x0f;
		uint32 NextSize;
		uint8 *RepeatArr;
        
		switch (High)
		{
		case 0x0:
			NextSize = Low;
			rLengthAction = read;
			break;
		case 0x1:
			NextSize = Low * 0x100 + Buffer[CurrentDecodePosition + 1];
			rLengthAction = read;
			break;
		case 0x2:
			NextSize = Low * 0x10000 + Buffer[CurrentDecodePosition + 1] * 0x100 + Buffer[CurrentDecodePosition + 2];
			rLengthAction = read;
			break;
		case 0x8:
			NextSize = Low;
			rLengthAction = repeat_background;
			break;
		case 0x9:
			NextSize = Low * 0x100 + Buffer[CurrentDecodePosition + 2];
			rLengthAction = repeat_background;
			break;
		case 0xa:
			NextSize = Low * 0x10000 + Buffer[CurrentDecodePosition + 2] * 0x100 + Buffer[CurrentDecodePosition + 3];
			rLengthAction = repeat_background;
			break;
		case 0xc:
			NextSize = Low;
			rLengthAction = repeat_transparent;
			break;
		case 0xd:
			NextSize = Low * 0x100 + Buffer[CurrentDecodePosition + 1];
			rLengthAction = repeat_transparent;
			break;
		case 0xe:
			NextSize = Low * 0x10000 + Buffer[CurrentDecodePosition + 1] * 0x100 + Buffer[CurrentDecodePosition + 2];
			rLengthAction = repeat_transparent;
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("Default: %x"), Buffer[CurrentDecodePosition]);
			break;
		}

		switch (rLengthAction)
		{
		case read:
			BufferDecoded.Append(&Buffer[CurrentDecodePosition + High + 1], NextSize);
			CurrentDecodePosition += High + 1 + NextSize;
			UE_LOG(LogTemp, Warning, TEXT("Read: %x"), Buffer[CurrentDecodePosition]);
			break;
		case repeat_background:
			RepeatArr = new uint8[NextSize];
			memset(RepeatArr, Buffer[CurrentDecodePosition + 1], NextSize);
            BufferDecoded.Append(RepeatArr, NextSize);
			delete[] RepeatArr;
			CurrentDecodePosition += (High % 4) + 2;
			UE_LOG(LogTemp, Warning, TEXT("repeat_background: %x"), Buffer[CurrentDecodePosition]);
			break;
		case repeat_transparent:
			RepeatArr = new uint8[NextSize];
			//transparent color
			memset(RepeatArr, 0x00, NextSize);
            BufferDecoded.Append(RepeatArr, NextSize);
			delete[] RepeatArr;
			CurrentDecodePosition += (High % 4) + 1;
			UE_LOG(LogTemp, Warning, TEXT("repeat_transparent: %x"), Buffer[CurrentDecodePosition]);
			break;
		default:
			break;
		}

		//CurrentDecodePosition = SizeOfBuffer;
	}
    UE_LOG(LogTemp, Warning, TEXT("Decoded Data size : %d"), BufferDecoded.Num());
    
	return BufferDecoded.GetData();
}

uint8 * CGGraphicDecoder::CreatePNG(uint8 *Buffer, uint32 SizeOfBuffer, uint32 PicWidth, uint32 PicHeight)
{
    PNGBuffer.FlushCache();
    PNGBuffer.Empty();
    
    //PNG_Title
    uint8 PNG_Title[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    PNGBuffer.Serialize(PNG_Title, 8);
    UE_LOG(LogTemp, Warning, TEXT("PNG_Title : %d"), PNGBuffer.Num());
    
    //PNG_IHDR
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
    Chunk_IHDR sChunk_IHDR = {SwapInt32(PicWidth), SwapInt32(PicHeight), 8, 3, 0, 0, 0};
    AddChunkToPNGBuffer(13, "IHDR", &sChunk_IHDR);
    
    //PNG_PLTE
    AddChunkToPNGBuffer(768, "PLTE", sPalet_00);
    
    //PNG_tRNS
    PaletColor Chunk_tRNS[256] = {{0x00, 0x00, 0x00}};//set transparent color
    memset(Chunk_tRNS + 1, (0xff, 0xff, 0xff), 255);
    AddChunkToPNGBuffer(256, "tRNS", Chunk_tRNS);
    
    //PNG_IDAT
    FBufferArchive ToBinary;
    ToBinary.Serialize(Buffer, SizeOfBuffer);
    TArray<uint8> CompressedData;
    FArchiveSaveCompressedProxy Compressor =
    FArchiveSaveCompressedProxy(CompressedData, ECompressionFlags::COMPRESS_ZLIB);
    Compressor << ToBinary;
    Compressor.Flush();
    Compressor.FlushCache();
    ToBinary.FlushCache();
    ToBinary.Empty();
    ToBinary.Close();
    uint8 *Chunk_IDAT = CompressedData.GetData();
    UE_LOG(LogTemp, Warning, TEXT("CompressedData.Num() : %d"), CompressedData.Num());
    AddChunkToPNGBuffer(CompressedData.Num(), "IDAT", CompressedData.GetData());
    CompressedData.Empty();
    
    //PNG_IEND
    AddChunkToPNGBuffer(0, "IEND", nullptr);
    
    FString fsTmpPngPath = FPaths::CreateTempFilename(*fsResPath, TEXT("tmp"), TEXT(".png"));
    IPlatformFile &platFormFile = FPlatformFileManager::Get().GetPlatformFile();
    IFileHandle *fileHandleTmp = platFormFile.OpenWrite(*fsTmpPngPath);
    if (fileHandleTmp)
    {
        fileHandleTmp->Write(PNGBuffer.GetData(), PNGBuffer.Num());
        delete fileHandleTmp;
    }
    
    return PNGBuffer.GetData();
}

void CGGraphicDecoder::AddChunkToPNGBuffer(uint32 Chunk_Length, FString Chunk_Type_Code, void *Chunk_Data)
{
    
    uint32 Chunk_Length_Swaped = SwapInt32(Chunk_Length);
    PNGBuffer << Chunk_Length_Swaped;
    PNGBuffer.Serialize((uint8 *)TCHAR_TO_ANSI(*Chunk_Type_Code), 4);
    if (Chunk_Length > 0)
    {
        PNGBuffer.Serialize(Chunk_Data, Chunk_Length);
    }
    uint32 crcIndex = PNGBuffer.Num() - Chunk_Length - 4;
    uint32 Chunk_CRC = FCrc::MemCrc32(&PNGBuffer[crcIndex], Chunk_Length + 4);
    uint32 Chunk_CRC_Swaped = SwapInt32(Chunk_CRC);
    PNGBuffer << Chunk_CRC_Swaped;
    UE_LOG(LogTemp, Warning, TEXT("PNG_%s + %d : %d"), *Chunk_Type_Code, Chunk_Length + 12, PNGBuffer.Num());
}

uint32 CGGraphicDecoder::SwapInt32(uint32 value)
{
    return ((value & 0x000000FF) << 24) |
    ((value & 0x0000FF00) << 8) |
    ((value & 0x00FF0000) >> 8) |
    ((value & 0xFF000000) >> 24) ;
}
