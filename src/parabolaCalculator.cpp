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

// Removed after updating gain plot, which shows there may be some minimial amplification at low
// frequencies.  Better not to state this explicitly and allow the user to see the effect on the
// gain plot.
/*double ParabolaCalculator::GetMinAmplifiedFrequency() const
{
	// Rayleigh criterion?  It's unclear how it would apply to sound waves, but I see it mentioned in various internet articles.
	// For now, it's the simple "how large a wave can fit in the diameter?" calculation.
	return speedOfSound / parabolaInfo.diameter;// [Hz]
}*/

double ParabolaCalculator::GetParabolaDepth() const
{
	return parabolaInfo.diameter * parabolaInfo.diameter * 0.0625 / parabolaInfo.focusPosition;
}

double ParabolaCalculator::GetMaxDesignError() const
{
	// The design creates the desired parabola along the center of each facet.  So the
	// largest error will be at the widest part of the parabola, were two facets join.
	// We'll report the error in a plane perpendicular to the axis of the parabola.
	
	const double halfFacetWidth(0.5 * M_PI * parabolaInfo.diameter / parabolaInfo.facetCount);
	const double jointDistance(sqrt(parabolaInfo.diameter * parabolaInfo.diameter * 0.25 + halfFacetWidth * halfFacetWidth));

	return jointDistance - 0.5 * parabolaInfo.diameter;
}
	
ParabolaCalculator::Vector2DVectors ParabolaCalculator::GetResponse(const unsigned int& pointCount, const double& maxFrequency) const
{
	// Current implementation of the response calculation is based on the equations given here:
	// http://www.dzwiekinatury.pl/upload/files/strony/4/the_parabolic_reflector_sten_wahlstr%C3%B6m.pdf
	// This paper has by far the most robust explanation of the calculations for any source I have found so far.
	// The gain calculated here is for on-axis sounds.
	// In this paper:
	// - a          = focus position (measured from parabola apex; consistent with our convention)
	// - lambda     = wavelength
	// - l          = parabola depth
	// - l / a      = depth-to-focus ratio
	// - a / lambda = focus-to-wavelength ratio
	
	// Other sources that were tried and rejcted:
	// https://www.wildtronics.com/parabolicaccuracy.html#.YBQSbPtKg5k
	// https://www.electronics-notes.com/articles/antennas-propagation/parabolic-reflector-antenna/antenna-gain-directivity.php
	
	Vector2DVectors response(pointCount);
	const double minFrequency(100.0);// [Hz] something small enough to show the low-frequency response without making the x-axis scaling unnecessarily tight
	const double frequencyStep((maxFrequency - minFrequency) / (pointCount - 1));
	const double depthToFocusRatio(GetParabolaDepth() / parabolaInfo.focusPosition);// [-]
	for (unsigned int i = 0; i < pointCount; ++i)
	{
		response[i](0) = minFrequency + i * frequencyStep;
		const double wavelength(speedOfSound / response[i](0));// [in]
		const double focusToWavelengthRatio(parabolaInfo.focusPosition / wavelength);// [-]
		const double b(log(1.0 + depthToFocusRatio));// [-] helper variable to clean up the below expression
		const double pressureFactor(sqrt(1.0 + pow(4.0 * M_PI * focusToWavelengthRatio * b, 2) + 8.0 * M_PI * focusToWavelengthRatio * b * sin(4.0 * M_PI * focusToWavelengthRatio)));// [-]
		response[i](1) = 20.0 * log10(pressureFactor);
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
