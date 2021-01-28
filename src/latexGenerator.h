// File:  latexGenerator.h
// Date:  12/16/2019
// Auth:  K. Loux
// Desc:  LaTeX source file generator.

#ifndef LATEX_GENERATOR_H_
#define LATEX_GENERATOR_H_

// Eigen headers
#include <Eigen/Eigen>

// Standard C++ headers
#include <vector>
#include <string>

class LaTeXGenerator
{
public:
	typedef std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>> Vector2DVectors;

	inline void SetMargin(const double& m) { margin = m; }
	inline void SetOverlap(const double& o) { overlap = o; }
	inline void SetPageSize(const double& w, const double& h) { pageWidth = w; pageHeight = h; }

	bool WriteFlatPatterns(const Vector2DVectors& shape, const std::string& fileName);

private:
	struct PageOffset
	{
		PageOffset() = default;
		PageOffset(const double& _x, const double& _y) : x(_x), y(_y) {}

		double x;// [in]
		double y;// [in]
	};

	std::string BuildFlatPatternTeX(const Vector2DVectors& pattern);

	void DeterminePageCount(const Vector2DVectors& pattern, std::vector<PageOffset>& offsets) const;

	std::string GenerateHeaderInfo() const;
	std::string GeneratePath(const Vector2DVectors& path, const PageOffset& offset, unsigned int& pointsOnPage, const bool& cycle = false) const;
	
	static Vector2DVectors ShiftToZeroXandY(const Vector2DVectors& pattern);

	static std::string GetBeginPictureString(const PageOffset& offset);
	static const std::string endPictureString;

	double margin = 0.5;// [in]
	double overlap = 0.75;// [in]

	double pageWidth = 17.0;// [in]
	double pageHeight = 11.0;// [in]
	
	enum class Position
	{
		Beginning,
		Middle,
		End
	};

	std::string GenerateScale() const;

	double DetermineIdealRotationAngle(const Vector2DVectors& pattern) const;
	static Vector2DVectors RotatePattern(const Vector2DVectors& pattern, const double& angle);

	std::string GeneratePageMatrix(const std::vector<PageOffset>& offsets, const PageOffset& currentOffset) const;
	std::string GenerateAlignmentMarks() const;

	enum class MarkRotation
	{
		Normal,
		Rotated
	};

	std::string GenerateAlignmentMark(const PageOffset& center, const MarkRotation& rotation, const double& halfSizeMM) const;
	
	Eigen::Vector2d GetBoundaryIntersection(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, const PageOffset& offset) const;
	Vector2DVectors GetBoundaryIntersections(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, const PageOffset& offset, const unsigned int& expectedIsectCount) const;
	bool PointsCrossPage(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, const PageOffset& offset, Eigen::Vector2d& isect1, Eigen::Vector2d& isect2) const;
	static Eigen::Vector2d FindIntersection(const Eigen::Vector2d& p1, const Eigen::Vector2d& dir1, const Eigen::Vector2d& p2, const Eigen::Vector2d& dir2);
	static double SolveForT(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, const Eigen::Vector2d& p3);
};

#endif// LATEX_GENERATOR_H_
