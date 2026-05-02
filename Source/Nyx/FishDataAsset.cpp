#include "FishDataAsset.h"

FName UFishDataAsset::GetStableFishId() const
{
	return FishId.IsNone() ? GetFName() : FishId;
}
