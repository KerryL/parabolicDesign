/*===================================================================================
                                 Parabolic Design App
                          Copyright Kerry R. Loux 2021

                   This code is licensed under the GPLv2 License
                     (http://opensource.org/licenses/GPL-2.0).

===================================================================================*/

// File:  parabolaCalculator.h
// Date:  1/22/2021
// Auth:  K. Loux
// Desc:  Calculations for parabolic reflectors.

#ifndef PARABOLA_CALCULATOR_H_
#define PARABOLA_CALCULATOR_H_

// Eigen headers
#include <Eigen/Eigen>

// Standard C++ headers
#include <vector>

class ParabolaCalculator
{
public:
	struct ParabolaInfo
	{
		double diameter = 24.0;// [in]
		double focusPosition = 6.0;// [in]
		unsigned int facetCount = 10;
	};
	
	void SetParabolaInfo(const ParabolaInfo& info) { parabolaInfo = info; }
	
	double GetMinAmplifiedFrequency() const;
	double GetParabolaDepth() const;
	double GetMaxDesignError() const;
	
	typedef std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>> Vector2DVectors;
	
	Vector2DVectors GetResponse(const unsigned int& pointCount, const double& maxFrequency) const;
	Vector2DVectors GetParabolaShape(const unsigned int& pointCount) const;
	Vector2DVectors GetFacetShape(const unsigned int& pointCount) const;

private:
	static const double speedOfSound;// [in/sec]
	
	ParabolaInfo parabolaInfo;
	
	double ComputeParabolaArcLength(const double& radius) const;
};

#endif// PARABOLA_CALCULATOR_H_
