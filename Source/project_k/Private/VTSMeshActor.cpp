// Fill out your copyright notice in the Description page of Project Settings.


#include "VTSMeshActor.h"

// Sets default values
AVTSMeshActor::AVTSMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVTSMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVTSMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVTSMeshActor::LoadMesh(FVTSLoadMesh* loadMeshInfo)
{
	TArray<FVector>* vertices = ExtractBuffer3(*loadMeshInfo->Spec, 0);
	//TArray<FVector>& normals;
	TArray<int32>* triangles = LoadTrianglesIndices(*loadMeshInfo->Spec);
	//TArray<FVector2D>& uvs;

	OnLoadMeshSimple.Broadcast(*vertices, *triangles);
}

TArray<FVector>* AVTSMeshActor::ExtractBuffer3(vts::GpuMeshSpec& spec, int attributeIndex) {
	auto a = spec.attributes[attributeIndex];
	if (!a.enable)
		return nullptr;

	uint32 typeSize = vts::gpuTypeSize(a.type);
	TArray<FVector>* r = new TArray<FVector>[spec.verticesCount];
	int stride = (int)(a.stride == 0 ? typeSize * a.components : a.stride);
	int start = (int)a.offset;
	for (uint32 i = 0; i < spec.verticesCount; i++)
	{
		FVector* vec = new FVector(
			ExtractFloat(spec, start + i * stride + 0 * (int)typeSize, a.type, a.normalized),
			ExtractFloat(spec, start + i * stride + 1 * (int)typeSize, a.type, a.normalized),
			ExtractFloat(spec, start + i * stride + 2 * (int)typeSize, a.type, a.normalized)
		);
		r->Insert(*vec, i);
	}
	return r;
}

TArray<int32>* AVTSMeshActor::LoadTrianglesIndices(vts::GpuMeshSpec& spec) {
	auto indices = new TArray<int32>();
	if (spec.indicesCount > 0) {
		for (uint32 i = 0; i < spec.indicesCount; i++) {
			indices->Add(BytesToInt16(spec.indices, i * 2));
		}
	}
	else
	{
		indices = new TArray<int32>[spec.verticesCount];
		for (uint32 i = 0; i < spec.verticesCount; i += 3)
		{
			indices->Insert(i + 0, i + 0);
			indices->Insert(i + 1, i + 1);
			indices->Insert(i + 2, i + 2);
		}
	}
	return indices;
}

float AVTSMeshActor::ExtractFloat(vts::GpuMeshSpec& spec, uint32 byteOffset, vts::GpuTypeEnum type, bool normalized)
{
	switch (type)
	{
	case vts::GpuTypeEnum::Float:
		return BytesToSingle(spec.vertices, byteOffset);
	case vts::GpuTypeEnum::UnsignedShort:
		return BytesToUInt16(spec.vertices, byteOffset) / 65535.0f;
	default:
		return 0;
	}
}


FMatrix* AVTSMeshActor::vts2Matrix(double proj[16]) {
	return new FMatrix(
		FPlane(
			proj[0],
			proj[1],
			proj[2],
			proj[3]
		),
		FPlane(
			proj[4],
			proj[5],
			proj[6],
			proj[7]
		),
		FPlane(
			proj[8],
			proj[9],
			proj[10],
			proj[11]
		),
		FPlane(
			proj[12],
			proj[13],
			proj[14],
			proj[15]
		)
	);
}

short AVTSMeshActor::BytesToInt16(vts::Buffer& input, uint32 startOffset)
{
	if (input.size() == 0) {
		return 0;
	}
	if ((unsigned int)startOffset >= input.size()) {
		return 0;
	}
	if (startOffset > input.size() - 2) {
		return 0;
	}

	// 0x0000
	if (true/*isLittleEndian*/) {       // MSB is loacted in higher address.
		return ((uint8)input.data()[startOffset]) | ((uint8)input.data()[startOffset + 1] << 8);
	}
	else {                    // MSB is located in lower address.
		return (input.data()[startOffset] << 8) | (input.data()[startOffset]);
	}
}

unsigned short AVTSMeshActor::BytesToUInt16(vts::Buffer& input, uint32 startOffset)
{
	if (input.size() == 0) {
		return 0;
	}
	if (startOffset >= input.size()) {
		return 0;
	}
	if (startOffset > input.size() - 2) {
		return 0;
	}

	return (unsigned short)BytesToInt16(input, startOffset);
}

int AVTSMeshActor::BytesToInt32(vts::Buffer& input, uint32 startOffset)
{
	if (input.size() == 0) {
		return 0;
	}
	if (startOffset >= input.size()) {
		return 0;
	}
	if (startOffset > input.size() - 4) {
		return 0;
	}

	auto d = input.data();

	uint8 b1 = d[startOffset];
	int b1i = b1 << 0;
	uint8 b2 = d[startOffset + 1];
	int b2i = b2 << 8;
	uint8 b3 = d[startOffset + 2];
	int b3i = b3 << 16;
	uint8 b4 = d[startOffset + 3];
	int b4i = b4 << 24;

	//int val = b1i | b2i | b3i | b4i;
	int val = b4i | b3i | b2i | b1i;

	return val;
	/*
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b1));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b1i));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b2));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b2i));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b3));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b3i));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b4));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + b4i));
	GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, FString("" + val));
	*/
	// 0x00000000
	if (true/*isLittleEndian*/) {       // MSB is loacted in higher address.
		return val;
		/*
		return ((int8)d[startOffset]) | ((int8)d[startOffset + 1] << 8) |
			((int8)d[startOffset + 2] << 16) | ((int8)d[startOffset + 3] << 24);
			*/
	}
	else {                    // MSB is located in lower address.
		return (d[startOffset] << 24) | (d[startOffset] << 16) |
			(d[startOffset] << 8) | (d[startOffset]);
	}
}

float AVTSMeshActor::BytesToSingle(vts::Buffer& input, uint32 startOffset)
{
	if (input.size() == 0) {
		return 0;
	}
	if (startOffset >= input.size()) {
		return 0;
	}
	if (startOffset > input.size() - 4) {
		return 0;
	}
	int val = BytesToInt32(input, startOffset);
	return *(float*)&val;
}
