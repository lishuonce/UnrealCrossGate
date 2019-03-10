

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"

/**
 * Date: March 2, 2019
 * Author: lishuonce@gmail.com
 * Importance:
 * The class dont inherit from any UClass
 * Garbage collection manually
 * Description:
 * Full Impletment Decode CrossGate Raw Assets:
 * bin/Graphic*_Info.bin
 * bin/Graphic*.bin
 * bin/pal/palet_*.cgp
 */

class UNREALCROSSGATE_API FCGGraphicDecoder
{

private:
    
    // struct of bin/Graphic*_Info.bin
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
		// Std:gWidth=64&gHeight=47&gOffsetX=-32&gOffsetY=-24
        uint8 gIsNonStd;// 0=Std, 1=NonStd
        uint8 IsReserved[4];
        uint32 gMapId;
    };
    
    // struct of bin/Graphic*.bin
    struct GraphicData
    {
        uint8 gHeader[2];// RD : Render Data
        uint8 gIscompressed;
        uint8 unknown;
        uint32 gWidth;
        uint32 gHeight;
        uint32 gLength;
        uint8 *gData;
    };
    
    // struct of bin/pal/palet_*.cgp
    struct PaletColor
    {
        uint8 Blue;
        uint8 Green;
        uint8 Red;
    };
    struct Palet
    {
        PaletColor PaletArray[256];
    };

	// struct of map/*/*.dat
	struct MapFlag
	{
		uint8 fTransit;// 0x10=MapTransit, 0x0=NoTransit
		uint8 fPass;// 0x192=Pass, 0x193=CannotPass, 0x0=NoMap
	};
	struct GraphicMap
	{
		uint8 mHeader[3];// MAP
		uint8 mBlank[9];
		uint32 mWeight;
		uint32 mHeight;
		uint16 *mGroundLayer;
		uint16 *mCoverLayer;
		MapFlag *mFlagLayer;
	};
    
	uint8 alpha_index;
	uint8 alpha_level;// 0x00:transparent 0xff:non-transparent
	TMap<FString, Palet> CGPalet;
    GraphicInfo * CGInfo;
    GraphicData CGData;
    FColor * ColorBuff;
	GraphicMap CGMap;
    
    IFileHandle *FileHandle;
	FString fsResPath, fsGraphicInfoPath, fsGraphicDataPath;
    
public:
    
    TArray<FString> PaletTypes;
	TArray<uint32> MapList;
    
public:
    
	static FCGGraphicDecoder &Get();
    
    FCGGraphicDecoder();
    ~FCGGraphicDecoder();
    
    FCGGraphicDecoder(FCGGraphicDecoder const&) = delete;
    void operator=(FCGGraphicDecoder const&) = delete;
    
	UTexture2D * GetTexture2D(uint32 GraphicId, FString PaletType);
	void SaveToPng(uint32 GraphicId, FString PaletType);
	void SaveMapToPng(uint32 GraphicId, FString PaletType = "00");
    
private:
    
    void SetResPath();

    bool IsResVerified();
	
	// load map/*.dat to MapList
	void LoadMapList();

	// load map/*.dat to CGMap
	void LoadMapData(uint32 MapId);

    // load bin/Graphic*_Info.bin to *CGInfo
    void LoadGraphicInfo();

    // load bin/pal/palet_*.cgp to TMap<FString, Palet> CGPalet
    void LoadPaletData();

    // init filehandle for bin/Graphic*.bin
    void InitGraphicData();

    // load bin/Graphic*.bin to CGData by GraphicId
    void LoadGraphicData(uint32 GraphicId);

    // if GraphicData is compressed, decode(JSS-RLE) SGData.gData
    void DecodeGraphicData();
    
    // format GraphicData from Down-Left to Top-Left
    void FormatGraphicData();

    // set ColorBuff by PaletType
    void SetColorBuff(FString PaletType);
    
};
