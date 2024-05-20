// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RogueCover_Settings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Rogue Cover Settings"))
class ROGUECOVER_API URogueCover_Settings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	URogueCover_Settings();	

	UPROPERTY(Category = Default, EditAnywhere)
	float MinSpaceBetweenValidPoints = 50.f;
	
	/** If the cover generation should be regenerated when navigation is rebuilt at runtime */
	UPROPERTY(Category = Default, EditAnywhere)
	bool bRegenerateAtNavigationRebuilt = false;

	UPROPERTY(Category = Default, EditAnywhere)
	float SegmentLength = 10; 

	UPROPERTY(Category = Default, EditAnywhere)
	float MaxSegDist = 350;

	UPROPERTY(Category = Default, EditAnywhere)
	float TraceLength = 100;
	
	/** Size of the spheres used to sphere-cast */
	UPROPERTY(Category = Default, EditAnywhere)
	float SphereSize = 5;

	UPROPERTY(Category = Default, EditAnywhere)
	float StepDistTrace = 20;

	/** The maximum height of the agents when crouching */
	UPROPERTY(Category = Default, EditAnywhere)
	float WidthMaxAgent = 80;

	/** The maximum height of the agents when crouching */
	UPROPERTY(Category = Default, EditAnywhere)
	float HeightMaxCrouching = 80;

	/** The maximum height of the agents when standing */
	UPROPERTY(Category = Default, EditAnywhere)
	float HeightMaxStanding = 180;

	/** The distance between in cover position and leaning out of the cover on the sides */
	UPROPERTY(Category = Default, EditAnywhere)
	float OffsetWhenLeaningSides = 65;

	/** The distance in front of a shooting position that must be free */
	UPROPERTY(Category = Default, EditAnywhere)
	float OffsetFrontAim = 100;

	UPROPERTY(Category = Default, EditAnywhere)
	float CharHeight = 180;

	/** Debug */

	/** Enable or disable all debugging. */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool bShowCoverDebug = false;

	/** Draw a green sphere for the start points, a red for the end and blue for pieces. */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool bDraw1AllSegmentPointsTested = false;

	/** Draw a green sphere when the point is not within bounds of an existing one, otherwise red. */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool bDraw2SegmentPointsWithinBounds = false;

	/** Draw a green sphere when the point passed the test, or red if failed the first line trace, orange if failed one of the sub line trace. */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool bDraw3SimpleCoverGeometryTest = false;

	/** Draw a red sphere if the position failed the side trace as well as the trace in blue and the collision in red. Green sphere is succeeded. */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool bDraw4SecondPassTracesSides = false;
	
	/** Draw a green if successful, or orange sphere + red trace if failed side bottom or side front trace. */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool bDraw4SecondPassTracesSidesFrontAndBottom = false;

	UPROPERTY(Category = Debug, EditAnywhere)
	bool bDraw5SecondPassArrows = false;

	// Display all the points found in the world
	UPROPERTY(Category = Debug, EditAnywhere)
	bool DebugDrawAllPoints = false;

	// Display all the points currently used in the world
	UPROPERTY(Category = Debug, EditAnywhere)
	bool DebugDrawAllBusyPoints = false;

	/** Toggle the text log outputs */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool DebugDisplayLog = false;

	/** The maximum distance from this actor at which to draw the debug information */
	UPROPERTY(Category = Debug, EditAnywhere)
	float DebugDistance = 25000.f;

	/** Toggle the display of the Octree boundaries */
	UPROPERTY(Category = Debug, EditAnywhere)
	bool bDrawOctreeBounds = false;
};
