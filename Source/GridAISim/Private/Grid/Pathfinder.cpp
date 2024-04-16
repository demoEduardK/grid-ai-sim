// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid/Pathfinder.h"

#include <functional>

#include "GameModes/GameModeDefault.h" // FGrid
#include "GridAISim/GridAISim.h"
#include "ProfilingDebugging/CountersTrace.h"


TRACE_DECLARE_INT_COUNTER(LessOpCount, TEXT("Less<> Operator() Uses"));

static const int32 ConnectionCost = 1;

bool Path::LessDistancePredicate::operator()(const FNodeRecord2& LeftRecord, const FNodeRecord2& RightRecord) const
{
	TRACE_COUNTER_INCREMENT(LessOpCount);
	UE_LOG(LogSim, Display, TEXT("LeftRecord: [%s]: Est.Cost: %s, Compare Weight: %s, IsVisited:%s, "
		       "\nFNodeRecord2 RightRecord[%s]: Est.Cost: %s,Compare Weight: %s, IsVisited:%s"),
	       *LeftRecord.Node.XY.ToString(), *FString::SanitizeFloat(LeftRecord.EstimatedTotalCost),
	       *FString::SanitizeFloat(LeftRecord.EstimatedTotalCost * StaticCast<int32>(LeftRecord.VisitStatus)),
	       *FString(LeftRecord.VisitStatus == Visited ? "Visited" : "NotVisited"),
	       *RightRecord.Node.XY.ToString(), *FString::SanitizeFloat(RightRecord.EstimatedTotalCost),
	       *FString::SanitizeFloat(RightRecord.EstimatedTotalCost * StaticCast<int32>(RightRecord.VisitStatus)),
	       *FString(RightRecord.VisitStatus == Visited ? "Visited" : "NotVisited"));
	return (LeftRecord.EstimatedTotalCost * StaticCast<int32>(LeftRecord.VisitStatus)
		< RightRecord.EstimatedTotalCost * StaticCast<int32>(RightRecord.VisitStatus));
};

TArray<Path::FConnection> Path::FGraph::GetNodeConnections(const FNodeRecord& InNodeRecord) const
{
	// TODO: The function must go through the neighboring Nodes and find the connections of a taken node
	/*
	 * We may use the rules of the Grid formation, like: Rect, Hex, Oct. Which will define the directions in which to check if there is a node
	 * NW NN NE | []		[1;0]		[]
	 * WW OO EE | [0;-1]	[0;0]		[0;1]
	 * SW SS SE | []		[-1;0]		[]
	 * Rect: NN, WW, EE, SS connections.
	 * Hex: NW, NE, WW, EE, SW, SE connections.
	 * Oct: All 8 directions.
	 *
	 * Using such rules will declare, that all the neighbors are 1 point away from the node with no distance calculations.
	 * To check: if the coordinates of interest are in bounds of the Grid size and are not below zero
	 * 0 <= X <= Grid.Size.X
	 * 0 <= Y <= Grid.Size.Y
	 */

	TArray<Path::FConnection> Connections;

	auto Points = GridRef.GetNodeConnections(FGridPoint());

	for (auto Point : Points)
	{
		Path::FConnection Connection;
		Connection.From = nullptr;
		Connection.To = nullptr;
		Connections.Add(Connection);
	}

	return Connections;
}

TArray<Path::FNode> Path::FGraph::GetNodeConnections(const FNode& InNode) const
{
	TArray<Path::FNode> NodeConnections;

	auto Points = GridRef.GetNodeConnections(FGridPoint(InNode.XY));

	for (auto Point : Points)
	{
		Path::FNode Node(Point.GridCoords);
		Node.bIsReachable = (Point.GameActor == nullptr ? true : false);
		NodeConnections.Emplace(Node);
	}

	return NodeConnections;
}

void GS_Pathfinder::InitGraph(const FGrid& InGrid)
{
	Graph = MakeUnique<Path::FGraph>(InGrid);
}

TArray<Path::FNode> GS_Pathfinder::FindPath(const Path::FNode& InStartNode, const Path::FNode& InEndNode)
{
	UE_LOG(LogSim, Display, TEXT("[FindPath] Building a path from %s to %s"), *InStartNode.XY.ToString(),
	       *InEndNode.XY.ToString());
	using namespace Path;

	TArray<FNodeRecord2*> NodesArray;

	TArray<Path::FNode> ResultNodes;

	//Path::FNodeRecordPtr StartNodeRecord = MakeShared<Path::FNodeRecord>(StartNode);
	FNodeRecord2* StartNodeRecord = new FNodeRecord2(InStartNode);
	StartNodeRecord->HeuristicValue = Path::FHeuristic::Estimate(InStartNode, InEndNode);
	StartNodeRecord->CostSoFar = 0.f;
	StartNodeRecord->VisitStatus = Discovered;
	StartNodeRecord->EstimatedTotalCost = StartNodeRecord->HeuristicValue;

	NodesArray.HeapPush(StartNodeRecord, Path::LessDistancePredicate());

	Path::FNodeRecord2* CurrentNodeRecord = nullptr;

	while (NodesArray.Num() > 0)
	{
		//NodesArray.HeapSort(Path::LessDistancePredicate());
		for (auto* Node : NodesArray)
		{
			UE_LOG(LogSim, Display, TEXT("[FindPath] NodeArray Node: %s, Est.Cost: %s. VisitStatus: %s"),
			       *Node->Node.XY.ToString(),
			       *FString::SanitizeFloat(Node->EstimatedTotalCost),
			       *FString(Node->VisitStatus == Visited ? "Visited" : Node->VisitStatus == Discovered ? "Discovered"
			        : Node->VisitStatus == EVisitStatus::Unvisited ? "Unvisited" : "Broken, or what?"));
		}
		if(CurrentNodeRecord == NodesArray.HeapTop())
		{
			UE_LOG(LogSim, Display, TEXT("[FindPath] Heap evaluation error. Welcome to the infinite loop! (jk, breaking the loop..)"));
			break;
		}
		//That's how it worked:
		//CurrentNodeRecord = NodesArray.HeapTop();

		// Update the current node's heap position with the new VisitStatus 
		NodesArray.HeapPop(CurrentNodeRecord, Path::LessDistancePredicate(), false);
		CurrentNodeRecord->VisitStatus = Visited;
		NodesArray.HeapPush(CurrentNodeRecord, Path::LessDistancePredicate());
		//~
		
		// Check if the current node is the target node
		if (CurrentNodeRecord->Node == InEndNode)
		{
			break;
		}

		UE_LOG(LogSim, Display, TEXT("[FindPath]Current Node: %s"), *CurrentNodeRecord->Node.XY.ToString());

		// // Get the current node's connections and iterate through them
		TArray<Path::FNode> NeighborNodes = Graph->GetNodeConnections(CurrentNodeRecord->Node);
		for (auto& NeighborNode : NeighborNodes)
		{
			UE_LOG(LogSim, Display, TEXT("[FindPath]   Neighbor Node: %s is %s"), *NeighborNode.XY.ToString(),
			       *FString(NeighborNode.bIsReachable ? "reachable" : "not reachable"));
			if (!NeighborNode.bIsReachable)
			{
				continue;
			}
			Path::FNodeRecord2 NextNodeRecord(NeighborNode);
			const float NextNodeCost = CurrentNodeRecord->CostSoFar + ConnectionCost;
			float NextNodeHeuristicValeue = 0.f;

			auto SearchPredicate = [&NextNodeRecord](const FNodeRecord2* NodeRecord)
			{
				return NextNodeRecord.Node == NodeRecord->Node;
			};

			//Connections may lead to discovered nodes
			if (auto* FoundNode = NodesArray.FindByPredicate(SearchPredicate))
			{
				if ((*FoundNode)->CostSoFar >= NextNodeCost)
				{
					continue;
				}

				NextNodeHeuristicValeue = (*FoundNode)->EstimatedTotalCost - (*FoundNode)->CostSoFar;
				NextNodeRecord.VisitStatus = (*FoundNode)->VisitStatus;

				// If a visited node somehow becomes effective again, make it visitable
				if ((*FoundNode)->VisitStatus == Visited)
				{
					(*FoundNode)->VisitStatus = Discovered;
				}
			}
			// Or to a new undiscovered node
			else
			{
				NextNodeHeuristicValeue = Path::FHeuristic::Estimate(NeighborNode, InEndNode);
			}

			NextNodeRecord.HeuristicValue = NextNodeHeuristicValeue;
			NextNodeRecord.CostSoFar = NextNodeCost;
			NextNodeRecord.ParentNodePtr = CurrentNodeRecord;
			NextNodeRecord.EstimatedTotalCost = NextNodeCost + NextNodeHeuristicValeue;

			if (NextNodeRecord.VisitStatus == Unvisited)
			{
				NextNodeRecord.VisitStatus = Discovered;
				NodesArray.HeapPush(new FNodeRecord2(NextNodeRecord), Path::LessDistancePredicate());
			}
		}
		//CurrentNodeRecord->VisitStatus = Visited;
		// TODO: check what's better - to sort the Heap, or to Pop and Push back a single element.
		//NodesArray.HeapSort(Path::LessDistancePredicate());
	}

	if (CurrentNodeRecord->Node != InEndNode)
	{
		UE_LOG(LogSim, Display, TEXT("[FindPath] No path could be found from %s to %s"), *InStartNode.XY.ToString(),
		   *InEndNode.XY.ToString());
		return TArray<Path::FNode>();
	}
	else
	{
		while (CurrentNodeRecord != nullptr)
		{
			ResultNodes.Add(CurrentNodeRecord->Node);
			if (auto* ParentNode = CurrentNodeRecord->ParentNodePtr)
			{
				CurrentNodeRecord = ParentNode;
			}
			else
			{
				break;
			}
		}
		Algo::Reverse(ResultNodes);
	}

	// TODO: make sure, that 
	NodesArray = TArray<FNodeRecord2*>();
	/*for (auto* entry : NodesArray)
	{
		delete entry;
	}*/


	FString NodesString;
	for (auto Node : ResultNodes)
	{
		NodesString.Appendf(TEXT(" [%s] "), *Node.XY.ToString());
	}

	UE_LOG(LogSim, Display, TEXT("[FindPath] Path found: %s"), *NodesString);
	return ResultNodes;
}

TArray<Path::FNode> GS_Pathfinder::GetNeighbors(const Path::FNode& InNode)
{
	return Graph->GetNodeConnections(InNode);
}

void GS_Pathfinder::VisualizePath(UWorld* World, TArray<Path::FNode> Array, float GridScale)
{
	if (World == nullptr)
	{
		UE_LOG(LogSim, Warning, TEXT("[VisualizePath] World is nullptr."))
		return;
	}

	if (Array.Num() < 2)
	{
		UE_LOG(LogSim, Display, TEXT("[VisualizePath] Path is too short for visualization."))
		return;
	}

	const auto FirstNode = *Array.begin();
	const FVector StartVec{
		StaticCast<float>(FirstNode.XY.X) * GridScale, StaticCast<float>(FirstNode.XY.Y) * GridScale,
		100.f
	};

	FVector PreviousLocation = StartVec;
	for (const auto Node : Array)
	{
		if (Node == FirstNode)
		{
			continue;
		}
		const FVector NodeVec{
			StaticCast<float>(Node.XY.X) * GridScale, StaticCast<float>(Node.XY.Y) *
			GridScale,
			100.f
		};
		DrawDebugLine(World, PreviousLocation, NodeVec, FColor::Green, true, 100.f, 10, 5.f);
		UE_LOG(LogSim, Display, TEXT("[VisualizePath] Draw line from %s to %s."), *PreviousLocation.ToString(),
		       *NodeVec.ToString());
		PreviousLocation = NodeVec;
	}
}

GS_Pathfinder::~GS_Pathfinder()
{
}
