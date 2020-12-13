#include "SkyUtils.h"
#include "../Math/Constants.h"

#include "components/debug/Debug.h"

int SkyUtils::getOctantIndex(bool posX, bool posY, bool posZ)
{
	// Use lowest 3 bits to represent 0-7.
	const char xBit = posX ? 0 : (1 << 0);
	const char yBit = posY ? 0 : (1 << 1);
	const char zBit = posZ ? 0 : (1 << 2);
	return xBit | yBit | zBit;
}

Double3 SkyUtils::getSkyObjectDirection(Radians angleX, Radians angleY)
{
	// @todo: some sine and cosine functions
	DebugNotImplemented();
	return Double3();
}

void SkyUtils::getSkyObjectDimensions(int imageWidth, int imageHeight, double *outWidth, double *outHeight)
{
	DebugNotImplemented();
	// @todo: use ArenaSkyUtils constexpr values?
	*outWidth = 0;
	*outHeight = 0;
}
