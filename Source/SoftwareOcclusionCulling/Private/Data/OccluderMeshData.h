// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OccluderMeshData.generated.h"

USTRUCT()
struct FOccluderMeshData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<uint16> Indices;

	FOccluderMeshData() = default;
	explicit FOccluderMeshData(UStaticMesh* StaticMesh)
	{
		if (IsRunningDedicatedServer())
		{
			return;
		}
		
		if(!IsValid(StaticMesh))
		{
			return;
		}

		const FStaticMeshLODResources& LODModel = StaticMesh->GetRenderData()->LODResources[StaticMesh->GetRenderData()->CurrentFirstLODIdx];
		const FRawStaticIndexBuffer& IndexBuffer = LODModel.DepthOnlyIndexBuffer.GetNumIndices() > 0 ? LODModel.DepthOnlyIndexBuffer : LODModel.IndexBuffer;
		if (!IndexBuffer.AccessStream16())
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot access 16-bit IndexBuffer for Occlusion Mesh: %s"), *GetNameSafe(StaticMesh));
			return;
		}

		const int32 NumVtx = LODModel.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		const int32 NumIndices = IndexBuffer.GetNumIndices();
		if (NumVtx > 0 && NumIndices > 0 && !IndexBuffer.Is32Bit())
		{
			Vertices.SetNumUninitialized(NumVtx);
			for (int i = 0; i < NumVtx; ++i)
			{
				Vertices.GetData()[i] = FVector(LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i));
			}

			Indices.SetNumUninitialized(NumIndices);
			for (int i = 0; i < NumIndices; ++i)
			{
				Indices.GetData()[i] = IndexBuffer.AccessStream16()[i];
			}
		}
	}
};