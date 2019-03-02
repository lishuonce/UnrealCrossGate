// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"

/**
 * Date: March 2, 2019
 * Author: lishuonce@gmail.com
 * Description:
 * Full Impletment Decode CrossGate Raw Assets:
 * bin/Graphic*_Info.bin
 * bin/Graphic*.bin
 * bin/pal/palet_*.cgp
 */

class UNREALCROSSGATE_API FCGGraphicDecoder
{

private:
    
    //struct of bin/Graphic*_Info.bin
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
        uint8 gIsNonStd;//Std:gWidth=64&gHeight=47&gOffsetX=-32&gOffsetY=-24
        uint8 IsReserved[4];
        uint32 gMapId;
    };
    
    //struct of bin/Graphic*.bin
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
    
    //struct of bin/pal/palet_*.cgp
    struct PaletColor
    {
        uint8 Blue;
        uint8 Green;
        uint8 Red;
    };
    struct Palet
    {
        PaletColor sPalet[256];
    };
    
    TMap<FString, Palet> PaletMap;
    GraphicInfo * SGInfo;
    GraphicData SGData;
    TArray<FColor> ColorBuff;
    
    IFileHandle *fileHandle;
    FString fsResPath, fsGraphicInfoPath, fsGraphicDataPath, fsPaletDataPath;
    
public:
    
    TArray<FString> PaletTypes;
    
public:
    
    static FCGGraphicDecoder &Get();
    
    FCGGraphicDecoder();
    ~FCGGraphicDecoder();
    
    FCGGraphicDecoder(FCGGraphicDecoder const&) = delete;
    void operator=(FCGGraphicDecoder const&) = delete;
    
    uint8 * GetDecodePngData(uint32 GraphicId, FString PaletType);
    TArray<FColor> * GetColorBuff(uint32 GraphicId, FString PaletType);
    
private:
    
    void SetResPath();
    bool IsResVerified();
    //load bin/Graphic*_Info.bin to *SGInfo
    void LoadGraphicInfo();
    //load bin/pal/palet_*.cgp to TMap<FString, Palet> PaletMap
    void LoadPaletData();
    //init filehandle for bin/Graphic*.bin
    void InitGraphicData();
    //set SGData by GraphicId
    void SetGraphicData(uint32 GraphicId);
    //if GraphicData is compressed, decode(JSS-RLE) SGData.gData
    void DecodeGraphicData();
    //set ColorBuff by PaletType
    void SetColorBuff(FString PaletType);
    //todo : reimplement by DecodeGraphicData()
    void JSSRLEDecode(uint8 *BufferEncoded, uint32 SizeOfBufferEncoded, uint8 *BufferDecoded, uint32 SizeOfBufferDecoded);
    
private:
    
    void PNGEncode(uint8 *Buffer, uint32 SizeOfBuffer, uint8 *PNGBuffer, uint32 &SizeOfPNGBuffer, uint32 PicWidth, uint32 PicHeight, FString PaletType);
    void AppendChunk(uint8 *PNGBuffer, uint32 &PNGBufferCursor, uint32 ChunkLength, FString ChunkTypeCode, void *ChunkData);
    
};
