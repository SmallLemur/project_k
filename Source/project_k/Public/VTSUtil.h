// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ObjectMacros.h"
#include "Math/Vector.h"

#include <vts-browser/map.hpp>
#include <vts-browser/camera.hpp>
#include <vts-browser/cameraDraws.hpp>
#include <vts-browser/navigation.hpp>
#include <vts-browser/position.hpp>
#include <vts-browser/log.h>
#include <vts-browser/mapCallbacks.hpp>
#include <vts-browser/resources.hpp>

#include "VTSUtil.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_K_API UVTSUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FMatrix vts2Matrix(float proj[16]);
	static FMatrix vts2Matrix(double proj[16]);
	static FTransform matrix2Transform(FMatrix m);
	static FVector vts2vector(double vec[3]);
	static FRotator vts2rotator(double vec[3]);
	static void matrix2vts(FMatrix mat, double out[16]);
	static void vector2vts(FVector vec, double out[3]);

	static short BytesToInt16(vts::Buffer& input, uint32 startOffset);
	static unsigned short BytesToUInt16(vts::Buffer& input, uint32 startOffset);
	static int BytesToInt32(vts::Buffer& input, uint32 startOffset);
	static float BytesToSingle(vts::Buffer& input, uint32 startOffset);
	
	static float ExtractFloat(vts::GpuMeshSpec& spec, uint32 byteOffset, vts::GpuTypeEnum type, bool normalized);
	static TArray<FVector>* ExtractBuffer3(vts::GpuMeshSpec& spec, int attributeIndex);
	static TArray<int32>* LoadTrianglesIndices(vts::GpuMeshSpec& spec);

	//https://github.com/melowntech/vts-browser-unity-plugin/blob/722f4e591d08d4a0b5aa983e824f177caa4b7904/src/Vts/Scripts/BrowserUtil/VtsUtil.cs
	
	
	static const FMatrix SwapYZ;
	static const FMatrix OpenGL2UE;
};
