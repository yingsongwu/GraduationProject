// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "GetMaterialAndObjStyle.h"

class FGetMaterialAndObjCommands : public TCommands<FGetMaterialAndObjCommands>
{
public:

	FGetMaterialAndObjCommands()
		: TCommands<FGetMaterialAndObjCommands>(TEXT("GetMaterialAndObj"), NSLOCTEXT("Contexts", "GetMaterialAndObj", "GetMaterialAndObj Plugin"), NAME_None, FGetMaterialAndObjStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};