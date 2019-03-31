

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "PaperTileLayer.h"

/**
 * Date: March 2, 2019
 * Author: lishuonce@gmail.com
 * Importance:
 * The class dont inherit from any UClass
 * Garbage collection manually
 * Description:
 * Full Impletment Decode CrossGate Raw Assets:
 * map/ *.dat
 * bin/AnimeInfo*.bin
 * bin/Anime*.bin
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
		/* gIsNonStd
         * 0=Std:gWidth=64&gHeight=47&gOffsetX=-32&gOffsetY=-24
         * 1=NonStd
         */
        uint8 gIsNonStd;
        uint8 IsReserved[4];
        uint32 gTileId;
    };
    
    // struct of bin/Graphic*.bin
    struct GraphicData
    {
        /* gHeader - RD : Render Data
         * [0]=R
         * [1]=D
         */
        uint8 gHeader[2];
        /* gIscompressed
         * 0=UnCompressed
         * 1=Comressed
         */
        uint8 gIscompressed;
        uint8 unknown;
        uint32 gWidth;
        uint32 gHeight;
        uint32 gLength;
        /* gData
         * bytesize = gLength -16
         */
        uint8 *gData;
    };
    
    // struct of bin/pal/palet_*.cgp
    struct PaletColor
    {
        uint8 Blue;
        uint8 Green;
        uint8 Red;
    };
    struct PaletData
    {
        PaletColor ColorData[256];
    };

	// struct of GraphicMap.mFlagLayer
    struct MapFlag
	{
        uint8 fTransit;
		uint8 fPass;
	};
    
    // struct of map/*/*.dat
	struct GraphicMap
	{
        /* mHeader
         * [0]=M
         * [1]=A
         * [2]=P
         */
        uint8 mHeader[3];
		uint8 mBlank[9];
		uint32 mWeight;
		uint32 mHeight;
        /* mTerrainLayer
         * bytesize = mWeight * mHeight *2
         * length = mWeight * mHeight
         * enum :
         * GraphicInfo.gTileId=TileId
         * 0x00=NoTile
         */
		uint16 *mTerrainLayer;
        /* mArtifactLayer
         * bytesize = mWeight * mHeight *2
         * length = mWeight * mHeight
         * enum :
         * GraphicInfo.gTileId=TileId
         * 0x00=NoTile
         * 0x02=??
         */
		uint16 *mArtifactLayer;
        /* mFlagLayer
         * bytesize = mWeight * mHeight *2
         * length = mWeight * mHeight
         * enum - MapFlag.fTransit :
         * 0x0A=MapTransit
         * 0x00=NoTransit
         * 0x02=?
         * 0x03=?
         * 0x05=?
         * 0x0B=?
         * 0x0D=?
         * enum - MapFlag.fPass
         * 0xC0=Pass
         * 0xC1=CannotPass
         * 0x00=NoMap
         */
		MapFlag *mFlagLayer;
	};
    
    // struct of bin/AnimeInfo_*.bin
    struct AnimeInfo
    {
        uint32 aId;
        uint32 aAddr;
        uint16 aNum;
        uint16 unknown;
    };
    
    // struct of bin/Anime_*.bin
    struct AnimeFrame
    {
        uint32 pId;
        uint8 unknown[6];
    };
    struct AnimeData
    {
        uint16 aDirctionId;// 0~7
        uint16 aMovementId;
        uint32 aDuration;// unit: ms
        uint32 aFrameNum;
        AnimeFrame *FrameData;// (byte)length = aFrameNum * 10
    };
    
    // init load
	uint8 alpha_index;
	uint8 alpha_level;// 0x00:transparent 0xff:non-transparent
	TMap<FString, PaletData> CGPalet;
    AnimeInfo * CGAnimeInfo;
    GraphicInfo * CGInfo;
    TMap<uint32, uint32> CGTileInfo;// gTileId, gId
	TMap<uint32, struct FPaperTileInfo> CGTileInfoStd;// TileId, TileInfo - TODO: should be save as config for runtime gameplay and update when the tile assets update

    // runtime set
    AnimeData CGAnimeData;
    GraphicData CGData;
    GraphicMap CGMap;
    FColor * ColorBuff;
    
    IFileHandle *FileHandle;
	FString fsResPath, fsGraphicInfoPath, fsGraphicDataPath, fsAnimeInfoPath, fsAnimeDataPath;
    
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
	void SaveTileToPng(uint32 GraphicId, FString PaletType = "00");
    
    void CreateTileMap(uint32 MapId);
    
    void Test();
    
private:
    
    void SetResPath();

    bool IsResVerified();
    
    // load map/*.dat to MapList
	void LoadMapList();

	// set map/*.dat to CGMap
	void SetMapData(uint32 MapId);
    
    // load bin/AnimeInfo*.bin to *CGAnimeInfo
    void LoadAnimeInfo();
    
    // set bin/Anime*.bin to CGAnimeData
    void SetAnimeData(uint32 AnimeId);

    // load bin/Graphic*_Info.bin to *CGInfo
    void LoadGraphicInfo();

    // load bin/pal/palet_*.cgp to TMap<FString, Palet> CGPalet
    void LoadPaletData();

    // init filehandle for bin/Graphic*.bin
    void InitGraphicData();

    // set bin/Graphic*.bin to CGData by GraphicId
    void SetGraphicData(uint32 GraphicId);

    // if GraphicData is compressed, decode(JSS-RLE) SGData.gData
    void DecodeGraphicData();
    
    // format GraphicData from Down-Left to Top-Left
    void FormatGraphicData();

    // set ColorBuff by PaletType
    void SetColorBuff(uint32 GraphicId, FString PaletType);
    
};
