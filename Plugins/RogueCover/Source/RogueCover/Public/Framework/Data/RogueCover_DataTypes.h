#pragma once

#include "Framework/CoverSystem/RogueCover_Point.h"


struct FCoverPointOctreeElement
{
	FBoxSphereBounds Bounds;

	/** Cover Point these data represents */
	const TWeakObjectPtr<URogueCover_Point> CoverPoint;

	FCoverPointOctreeElement(URogueCover_Point& CoverPointIn)
		:CoverPoint(&CoverPointIn)
	{
		Bounds = FBoxSphereBounds(&CoverPoint->Location, 1);
	}

	FORCEINLINE URogueCover_Point* GetOwner() const { return CoverPoint.Get(); }
};

struct FCoverPointOctreeSemantics
{
	enum { MaxElementsPerLeaf = 16 };
	enum { MinInclusiveElementsPerNode = 7 };
	enum { MaxNodeDepth = 12 };

	/** Using the heap allocator instead of an inline allocator to trade off add/remove performance for memory. */
	/** Since we won't generate covers after init, should be ok. */
	typedef FDefaultAllocator ElementAllocator;

	FORCEINLINE static bool AreElementsEqual(const FCoverPointOctreeElement& A, const FCoverPointOctreeElement& B)
	{
		return A.CoverPoint == B.CoverPoint;
	}

	static void SetElementId(const FCoverPointOctreeElement& Element, FOctreeElementId2 Id)
	{
	}

	FORCEINLINE static const FBoxSphereBounds& GetBoundingBox(const FCoverPointOctreeElement& CoverData)
	{
		return CoverData.Bounds;
	}
};

typedef TOctree2<FCoverPointOctreeElement, FCoverPointOctreeSemantics> FCoverPointOctree;