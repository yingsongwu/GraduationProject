// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "GetMaterialAndObjCommands.h"

#define LOCTEXT_NAMESPACE "FGetMaterialAndObjModule"

void FGetMaterialAndObjCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "GetMaterialAndObj", "Bring up GetMaterialAndObj window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
