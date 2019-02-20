// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Paths.h"
#include "PlatformFilemanager.h"
#include "GenericPlatformFile.h"

/**
 * 
 */
class UNREALCROSSGATE_API CGGraphicDecoder
{
public:

	static CGGraphicDecoder &Get();

	CGGraphicDecoder();
	~CGGraphicDecoder();

	CGGraphicDecoder(CGGraphicDecoder const&) = delete;
	void operator=(CGGraphicDecoder const&) = delete;

	uint8 * GetDecodePngData(uint32 GraphicId, FString PaletType);
    void test();
    TArray<FString> GetPaletTypeList();

private:

	IFileHandle *fileHandle;
	FString fsResPath, fsGraphicInfoPath, fsGraphicDataPath, fsPaletDataPath;

private:

	void SetResPath();
	bool IsResVerified();
	void LoadGraphicInfo();
	void LoadPaletData();
	void InitGraphicData();
	void JSSRLEDecode(uint8 *BufferEncoded, uint32 SizeOfBufferEncoded, uint8 *BufferDecoded, uint32 SizeOfBufferDecoded);

private:
    void PNGEncode(uint8 *Buffer, uint32 SizeOfBuffer, uint8 *PNGBuffer, uint32 &SizeOfPNGBuffer, uint32 PicWidth, uint32 PicHeight, FString PaletType);
    void AppendChunk(uint8 *PNGBuffer, uint32 &PNGBufferCursor, uint32 ChunkLength, FString ChunkTypeCode, void *ChunkData);

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
        uint8 gIsSpecial;//!(gWidth=64&gHeight=47&gOffsetX=-32&gOffsetY=-24)
		uint8 IsReserved[4];
		uint32 gMapId;
	}*SGInfo;

	struct PaletColor
	{
		uint8 Blue;
		uint8 Green;
		uint8 Red;
	}sPalet_00[256];
    
    struct Palet {
        PaletColor sPalet[256];
    };
    
    TMap<FString, Palet> PaletMap;
    
	struct GraphicData
	{
        uint8 gHeader[2];//RD : Render Data
		uint8 gIscompressed;
		uint8 unknown;
		uint32 gWidth;
		uint32 gHeight;
		uint32 gLength;
		uint8 *gData;
	};
};
