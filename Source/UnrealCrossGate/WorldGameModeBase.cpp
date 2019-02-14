// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGameModeBase.h"
#include "CGGraphicDecoder.h"

AWorldGameModeBase::AWorldGameModeBase()
{
}

void AWorldGameModeBase::BeginPlay()
{
    CGGraphicDecoder &CGGraphicDecoderSingle = CGGraphicDecoder::Get();
    for (uint32 i = 0; i < 100; i++) {
        CGGraphicDecoderSingle.GetDecodePngData(i, 0);
    }
}
