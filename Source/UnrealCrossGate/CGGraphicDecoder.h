

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
		 * not worked
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
        uint32 gId;
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

	struct Movement
	{
		uint16 aDirctionId;// 0~7
		uint16 aMovementId;
		uint32 aDuration;// unit: ms
		uint32 aFrameNum;
		uint32 * gId;
	};

	struct AnimeTable
	{
		uint32 aId;
		uint16 aNum;
		Movement * aMovement;
	};
    
    // init load
	uint8 alpha_index;
	uint8 alpha_level;// 0x00:transparent 0xff:non-transparent
	TMap<FString, PaletData> CGPalet;

    // runtime set
    GraphicMap CurMap;
	GraphicData CurGraphicData;
    FColor * ColorBuff;

public:

	// Asset
	enum AssetVer
	{
		V_0,
		V_Ex,
        V_unknown
	};

private:

	enum AssetType
	{
		T_GraphicInfo,
		T_GraphicData,
		T_AnimeInfo,
		T_AnimeData
	};

	struct TileData
	{
        AssetVer Ver;
        uint32 gId;
        uint8 bIsTile;//isTile=1 : 64*47
		uint32 TileSetId;//tileset file id
		uint32 TileSetIndex;//index in tileset
        uint32 CountTerrain;
        uint32 CountArtifact;
        uint32 bHasTileId;
        TSet<uint32> Map;
	};
    
    TMap<uint32, TileData> TileInfo;//gTileId

	struct AssetInfo
	{
		TMap<AssetType, FString> Path;
		GraphicInfo * GraphicInfo;
        uint32 GraphicNum;
		AnimeTable * AnimeTable;
        uint32 AnimeNum;
		IFileHandle * FileHandle;
	};
	
	TMap<AssetVer, AssetInfo> CGAsset;
    
public:
    
    TArray<FString> PaletTypes;
	TArray<uint32> MapList;
    
public:
    
	static FCGGraphicDecoder &Get();
    
    FCGGraphicDecoder();
    ~FCGGraphicDecoder();
    
    FCGGraphicDecoder(FCGGraphicDecoder const&) = delete;
    void operator=(FCGGraphicDecoder const&) = delete;
    
	UTexture2D * GetTexture2D(uint32 GraphicId, FString PaletType, AssetVer Ver);
	void SaveToPng(uint32 GraphicId, FString PaletType, AssetVer Ver);
	void ExportTileMapPng();
    
    void Test();
    
private:
    
    void SetResPath();

    bool IsResVerified();
    
    // load map/*.dat to MapList
	void LoadMapList();

	// set map/*.dat to CurMap
	void SetMapData(uint32 MapId);
    
    // create tilemap assets
    void CreateTileMap(uint32 MapId);
    
    // load bin/AnimeInfo*.bin Anime*.bin
	void LoadAnimeTable(AssetVer Ver);

    // load bin/Graphic*_Info.bin
    void LoadGraphicInfo(AssetVer Ver);
    
    // set TMap<uint32, TileData> TileInfo
    void SetTileInfo(AssetVer Ver);

    // load bin/pal/palet_*.cgp to TMap<FString, Palet> CGPalet
    void LoadPaletData();

    // init filehandle for bin/Graphic*.bin // todo : save filehandle to cgasset
    void InitGraphicData(AssetVer Ver);

    // set bin/Graphic*.bin to CurData
    void SetGraphicData(uint32 GraphicId, AssetVer Ver);

    // if GraphicData is compressed, decode(JSS-RLE) CurGraphicData.gData
    void DecodeGraphicData();
    
    // format GraphicData from Down-Left to Top-Left
    void FormatGraphicData();

    // set ColorBuff by PaletType
    void SetColorBuff(uint32 GraphicId, FString PaletType, AssetVer Ver);
    
};
