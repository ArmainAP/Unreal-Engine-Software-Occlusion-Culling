// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"

typedef TArray<FVector> FOccluderVertexArray;
typedef TArray<uint16> FOccluderIndexArray;
typedef TSharedPtr<FOccluderVertexArray, ESPMode::ThreadSafe> FOccluderVertexArraySP;
typedef TSharedPtr<FOccluderIndexArray, ESPMode::ThreadSafe> FOccluderIndexArraySP;

struct FOccluderMeshData
{
	FOccluderVertexArraySP VerticesSP = MakeShared<FOccluderVertexArray, ESPMode::ThreadSafe>();
	FOccluderIndexArraySP IndicesSP = MakeShared<FOccluderIndexArray, ESPMode::ThreadSafe>();

	FOccluderMeshData() = default;
	explicit FOccluderMeshData(UStaticMesh* StaticMesh)
	{
		if(!IsValid(StaticMesh))
		{
			return;
		}
		
		if (UKismetSystemLibrary::IsDedicatedServer(StaticMesh))
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
			VerticesSP->SetNumUninitialized(NumVtx);
			for (int i = 0; i < NumVtx; ++i)
			{
				VerticesSP->GetData()[i] = FVector(LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i));
			}

			IndicesSP->SetNumUninitialized(NumIndices);
			for (int i = 0; i < NumIndices; ++i)
			{
				IndicesSP->GetData()[i] = IndexBuffer.AccessStream16()[i];
			}
		}
	}
};