// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BufferArchive.h"

/**
 * 
 */
class UNREALCROSSGATE_API CGGraphicDecoder
{
public:

	static CGGraphicDecoder& GetSingletonObject();

	CGGraphicDecoder();
	~CGGraphicDecoder();

	CGGraphicDecoder(CGGraphicDecoder const&) = delete;
	void operator=(CGGraphicDecoder const&) = delete;

	uint8 * GetDecodePngData(uint32 GraphicId, uint8 PaletId);

private:

	IFileHandle *fileHandle;
	FString fsResPath, fsGraphicInfoPath, fsGraphicDataPath, fsPaletDataPath;
	uint8 *BufferData;
	uint32 BufferDataLenth;
    FBufferArchive PNGBuffer;

private:

	void SetResPath();
	bool IsResVerified();
	void LoadGraphicInfo();
	void LoadPaletData();
	void InitGraphicData();
	//GetColorFromPalet();
	uint8 *JSSRLEDecode(uint8 *Buffer, uint32 SizeOfBuffer);
    uint8 *CreatePNG(uint8 *Buffer, uint32 SizeOfBuffer, uint32 PicWidth, uint32 PicHeight);
    void AddChunkToPNGBuffer(uint32 Chunk_Length, FString Chunk_Type_Code, void *Chunk_Data);
    uint32 SwapInt32(uint32 value);
    //uint8 *ChunkToPNG(uint32 Chunk_Length_IDAT, uint8 *Chunk_Type_Code_IDAT, uint8 *Chunk_IDAT, uint32 Chunk_CRC_IDAT);

private:

	struct GraphicInfo
	{
		uint32 gId;
		uint32 gAddr;
		uint32 gLength;
		uint32 gOffsetX;
		uint32 gOffsetY;
		uint32 gWidth;
		uint32 gHeight;
		uint8 gEast;
		uint8 gSouth;
		uint8 gIsFloor;
		uint8 unknown[5];
		uint32 gMapId;
	}*sGraphicInfo;

	struct PaletColor
	{
		uint8 Blue;
		uint8 Green;
		uint8 Red;
	}sPalet_00[256], sPalet_01[256], sPalet_02[256], sPalet_03[256];

	struct GraphicData
	{
		char gRD[2];
		uint8 gIscompressed;
		uint8 unknown;
		uint32 gWidth;
		uint32 gHeight;
		uint32 gLength;
		uint8* gData;
	};
};
