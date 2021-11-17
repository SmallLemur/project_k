// Fill out your copyright notice in the Description page of Project Settings.


#include "VTSUtil.h"

FMatrix UVTSUtil::vts2Matrix(float proj[16]) {
	return FMatrix(
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

}FMatrix UVTSUtil::vts2Matrix(double proj[16]) {
	return FMatrix(
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

// not sure it is super useful to rewrite this
FTransform UVTSUtil::matrix2Transform(FMatrix m) {
	FTransform *tr = new FTransform();
	tr->SetLocation(m.GetColumn(3));
	float sxs = m.Determinant() < 0 ? -1 : 1;
	tr->SetScale3D(	
		FVector(m.GetColumn(0).Size() * sxs,
				m.GetColumn(1).Size(),
				m.GetColumn(2).Size())
	);
	tr->SetRotation(
		FQuat::FindBetweenVectors(
			m.GetColumn(2) / tr->GetScale3D().Z,
			m.GetColumn(1) / tr->GetScale3D().Y)
	);
	return *tr;
}

void UVTSUtil::matrix2vts(FMatrix mat, double out[16]) {
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			out[j + i * 4] = mat.M[i][j];
		}
	}
}

FVector UVTSUtil::vts2vector(double vec[3]) {
	return FVector(
		vec[0],
		vec[1],
		vec[2]
	);
}

FRotator UVTSUtil::vts2rotator(double vec[3]) {
	return FRotator(
		vec[0],
		vec[1],
		vec[2]
	);
}

void UVTSUtil::vector2vts(FVector vec, double out[3]) {
	out[0] = vec.X;
	out[1] = vec.Y;
	out[2] = vec.Z;
}

const FMatrix UVTSUtil::SwapYZ = FMatrix(
	FPlane(1, 0, 0, 0),
	FPlane(0, 0, 1, 0),
	FPlane(0, 1, 0, 0),
	FPlane(0, 0, 0, 1)
);


short UVTSUtil::BytesToInt16(vts::Buffer& input, uint32 startOffset)
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

unsigned short UVTSUtil::BytesToUInt16(vts::Buffer& input, uint32 startOffset)
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

	return (unsigned short)UVTSUtil::BytesToInt16(input, startOffset);
}

int UVTSUtil::BytesToInt32(vts::Buffer& input, uint32 startOffset)
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

	// 0x00000000
	if (true/*isLittleEndian*/) {       // MSB is loacted in higher address.
		return val;
	}
	else {                    // MSB is located in lower address.
		return (d[startOffset] << 24) | (d[startOffset] << 16) |
			(d[startOffset] << 8) | (d[startOffset]);
	}
}

float UVTSUtil::BytesToSingle(vts::Buffer& input, uint32 startOffset)
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
	int val = UVTSUtil::BytesToInt32(input, startOffset);
	return *(float*)&val;
}

float UVTSUtil::ExtractFloat(vts::GpuMeshSpec& spec, uint32 byteOffset, vts::GpuTypeEnum type, bool normalized)
{
	switch (type)
	{
	case vts::GpuTypeEnum::Float:
		return UVTSUtil::BytesToSingle(spec.vertices, byteOffset);
	case vts::GpuTypeEnum::UnsignedShort:
		return UVTSUtil::BytesToUInt16(spec.vertices, byteOffset) / 65535.0f;
	default:
		return 0;
	}
}

TArray<FVector>* UVTSUtil::ExtractBuffer3(vts::GpuMeshSpec& spec, int attributeIndex) {
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
			UVTSUtil::ExtractFloat(spec, start + i * stride + 0 * (int)typeSize, a.type, a.normalized),
			UVTSUtil::ExtractFloat(spec, start + i * stride + 1 * (int)typeSize, a.type, a.normalized),
			UVTSUtil::ExtractFloat(spec, start + i * stride + 2 * (int)typeSize, a.type, a.normalized)
		);
		r->Insert(*vec, i);
	}
	return r;
}

TArray<int32>* UVTSUtil::LoadTrianglesIndices(vts::GpuMeshSpec& spec) {
	auto indices = new TArray<int32>();

	if (spec.indicesCount > 0) {
		for (uint32 i = 0; i < spec.indicesCount; i++) {
			indices->Add(UVTSUtil::BytesToInt16(spec.indices, i * 2));
		}
	}
	else
	{
		for (uint32 i = 0; i < spec.verticesCount; i += 3)
		{
			indices->Add(i + 0);
			indices->Add(i + 1);
			indices->Add(i + 2);
		}
	}
	return indices;
}