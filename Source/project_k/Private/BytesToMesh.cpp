// Fill out your copyright notice in the Description page of Project Settings.


#include "BytesToMesh.h"


void UBytesToMesh::Generate(
    const TArray<uint8>& Heightmap,
    const float scale, // scale is the verticale scaling factor
    const int32 tileSize, // Tile size is the spacing between vertices
    const float uvSpacing,
    TArray<FVector>& vertices,
    TArray<FVector>& normals,
    TArray<int32>& triangles,
    TArray<FVector2D>& uvs,
    TArray<FLinearColor>& vertexColors
) {

    int32 width = FMath::Sqrt(Heightmap.Num() / 4);
    int32 height = FMath::Sqrt(Heightmap.Num() / 4);

    // generate vertices

    int32 x = 0;
    int32 y = 0;
    for (int i = 0; i < Heightmap.Num(); i += 4) {
        FLinearColor pixelColor = FLinearColor(
            Heightmap[i] / 255.0f, // R
            Heightmap[i + 1] / 255.0f, // G
            Heightmap[i + 2] / 255.0f, // B
            Heightmap[i + 3] / 255.0f// A
        );

        float vertexHeight = FMath::Fmod((pixelColor.LinearRGBToHSV().R + 120.0f), 360.0f) / 360.0f;

        vertices.Add(FVector(x * tileSize, y * tileSize, vertexHeight * scale));
        normals.Add(FVector(0.0f, 0.0f, 1.0f));
        uvs.Add(FVector2D(x * uvSpacing, y * uvSpacing));
        vertexColors.Add(pixelColor);

        ++x;
        if (x >= width) {
            x = 0;
            ++y;
        }
    }
    /*
    for (y = 0; y < height - 1; y++)
    {
        for (x = 0; x < width - 1; x++)
        {
            triangles.Add(x + (y * width));                    //current vertex
            triangles.Add(x + (y * width) + width);            //current vertex + row
            triangles.Add(x + (y * width) + width + 1);        //current vertex + row + one right

            triangles.Add(x + (y * width));                    //current vertex
            triangles.Add(x + (y * width) + width + 1);        //current vertex + row + one right
            triangles.Add(x + (y * width) + 1);                //current vertex + one right
        }
    }
    */

    for (y = height - 1; y > 0; y--)
    {
        for (x = width - 1; x > 0; x--)
        {
            triangles.Add(x + y * width);
            triangles.Add((x - 1) + y * width);
            triangles.Add((x - height) + y * width);

            triangles.Add((x - 1) + y * width);
            triangles.Add((x - 1) + (y - 1) * width);
            triangles.Add(x + (y - 1) * width);
        }
    }

}
