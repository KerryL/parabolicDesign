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
	// TODO:  Calculate
	return -1.0;
}
	
ParabolaCalculator::Vector2DVectors ParabolaCalculator::GetResponse(const unsigned int& pointCount, const double& maxFrequency) const
{
	Vector2DVectors response(pointCount);
	for (unsigned int i = 0; i < pointCount; ++i)
	{
		response[i](0) = 1.0;
		response[i](1) = 0.0;
	}
	// TODO
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
	Vector2DVectors shape(pointCount);
	for (unsigned int i = 0; i < pointCount; ++i)
	{
		shape[i](0) = 0.0;
		shape[i](1) = 0.0;
	}
	// TODO
	return shape;
}
