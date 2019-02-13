// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldGameModeBase.h"
#include "CGGraphicDecoder.h"

AWorldGameModeBase::AWorldGameModeBase()
{
}

void AWorldGameModeBase::BeginPlay()
{
	CGGraphicDecoder& CGGraphicDecoderSingle = CGGraphicDecoderSingle.GetSingletonObject();
	CGGraphicDecoderSingle.GetDecodePngData(0, 2);
}
