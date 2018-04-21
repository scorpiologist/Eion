// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Layout/ArrangedWidget.h"
#include "Widgets/SWidget.h"
#include "EdGraphUtilities.h"
#include "ConnectionDrawingPolicy.h"

class FSlateWindowElementList;
class UEdGraph;

struct FVoxelGraphConnectionDrawingPolicyFactory : public FGraphPanelPinConnectionFactory
{
public:
	virtual ~FVoxelGraphConnectionDrawingPolicyFactory() {}

	// FGraphPanelPinConnectionFactory
	virtual class FConnectionDrawingPolicy* CreateConnectionPolicy(const class UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const class FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;
	// ~FGraphPanelPinConnectionFactory

};


class FSlateWindowElementList;
class UEdGraph;

/////////////////////////////////////////////////////
// FVoxelGraphConnectionDrawingPolicy

// This class draws the connections for an UEdGraph using a Voxel schema
class FVoxelGraphConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
protected:
	// Times for one execution pair within the current graph
	struct FTimePair
	{
		double PredExecTime;
		double ThisExecTime;

		FTimePair()
			: PredExecTime(0.0)
			, ThisExecTime(0.0)
		{
		}
	};

	// Map of pairings
	typedef TMap<UEdGraphNode*, FTimePair> FExecPairingMap;

	// Map of nodes that preceded before a given node in the execution sequence (one entry for each pairing)
	TMap<UEdGraphNode*, FExecPairingMap> PredecessorNodes;

	UEdGraph* GraphObj;

	FLinearColor ActiveColor;
	FLinearColor InactiveColor;

	float ActiveWireThickness;
	float InactiveWireThickness;

public:
	FVoxelGraphConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);

	// FConnectionDrawingPolicy interface
	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params) override;
	virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& PinGeometries, FArrangedChildren& ArrangedNodes) override;
	// End of FConnectionDrawingPolicy interface

	FLinearColor AttackColor;
	FLinearColor SustainColor;
	FLinearColor ReleaseColor;

	float AttackWireThickness;
	float SustainWireThickness;
	float ReleaseWireThickness;
	float DefaultDataWireThickness;
	float DefaultExecutionWireThickness;

	float TracePositionBonusPeriod;
	float TracePositionExponent;
	float AttackHoldPeriod;
	float DecayPeriod;
	float DecayExponent;
	float SustainHoldPeriod;
	float ReleasePeriod;
	float ReleaseExponent;
};
