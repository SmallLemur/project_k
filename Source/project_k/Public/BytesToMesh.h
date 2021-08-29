// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Vector.h"
#include "UObject/ObjectMacros.h"
#include "BytesToMesh.generated.h"

/**
 *
 */
UCLASS()
class PROJECT_K_API UBytesToMesh : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static void Generate(
			const TArray<uint8>& Heightmap,
			const float scale,
			const int32 tileSize,
			const float uvSpacing,
			TArray<FVector>& vertices,
			TArray<FVector>& normals,
			TArray<int32>& triangles,
			TArray<FVector2D>& uvs,
			TArray<FLinearColor>& vertexColors
		);


};
