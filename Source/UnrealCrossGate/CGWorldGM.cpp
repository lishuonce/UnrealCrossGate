// Fill out your copyright notice in the Description page of Project Settings.

#include "CGWorldGM.h"
#include "CGGraphicDecoder.h"

DEFINE_LOG_CATEGORY_CLASS(ACGWorldGM,  CGWorldGM)

ACGWorldGM::ACGWorldGM()
{
}

void ACGWorldGM::BeginPlay()
{
    CGGraphicDecoder &CGGraphicDecoderSingle = CGGraphicDecoder::Get();
//    for (uint32 i = 0; i < 100; i++) {
//        CGGraphicDecoderSingle.GetDecodePngData(i, 0);
//    }
    TArray<FString> PaletMapKey = CGGraphicDecoderSingle.GetPaletTypeList();
    for (auto Key = PaletMapKey.CreateConstIterator(); Key; ++Key)
    {
        CGGraphicDecoderSingle.GetDecodePngData(19713, *Key);
    }
//    CGGraphicDecoderSingle.test();
}
