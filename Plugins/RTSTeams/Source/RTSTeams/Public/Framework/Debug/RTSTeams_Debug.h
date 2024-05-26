﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Logging/LogMacros.h"

#if WITH_EDITOR

// Logs
DECLARE_LOG_CATEGORY_EXTERN(LogRTSTeams, Log, All);

// Print Screen
#define RTSTEAMS_PRINT_TICK(Color, Text) if (GEngine) GEngine->AddOnScreenDebugMessage(1, -1, Color, Text)
#define RTSTEAMS_PRINT_TIME(Time, Color, Text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, Time, Color, Text)

#endif