// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grid.h"
#include "GridAISim/GridAISim.h"

struct FGrid;

namespace Path
{
	struct FConnection;
	struct FNodeRecord;

	struct GRIDAISIM_API FNode
	{
		FIntPoint XY;
		bool bIsReachable = false;

		bool operator==(const FNode& InNode) const
		{
			return (this->XY == InNode.XY);
		}
	};

	struct FConnection
	{
		FNode NodeFrom;
		FNode NodeTo;
		float ConnectionCost = 0.f;

		FNodeRecord* From = nullptr;
		FNodeRecord* To = nullptr;
	};

	enum EVisitStatus
	{
		Unvisited = 0,
		Discovered = 1,
		Visited = 2
	};

	struct GRIDAISIM_API FNodeRecord
	{
		FNodeRecord()
		{
			VisitStatus = Unvisited;
		};

		FNodeRecord(const FNode& InNode, EVisitStatus InVisitStatus = Unvisited)
		{
			Node = InNode;
			VisitStatus = InVisitStatus;
		}
		~FNodeRecord()
		{
			UE_LOG(LogSim, Display, TEXT("Node Record for %s is deconstructed."), *Node.XY.ToString());
		}
		
		float HeuristicValue = 0.f;
		float CostSoFar = 0.f;
		float EstimatedTotalCost = 0.f;
		FNode Node;
		FNodeRecord* ParentNodePtr = nullptr;
		EVisitStatus VisitStatus;
	};

	struct GRIDAISIM_API FHeuristic
	{
		FHeuristic() = delete;

		FHeuristic(const FNode& InEndNode)
		{
			EndNode = InEndNode;
		}

		float Estimate(const FNode& InStartNode) const
		{
			return FIntPoint(EndNode.XY - InStartNode.XY).SizeSquared();
		}

		static float Estimate(const FNode& InStartNode, const FNode& InEndNode)
		{
			return FIntPoint(InEndNode.XY - InStartNode.XY).SizeSquared();
		}

	private:
		FNode EndNode;
	};

	/**
	* A predicate class for pushing NodeRecords to the heapified open and closed lists.
	*/
	struct GRIDAISIM_API LessDistancePredicate
	{
		bool operator()(const FNodeRecord& LeftRecord, const FNodeRecord& RightRecord) const;
	};

	struct GRIDAISIM_API FGraph
	{
		FGraph(const FGrid& InGrid)
			: GridRef(InGrid)
		{
		}

		TArray<FConnection> GetNodeConnections(const FNodeRecord& InNodeRecord) const;
		TArray<FNode> GetNodeConnections(const FNode& InNode) const;

		const FGrid& GridRef;
	};
}

/**
 * 
 */
class GRIDAISIM_API GS_Pathfinder
{
public:
	GS_Pathfinder() = default;

	void InitGraph(const FGrid& InGrid);

	TArray<Path::FNode> FindPath(const Path::FNode& StartNode, const Path::FNode& EndNode);
	TArray<Path::FNode> GetNeighbors(const Path::FNode& InNode);
	void VisualizePath(UWorld* World, TArray<Path::FNode> Array, float GridScale);

	~GS_Pathfinder();

private:
	TUniquePtr<Path::FGraph> Graph;
};
