#pragma once
#include "Components/PrimitiveComponent.h"
#include "Engine/MeshMerging.h"
#include "Exporters/Exporter.h"

class GetMultipleMaterial
{
public:
	GetMultipleMaterial();
	~GetMultipleMaterial();
	static void run(UPrimitiveComponent* Component, FString PathName, FString Name, FMeshMergingSettings settings);
	static void AssembleListOfExporters(TArray<UExporter*>& OutExporters);
	static void ExportMaterialArray(UPrimitiveComponent* Component, FMeshMergingSettings settings, TArray<UObject*>& OutAssetsToSync, FString& InBasePackageName);
	static void ExportObjectsToBMP(TArray<UObject*>& ObjectsToExport);
		//static void run(const TArray<UPrimitiveComponent*> &Component, FString PathName);
};

