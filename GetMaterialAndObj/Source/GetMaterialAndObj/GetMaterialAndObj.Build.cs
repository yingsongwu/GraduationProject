// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GetMaterialAndObj : ModuleRules
{
	public GetMaterialAndObj(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"GetMaterialAndObj/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"GetMaterialAndObj/Private",
				// ... add other private include paths required here ...
			}
			);
        PublicIncludePathModuleNames.AddRange(
            new string[] {
                "HierarchicalLODUtilities",
                "MeshUtilities",
                "MeshReductionInterface",
             }
            );

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "HierarchicalLODUtilities",
                "MeshUtilities",
                "MeshReductionInterface",
                "MaterialBaking",
            }
            );

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                // ... add other public dependencies that you statically link with here ...
                "MeshUtilities",
                "MaterialUtilities",
			    "RawMesh",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                // ... add private dependencies that you statically link with here ...	
                 "Core",
                "CoreUObject",
                "Engine",
                "RenderCore",
                "Renderer",
                "RHI",
                "Landscape",
                "UnrealEd",
                "ShaderCore",
                "MaterialUtilities",
                "SlateCore",
                "Slate",
                "StaticMeshEditor",
                "SkeletalMeshEditor",
                "MaterialBaking",
                "MeshMergeUtilities"
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
                "HierarchicalLODUtilities",
                "MeshReductionInterface",
            }
			);
	}
}
