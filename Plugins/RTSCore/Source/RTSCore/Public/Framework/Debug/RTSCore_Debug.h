// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Logging/LogMacros.h"



// Logs
DECLARE_LOG_CATEGORY_EXTERN(LogRTSCore, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRTSMultiMenu, Log, All);

#if WITH_EDITOR

// Print Screen
#define RTSCORE_PRINT_TICK(Color, Text) if (GEngine) GEngine->AddOnScreenDebugMessage(1, -1, Color, Text)
#define RTSCORE_PRINT_TIME(Time, Color, Text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, Time, Color, Text)

#endif