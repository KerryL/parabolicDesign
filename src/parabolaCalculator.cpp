/*===================================================================================
                                 Parabolic Design App
                          Copyright Kerry R. Loux 2021

                   This code is licensed under the GPLv2 License
                     (http://opensource.org/licenses/GPL-2.0).

===================================================================================*/

// File:  parabolaCalculator.cpp
// Date:  1/22/2021
// Auth:  K. Loux
// Desc:  Calculations for parabolic reflectors.

// Local headers
#include "parabolaCalculator.h"

// Standard C++ headers
#include <cassert>

const double ParabolaCalculator::speedOfSound(13503.937008);// [in/sec]

double ParabolaCalculator::GetMinAmplifiedFrequency() const
{
	return speedOfSound / parabolaInfo.diameter;// [Hz]
}

double ParabolaCalculator::GetParabolaDepth() const
{
	return parabolaInfo.diameter * parabolaInfo.diameter * 0.0625 / parabolaInfo.focusPosition;
}

double ParabolaCalculator::GetMaxDesignError() const
{
	// The design creates the desired parabola along the center of each facet.  So the
	// largest error will be at the widest part of the parabola, were two facets join.
	// TODO:  Calculate
	return -1.0;
}
	
ParabolaCalculator::Vector2DVectors ParabolaCalculator::GetResponse(const unsigned int& pointCount, const double& maxFrequency) const
{
	// Response equation from https://www.wildtronics.com/parabolicaccuracy.html#.YBQSbPtKg5k
	// I need to find some more sources to derive/verify this equation.  The link seems to have
	// an efficiency that varies with frequency and error, but they don't give this relationship.
	Vector2DVectors response(pointCount);
	const double minFrequency(GetMinAmplifiedFrequency());
	const double frequencyStep((maxFrequency - minFrequency) / (pointCount - 1));
	for (unsigned int i = 0; i < pointCount; ++i)
	{
		response[i](0) = minFrequency + i * frequencyStep;
		response[i](1) = 20.0 * log10(3.25 * parabolaInfo.diameter / speedOfSound * (response[i](0) * 2.0 * M_PI));
	}

	return response;
}

ParabolaCalculator::Vector2DVectors ParabolaCalculator::GetParabolaShape(const unsigned int& pointCount) const
{
	Vector2DVectors shape(pointCount);
	const double xStep(parabolaInfo.diameter * 0.5 / (pointCount - 1));
	shape.front() = Eigen::Vector2d::Zero();
	for (unsigned int i = 1; i < pointCount; ++i)
	{
		shape[i](0) = shape[i - 1](0) + xStep;
		shape[i](1) = shape[i](0) * shape[i](0) * 0.25 / parabolaInfo.focusPosition;
	}

	return shape;
}

ParabolaCalculator::Vector2DVectors ParabolaCalculator::GetFacetShape(const unsigned int& pointCount) const
{
	assert(pointCount % 2 == 0 && "Requires even number of points");

	const unsigned int halfPointCount(static_cast<unsigned int>(0.5 * pointCount));
	const double xStep(0.5 * parabolaInfo.diameter / (halfPointCount - 1));
	
	Vector2DVectors shape(pointCount);
	for (unsigned int i = 0; i < halfPointCount; ++i)
	{
		const double radius(i * xStep);
		shape[i](0) = ComputeParabolaArcLength(radius);
		shape[i](1) = M_PI * radius / parabolaInfo.facetCount;// Divide circumference at this radius by the number of facets, then take half of that value
		
		// Symmetric about the x-axis
		shape[pointCount - 1 - i](0) = shape[i](0);
		shape[pointCount - 1 - i](1) = -shape[i](1);
	}

	return shape;
}

double ParabolaCalculator::ComputeParabolaArcLength(const double& radius) const
{
	// Computed by:
	// integral of sqrt(1 + d/dx(parabola equation)) dx from 0 to radius
	const double w(0.5 * radius / parabolaInfo.focusPosition);
	const double s(sqrt(1 + w * w));
	return parabolaInfo.focusPosition * (w * s + log(w + s));
}
