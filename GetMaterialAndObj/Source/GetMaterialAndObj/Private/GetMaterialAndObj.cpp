// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "GetMaterialAndObj.h"
#include "GetMaterialAndObjStyle.h"
#include "GetMaterialAndObjCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Widgets/Input/SButton.h"

#include "FileHelper.h"
#include "Developer/MaterialUtilities/Public/MaterialUtilities.h"

#include "GetMultipleMaterial.h"
#include "UnrealEd.h"
//#include "Developer/ShaderPreprocessor/Private/PreprocessorPrivate.h"

static const FName GetMaterialAndObjTabName("GetMaterialAndObj");

#define LOCTEXT_NAMESPACE "FGetMaterialAndObjModule"

void FGetMaterialAndObjModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FGetMaterialAndObjStyle::Initialize();
	FGetMaterialAndObjStyle::ReloadTextures();

	FGetMaterialAndObjCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FGetMaterialAndObjCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FGetMaterialAndObjModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FGetMaterialAndObjModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FGetMaterialAndObjModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GetMaterialAndObjTabName, FOnSpawnTab::CreateRaw(this, &FGetMaterialAndObjModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FGetMaterialAndObjTabTitle", "GetMaterialAndObj"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FGetMaterialAndObjModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGetMaterialAndObjStyle::Shutdown();

	FGetMaterialAndObjCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GetMaterialAndObjTabName);
}


inline FString FixupMaterialName(UMaterialInterface* Material)
{
	return Material->GetName().Replace(TEXT("."), TEXT("_")).Replace(TEXT(":"), TEXT("_"));
}


static void ExportMaterialPropertyTexture(const FString &BMPFilename, UMaterialInterface* Material, const EMaterialProperty MatProp)
{
	check(Material);

	// make the BMP for the diffuse channel
	TArray<FColor> OutputBMP;
	TArray<FColor> OutputBMP_M, OutputBMP_S, OutputBMP_R;
	FIntPoint OutSize;

	TEnumAsByte<EBlendMode> BlendMode = Material->GetBlendMode();

	bool bIsValidMaterial;
	if (MatProp == MP_Metallic || MatProp == MP_Specular || MatProp == MP_Roughness)
	{
		bIsValidMaterial = FMaterialUtilities::SupportsExport((EBlendMode)(BlendMode), MP_Metallic) && FMaterialUtilities::SupportsExport((EBlendMode)(BlendMode), MP_Specular)
			&& FMaterialUtilities::SupportsExport((EBlendMode)(BlendMode), MP_Roughness);

		if (bIsValidMaterial)
		{

			// render the material to a texture to export as a bmp
			if (!FMaterialUtilities::ExportMaterialProperty(Material, MP_Metallic, OutputBMP_M, OutSize))
			{
				bIsValidMaterial = false;
			}
			if (!FMaterialUtilities::ExportMaterialProperty(Material, MP_Specular, OutputBMP_S, OutSize))
			{
				bIsValidMaterial = false;
			}
			if (!FMaterialUtilities::ExportMaterialProperty(Material, MP_Roughness, OutputBMP_R, OutSize))
			{
				bIsValidMaterial = false;
			}
			//OutputBMP.// = OutputBMP_M + OutputBMP_S + OutputBMP_R;
			OutputBMP = OutputBMP_M;
			for (auto i = 0; i < OutputBMP_M.Num(); i++)
			{
				OutputBMP[i].R = OutputBMP_M[i].R;
				OutputBMP[i].G = OutputBMP_R[i].G;
				OutputBMP[i].B = OutputBMP_S[i].B;
				OutputBMP[i].A = 0;
			}
		}
	}
	else {
		bIsValidMaterial = FMaterialUtilities::SupportsExport((EBlendMode)(BlendMode), MatProp);

		if (bIsValidMaterial)
		{
			// render the material to a texture to export as a bmp
			if (!FMaterialUtilities::ExportMaterialProperty(Material, MatProp, OutputBMP, OutSize))
			{
				bIsValidMaterial = false;
			}

		}
	}

	// make invalid textures a solid red
	if (!bIsValidMaterial)
	{
		OutSize = FIntPoint(1, 1);
		OutputBMP.Empty();
		OutputBMP.Add(FColor(255, 0, 0, 255));
	}

	// export the diffuse channel bmp
	FFileHelper::CreateBitmap(*BMPFilename, OutSize.X, OutSize.Y, OutputBMP.GetData());
}

/*
void FGetMaterialAndObjModule::GetSelectedActors()
{
	USelection* SelectedActors = GEditor->GetSelectedActors();
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		AActor* Actor = Cast<AActor>(*Iter);
		if (Actor)
		{
			Actors.Add(Actor);
			//Components.Add(Actor->GetStaticMeshComponent());
			//UniqueLevels.AddUnique(Actor->GetLevel());
		}
	}
	{
		// Retrieve static mesh components from selected actors
		//SelectedComponents.Empty();
		for (int32 ActorIndex = 0; ActorIndex < Actors.Num(); ++ActorIndex)
		{
			AActor* Actor = Actors[ActorIndex];
			check(Actor != nullptr);

			TArray<UChildActorComponent*> ChildActorComponents;
			Actor->GetComponents<UChildActorComponent>(ChildActorComponents);
			for (UChildActorComponent* ChildComponent : ChildActorComponents)
			{
				// Push actor at the back of array so we will process it
				AActor* ChildActor = ChildComponent->GetChildActor();
				if (ChildActor)
				{
					Actors.Add(ChildActor);
				}
			}

			TArray<UPrimitiveComponent*> PrimComponents;
			Actor->GetComponents<UPrimitiveComponent>(PrimComponents);
			for (UPrimitiveComponent* PrimComponent : PrimComponents)
			{
				UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(PrimComponent);
				if (MeshComponent &&
					MeshComponent->GetStaticMesh() != nullptr &&
					MeshComponent->GetStaticMesh()->SourceModels.Num() > 0)
				{
					Components.Add(MeshComponent);
				}

				UShapeComponent* ShapeComponent = Cast<UShapeComponent>(PrimComponent);
				if (ShapeComponent)
				{
					Components.Add(ShapeComponent);
				}
			}
		}
	}
}
*/
TSharedPtr<SSpinBox<int32>> TextureSizeX;
TSharedPtr<SSpinBox<int32>> TextureSizeY;

TArray<UObject*> ExportedAssets;

bool ExportMultipleObj(UObject* Object, FString ProjectPathName)
{
	//Componet
	UStaticMesh* StaticMesh = CastChecked<UStaticMesh>(Object);
	TArray<UObject*> AssetsToSync;

	//Filename为带obj的绝对路径名字
	for (int32 LODindex = 0; LODindex < StaticMesh->GetNumLODs(); LODindex++)
	{
		//绝对路径，path+name_LODN.obj
		FString Filename = ProjectPathName + FPackageName::GetShortName(StaticMesh->GetOutermost()->GetName())+ TEXT("_") + FPackageName::GetShortName(StaticMesh->GetOutermost()->GetName()) + FString("_LOD" + FString::FromInt(LODindex)) + TEXT(".obj");
		Filename = Filename.Replace(TEXT("_LOD0"),TEXT(""));
		FArchive* File = IFileManager::Get().CreateFileWriter(*Filename);
		
		FString MaterialLibFilename = FPaths::GetBaseFilename(Filename, false) + TEXT(".mtl");

		TArray<FStaticMaterial> StaticMaterials = StaticMesh->StaticMaterials;

		FOutputDeviceFile* MaterialLib = new FOutputDeviceFile(*MaterialLibFilename, true);
		MaterialLib->SetSuppressEventTag(true);
		MaterialLib->SetAutoEmitLineTerminator(false);

		for (int32 Materialindex = 0; Materialindex < StaticMaterials.Num(); Materialindex++)
		{
			//FString MaterialName = TEXT("../GetMaterial/");//FPackageName::GetShortName(StaticMesh->GetOutermost()->GetName()) + TEXT("_") + FPackageName::GetShortName(StaticMesh->GetOutermost()->GetName());
			//Materialname_LODN
			FString MaterialName = TEXT("maps/")+  StaticMaterials[Materialindex].MaterialSlotName.ToString()+TEXT("_")+ StaticMaterials[Materialindex].MaterialSlotName.ToString() + FString("_LOD" + FString::FromInt(LODindex));
			MaterialName = MaterialName.Replace(TEXT("_LOD0"), TEXT(""));
			
			MaterialLib->Logf(TEXT("newmtl %s\r\n"), *MaterialName);
		
			//Materialname_LODN_DNS.bmp
			FString BMPFilename = MaterialName + TEXT("_D.bmp");
			MaterialLib->Logf(TEXT("\tmap_Kd %s\r\n"), *BMPFilename); 
			BMPFilename = MaterialName + TEXT("_S.bmp");
			MaterialLib->Logf(TEXT("\tmap_Ks %s\r\n"), *BMPFilename);
			BMPFilename = MaterialName + TEXT("_N.bmp");
			MaterialLib->Logf(TEXT("\tbump %s\r\n"), *BMPFilename);

			MaterialLib->Logf(TEXT("\r\n\n"));
		}
		MaterialLib->TearDown();
		delete MaterialLib;
		File->Logf(TEXT("# UnrealEd OBJ exporter\r\n"));

		// 这里只提供了LOD0的obj
		const FStaticMeshLODResources& RenderData = StaticMesh->GetLODForExport(LODindex);
		uint32 VertexCount = RenderData.GetNumVertices();

		check(VertexCount == RenderData.VertexBuffer.GetNumVertices());

		File->Logf(TEXT("\r\n"));
		File->Logf(TEXT("mtllib %s\n"), *FPaths::GetCleanFilename(MaterialLibFilename));
		for (uint32 i = 0; i < VertexCount; i++)
		{
			const FVector& OSPos = RenderData.PositionVertexBuffer.VertexPosition(i);
			//			const FVector WSPos = StaticMeshComponent->LocalToWorld.TransformPosition( OSPos );
			const FVector WSPos = OSPos;

			// Transform to Lightwave's coordinate system
			File->Logf(TEXT("v %f %f %f\r\n"), WSPos.X, WSPos.Z, WSPos.Y);
		}

		File->Logf(TEXT("\r\n"));
		
		for (uint32 i = 0; i < VertexCount; ++i)
		{
			// takes the first UV
			const FVector2D UV = RenderData.VertexBuffer.GetVertexUV(i, 0);

			// Invert the y-coordinate (Lightwave has their bitmaps upside-down from us).
			File->Logf(TEXT("vt %f %f\r\n"), UV.X, 1.0f - UV.Y);
		}

		File->Logf(TEXT("\r\n"));

		for (uint32 i = 0; i < VertexCount; ++i)
		{
			const FVector& OSNormal = RenderData.VertexBuffer.VertexTangentZ(i);
			const FVector WSNormal = OSNormal;

			// Transform to Lightwave's coordinate system
			File->Logf(TEXT("vn %f %f %f\r\n"), WSNormal.X, WSNormal.Z, WSNormal.Y);
		}

		//File->Logf(TEXT("usemtl %s\n"), *FPaths::GetCleanFilename(MaterialName));
		//for(int32 n = 0;i<RenderData)
		
		TArray<uint32> MaterialIndexArray;
		for (auto temp: RenderData.Sections)
		{
			MaterialIndexArray.Add(temp.FirstIndex);
		}

		{
			FIndexArrayView Indices = RenderData.IndexBuffer.GetArrayView();
			uint32 NumIndices = Indices.Num();

			check(NumIndices % 3 == 0); int count = 0;
			for (uint32 i = 0; i < NumIndices/3 ; i++)
			{
				if(count < MaterialIndexArray.Num() && i == MaterialIndexArray[count]/3)
				{
					FString mtl = StaticMaterials[count].MaterialSlotName.ToString()+TEXT("_")+StaticMaterials[count].MaterialSlotName.ToString();
					File->Logf(TEXT("usemtl %s\n"), *FPaths::GetCleanFilename(mtl));//*FPaths::GetCleanFilename(MaterialName));
					count++;
				}

				// Wavefront indices are 1 based
				uint32 a = Indices[3*i] + 1;
				uint32 b = Indices[3 * i+1] + 1;
				uint32 c = Indices[3 * i+2] + 1;

				File->Logf(TEXT("f %d/%d/%d %d/%d/%d %d/%d/%d\r\n"),
					a, a, a,
					b, b, b,
					c, c, c);
			}
		}

		delete File;
	}
	return true;
}

FReply FGetMaterialAndObjModule::GetObjMethod()
{
	//FFeedbackContext* Warn;
	GWarn->BeginSlowTask(NSLOCTEXT("UnrealEd", "ExportingOBJ", "Exporting OBJ And Mtl"), true);

	//const FText LocalizedExportingMap = FText::Format(NSLOCTEXT("UnrealEd", "ExportingMap_F", "Exporting map: {0}..."), FText::FromString(MapFileName));
	//GWarn->BeginSlowTask(LocalizedExportingMap, true);

	TArray<UObject*> Objects;
	TArray<FAssetData> AssetDataList;
	{
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* Actor = static_cast<AActor*>(*It);
			checkSlow(Actor->IsA(AActor::StaticClass()));
			
			// If the actor is an instance of a blueprint, just add the blueprint.
			UBlueprint* GeneratingBP = Cast<UBlueprint>(It->GetClass()->ClassGeneratedBy);
			if (GeneratingBP != NULL)
			{
				Objects.Add(GeneratingBP);
			}
			// Otherwise, add the results of the GetReferencedContentObjects call
			else
			{
				{
					TInlineComponentArray<UStaticMeshComponent*> StaticMeshComponents;
					Actor->GetComponents(StaticMeshComponents);

					if(StaticMeshComponents.Num() != 0)
					{
						Actor->GetReferencedContentObjects(Objects);
					}
				}
			}
		}
		for (int32 ActorIndex = 0; ActorIndex < Objects.Num(); ++ActorIndex)
		{
			AssetDataList.Add(FAssetData(Objects[ActorIndex]));
		}
	}

	{
		//放入要导出的
		for (int32 Index = 0; Index < AssetDataList.Num(); Index++)
		{
			{
				//差一个能否导出判断
				GWarn->StatusUpdate(Index, AssetDataList.Num(), NSLOCTEXT("UnrealEd", "ExportingOBJ", "Exporting OBJ And Mtl"));

				UObject* ObjectToExport = AssetDataList[Index].GetAsset();
				//auto a = ObjectToExport->StaticClass();
				
				FString ProjectPathName = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + TEXT("GetObj/");
				ExportMultipleObj(ObjectToExport, ProjectPathName);		
			}
		}
	}
	GWarn->EndSlowTask();
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("GetObjAndMtl DONE")));

	return FReply::Handled();
}

FReply FGetMaterialAndObjModule::GetMaterialMethod()
{

	//GetSelectedActors();
	TArray<AActor*> Actors;
	TArray<UPrimitiveComponent*> Components;
	GWarn->BeginSlowTask(NSLOCTEXT("UnrealEd", "ExportingOBJ", "Exporting Material"), true);
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();
		for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
		{
			AActor* Actor = Cast<AActor>(*Iter);
			if (Actor)
			{
				Actors.Add(Actor);
				//Components.Add(Actor->GetStaticMeshComponent());
				//UniqueLevels.AddUnique(Actor->GetLevel());
			}
		}
		{
			// Retrieve static mesh components from selected actors
			//SelectedComponents.Empty();
			for (int32 ActorIndex = 0; ActorIndex < Actors.Num(); ++ActorIndex)
			{
				AActor* Actor = Actors[ActorIndex];
				check(Actor != nullptr);

				TArray<UChildActorComponent*> ChildActorComponents;
				Actor->GetComponents<UChildActorComponent>(ChildActorComponents);
				for (UChildActorComponent* ChildComponent : ChildActorComponents)
				{
					// Push actor at the back of array so we will process it
					AActor* ChildActor = ChildComponent->GetChildActor();
					if (ChildActor)
					{
						Actors.Add(ChildActor);
					}
				}

				TArray<UPrimitiveComponent*> PrimComponents;
				Actor->GetComponents<UPrimitiveComponent>(PrimComponents);
				for (UPrimitiveComponent* PrimComponent : PrimComponents)
				{
					UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(PrimComponent);
					if (MeshComponent &&
						MeshComponent->GetStaticMesh() != nullptr &&
						MeshComponent->GetStaticMesh()->SourceModels.Num() > 0)
					{
						Components.Add(MeshComponent);
					}

					UShapeComponent* ShapeComponent = Cast<UShapeComponent>(PrimComponent);
					if (ShapeComponent)
					{
						Components.Add(ShapeComponent);
					}
				}
			}
		}
	}
	//for (auto Component : Components)
	for (int32 Index = 0; Index < Components.Num(); Index++)
	{
		//缺一个判断
		GWarn->StatusUpdate(Index, Components.Num(), NSLOCTEXT("UnrealEd", "ExportingOBJ", "Exporting Material"));

		UPrimitiveComponent* Component = Components[Index];
		FMeshMergingSettings settings;
		settings.LODSelectionType = EMeshLODSelectionType::SpecificLOD;
		settings.bMergeMaterials = true;
		settings.bMergePhysicsData = false;
		settings.MaterialSettings.bMetallicMap = true;
		settings.MaterialSettings.bSpecularMap = true;
		settings.MaterialSettings.bRoughnessMap = true;
		settings.MaterialSettings.bOpacityMap = true;
		settings.MaterialSettings.bOpacityMaskMap = true;
		settings.MaterialSettings.BlendMode = BLEND_Masked;
		settings.MaterialSettings.TextureSize = FIntPoint(TextureSizeX->GetValue(), TextureSizeY->GetValue());

		FString ProjectPath = "/Game/GetObj/maps/";
		FString CompName = FPackageName::GetShortName(Cast<UStaticMeshComponent>(Component)->GetStaticMesh()->GetOutermost()->GetName());
		
		GetMultipleMaterial::run(Component, ProjectPath, CompName, settings);
	}
	
	Actors.Empty();
	Components.Empty();
	GWarn->EndSlowTask();
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("GetMaterial DONE")));
	return FReply::Handled();
}

/*
FReply FGetMaterialAndObjModule::GetSingleMaterialMethod()
{
	GetSelectedActors();
	{
		for (auto actor : Actors)
		{
			UMaterialInterface* Material = actor->GetStaticMeshComponent()->GetMaterial(0);

			FString MaterialName = FixupMaterialName(Material);
			////FString MaterialLibFilename = FPaths::GetBaseFilename(OBJFilename, false) + TEXT(".mtl");

			//absolute address to save BMP
			FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir());

			FString BMPFilename = ProjectPath + TEXT("/GetMaterial/") + MaterialName;
			ExportMaterialPropertyTexture(BMPFilename + TEXT("_D.bmp"), Material, MP_BaseColor);
			ExportMaterialPropertyTexture(BMPFilename + TEXT("_MSR.bmp"), Material, MP_Metallic);
			ExportMaterialPropertyTexture(BMPFilename + TEXT("_N.bmp"), Material, MP_Normal);

		}
	}
}
*/

TSharedRef<SDockTab> FGetMaterialAndObjModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedPtr<SButton> GetMaterialBtn;
	TSharedPtr<SButton> GetObjBtn;

	TSharedRef<SDockTab> mainTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.ShouldAutosize(true)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(10, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				//.Padding(4, 4, 10, 4)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("TextureSize(X,Y):")))
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				//.Padding(4, 4, 10, 4)
				[
					SAssignNew(TextureSizeX, SSpinBox<int32>)
					//SNew(SSpinBox<int32>)
					.MaxValue(16384)
					.MinValue(1)
					.Value(1024)
				]
				+ SHorizontalBox::Slot()
				//.AutoHeight()
				.HAlign(HAlign_Center)
				//.Padding(4, 4, 10, 4)
				[
					SAssignNew(TextureSizeY, SSpinBox<int32>)
					.Value(1024)
					.MaxValue(16384)
					.MinValue(1)
				]
			]
			//放置按钮
			+ SScrollBox::Slot().Padding(10, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				//.AutoWeight()
				.HAlign(HAlign_Center)
				//.Padding(4, 4, 10, 4)
				[
					//SNew(SButton)
					SAssignNew(GetMaterialBtn, SButton)
					.Text(LOCTEXT("GetMaterial", "Get material"))
					.OnClicked_Raw(this, &FGetMaterialAndObjModule::GetMaterialMethod)
				]
				+ SHorizontalBox::Slot()
				//.AutoHeight()
				.HAlign(HAlign_Center)
				//.Padding(4, 4, 10, 4)
				[
					SAssignNew(GetObjBtn, SButton)
					.Text(LOCTEXT("GetObjAndMtl", "Get obj And mtl file"))
					.OnClicked_Raw(this, &FGetMaterialAndObjModule::GetObjMethod)
				]
			]
		];
	return mainTab;
}

void FGetMaterialAndObjModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(GetMaterialAndObjTabName);
}

void FGetMaterialAndObjModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FGetMaterialAndObjCommands::Get().OpenPluginWindow);
}

void FGetMaterialAndObjModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FGetMaterialAndObjCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGetMaterialAndObjModule, GetMaterialAndObj)