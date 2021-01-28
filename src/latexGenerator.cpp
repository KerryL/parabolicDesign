// File:  latexGenerator.cpp
// Date:  12/16/2019
// Auth:  K. Loux
// Desc:  LaTeX source file generator.

// Local headers
#include "latexGenerator.h"

// Standard C++ headers
#include <fstream>
#include <sstream>

const std::string LaTeXGenerator::endPictureString("    \\end{tikzpicture}\n  };\n\\end{tikzpicture}\n\n");

std::string LaTeXGenerator::GetBeginPictureString(const PageOffset& offset)
{
	std::ostringstream ss;
	ss << "\\begin{tikzpicture}[remember picture, overlay]\n"
		<< "\\node [xshift=" << offset.x << "in,yshift=" << offset.y << "in] at (current page.south west){"
		<< "\n    \\begin{tikzpicture}[remember picture, overlay]\n";
	return ss.str();
}

bool LaTeXGenerator::WriteFlatPatterns(const Vector2DVectors& shape, const std::string& fileName)
{
	std::ofstream file(fileName);
	if (!file.is_open())
		return false;

	const double shapeRotation(DetermineIdealRotationAngle(shape));
	const auto shapeRotated(ShiftToZeroXandY(RotatePattern(shape, shapeRotation)));

	std::vector<PageOffset> offsets;
	DeterminePageCount(shapeRotated, offsets);
	
	file << GenerateHeaderInfo();
	file << BuildFlatPatternTeX(shapeRotated);
	file << "\\end{document}\n";
	
	return true;
}

std::string LaTeXGenerator::GenerateHeaderInfo() const
{
	std::ostringstream ss;
	ss << "\\documentclass{article}\n\n"
		<< "\\usepackage{tikz}\n"
		<< "\\usepackage[margin=" << margin << "in,paperwidth=" << pageWidth << "in,paperheight=" << pageHeight << "in]{geometry}\n\n"
		<< "\\begin{document}\n\n"
		<< "\\tikzset\n"
		<< "{\n"
		<< "  x=1mm,\n"
		<< "  y=1mm\n"
		<< "}\n\n";
	return ss.str();
}

std::string LaTeXGenerator::GeneratePath(const Vector2DVectors& path, const PageOffset& offset, unsigned int& pointsOnPage, const bool& cycle) const
{
	const double availableWidth(pageWidth - 2.0 * margin);// [in]
	const double availableHeight(pageHeight - 2.0 * margin);// [in]

	const double minX(offset.x * 25.4);// [mm]
	const double maxX(minX + availableWidth * 25.4);// [mm]
	const double minY(offset.y * 25.4);// [mm]
	const double maxY(minY + availableHeight * 25.4);// [mm]
	
	auto isOnPage([&minX, &minY, &maxX, &maxY](const Eigen::Vector2d& p)
	{
		if (p(0) <= minX || p(0) >= maxX || p(1) <= minY || p(1) >= maxY)
			return false;
		return true;
	});

	PageOffset offsetMM((offset.x - margin) * 25.4, (offset.y - margin) * 25.4);
	
	std::ostringstream ss;
	ss << "% Pattern path\n";
	bool restart(true);
	bool lastPValid(false);
	Eigen::Vector2d lastP;
	pointsOnPage = 0;
	for (const auto& p : path)
	{
		if (restart)
		{
			if (!isOnPage(p))
			{
				if (lastPValid)
				{
					Eigen::Vector2d isect1, isect2;
					if (PointsCrossPage(lastP, p, offset, isect1, isect2))
					{
						ss << "\\draw (" << isect1(0) - offsetMM.x << ',' << isect1(1) - offsetMM.y << ") -- (" << isect2(0) - offsetMM.x << ',' << isect2(1) - offsetMM.y << ");\n\n";
						++pointsOnPage;
					}
				}
				lastP = p;
				lastPValid = true;
				continue;
			}
			
			restart = false;
			if (lastPValid)
			{
				const auto intersection(GetBoundaryIntersection(lastP, p, offset));
				ss << "\\draw (" << intersection(0) - offsetMM.x << ',' << intersection(1) - offsetMM.y << ") -- (" << p(0) - offsetMM.x << ',' << p(1) - offsetMM.y << ")";
				++pointsOnPage;
			}
			else
			{
				ss << "\\draw (" << p(0) - offsetMM.x << ',' << p(1) - offsetMM.y << ")";
				++pointsOnPage;
			}
		}
		else if (!isOnPage(p))// Previous point was on the page
		{
			const auto intersection(GetBoundaryIntersection(lastP, p, offset));
			ss << " -- (" << intersection(0) - offsetMM.x << ',' << intersection(1) - offsetMM.y << ");\n\n";
			restart = true;
		}
		else// This one and last one are on the page
		{
			ss << " -- (" << p(0) - offsetMM.x << ',' << p(1) - offsetMM.y << ")";
			++pointsOnPage;
		}

		lastP = p;
		lastPValid = true;
	}

	if (cycle && isOnPage(path.front()))
	{
		if (restart)
		{
			const auto intersection(GetBoundaryIntersection(lastP, path.front(), offset));
			ss << " -- (" << intersection(0) - offsetMM.x << ',' << intersection(1) - offsetMM.y << ");\n\n";
		}
		else
			ss << " -- (" << path.front()(0) - offsetMM.x << ',' << path.front()(1) - offsetMM.y << ")";
	}
	
	if (!ss.str().empty() && !restart)// TODO:  Check should be "last non-whitespace character is not ';'"
		ss << ";\n\n";

	return ss.str();
}

LaTeXGenerator::Vector2DVectors LaTeXGenerator::ShiftToZeroXandY(const Vector2DVectors& pattern)
{
	double minX(std::numeric_limits<double>::max()), minY(std::numeric_limits<double>::max());

	for (const auto & p : pattern)
	{
		if (p(0) < minX)
			minX = p(0);
		if (p(1) < minY)
			minY = p(1);
	}
	
	Eigen::Vector2d shift(minX, minY);
	
	Vector2DVectors shifted(pattern);
	for (auto& p : shifted)
		p -= shift;
	
	return shifted;
}

void LaTeXGenerator::DeterminePageCount(const Vector2DVectors& pattern, std::vector<PageOffset>& offsets) const
{
	double maxX(std::numeric_limits<double>::min()), maxY(std::numeric_limits<double>::min());

	for (const auto & p : pattern)
	{
		assert(p(0) >= 0.0);
		assert(p(1) >= 0.0);

		if (p(0) > maxX)
			maxX = p(0);
		if (p(1) > maxY)
			maxY = p(1);
	}
	
	maxX /= 25.4;
	maxY /= 25.4;

	auto countPages([this](const double& paperDim, const double& patternDim)
	{
		const auto count(static_cast<unsigned int>(ceil((patternDim - paperDim + 2.0 * margin) / (paperDim - 2.0 * margin - overlap)) + 1));
		assert(count > 0);
		return count;
	});

	const unsigned int xPages(countPages(pageWidth, maxX));
	const unsigned int yPages(countPages(pageHeight, maxY));

	const double baseXOffset(xPages * yPages == 1 ? 0.5 * (maxX - pageWidth) + margin : 0.0);
	const double baseYOffset(xPages * yPages == 1 ? 0.5 * (maxY - pageHeight) + margin : 0.0);

	const double availableWidth(pageWidth - 2.0 * margin);
	const double availableHeight(pageHeight - 2.0 * margin);

	offsets.resize(xPages * yPages);
	for (unsigned int x = 0; x < xPages; ++x)
	{
		for (unsigned int y = 0; y < yPages; ++y)
		{
			// The first page (x = 0, y = 0) will have global (0,0) at it's lower LH corner.
			// Offsets are always equal to the location of the lower LH corner of the page with respect to global (0,0).
			offsets[x * yPages + y].x = baseXOffset + x * (availableWidth - overlap);
			offsets[x * yPages + y].y = baseYOffset + y * (availableHeight - overlap);
		}
	}
}

std::string LaTeXGenerator::BuildFlatPatternTeX(const Vector2DVectors& pattern)
{
	std::ostringstream ss;

	std::vector<PageOffset> offsets;
	DeterminePageCount(pattern, offsets);
	
	bool firstPage(true);
	for (const auto& o : offsets)
	{
		std::ostringstream pageSS;
		pageSS << "\\newpage\n"
			<< "\\thispagestyle{empty}\n\n";

		bool handlingFirstPage(false);
		if (firstPage)
		{
			pageSS << GenerateScale();
			firstPage = false;
			handlingFirstPage = true;
		}
		
		pageSS << GetBeginPictureString(PageOffset(0.0, 0.0));
		unsigned int pointCount;
		pageSS << GeneratePath(pattern, o, pointCount, true);
		if (pointCount == 0)
		{
			if (handlingFirstPage)
				firstPage = true;
			continue;// Don't add blank pages
		}
		pageSS << endPictureString;

		if (offsets.size() > 1)
		{
			pageSS << GenerateAlignmentMarks();
			pageSS << GeneratePageMatrix(offsets, o);
		}

		ss << pageSS.str();
	}

	return ss.str();
}

std::string LaTeXGenerator::GenerateScale() const
{
	std::ostringstream ss;
	ss << "% Scale mark\n";
	ss << GetBeginPictureString(PageOffset(margin, 2 * overlap));
	ss << "\\draw (12.7,0) -- (0,0) -- (0,12.7) -- (12.7,12.7) -- (12.7,25.4) -- (0,25.4) -- (0,31.75) -- (6.35,31.75) -- (6.35,38.1) -- (0,38.1);\n";
	ss << endPictureString;
	
	return ss.str();
}

double LaTeXGenerator::DetermineIdealRotationAngle(const Vector2DVectors& pattern) const
{
	std::vector<PageOffset> offsets;
	DeterminePageCount(ShiftToZeroXandY(pattern), offsets);
	unsigned int minPages(offsets.size());
	double smallestAngle(0.0);// [deg]

	// Prefer a 0 or 90 deg rotation
	{
		const auto rotated(ShiftToZeroXandY(RotatePattern(pattern, 90.0)));
		DeterminePageCount(rotated, offsets);
		if (offsets.size() < minPages)
		{
			minPages = offsets.size();
			smallestAngle = 90.0;
		}
	}

	const double step(1.0);// [deg]
	for (double angle = step; angle < 360.0; angle += step)
	{
		const auto rotated(ShiftToZeroXandY(RotatePattern(pattern, angle)));
		DeterminePageCount(rotated, offsets);
		if (offsets.size() < minPages)
		{
			minPages = offsets.size();
			smallestAngle = angle;
		}
	}

	return smallestAngle;
}

LaTeXGenerator::Vector2DVectors LaTeXGenerator::RotatePattern(const Vector2DVectors& pattern, const double& angle)
{
	const auto rotation(Eigen::Rotation2D<double>(angle * M_PI / 180.0));
	Vector2DVectors rotated(pattern.size());
	for (unsigned int i = 0; i < pattern.size(); ++i)
		rotated[i] = rotation * pattern[i];

	return rotated;
}

std::string LaTeXGenerator::GenerateAlignmentMarks() const
{
	const double edgeOffset(margin + 0.5 * overlap);
	const PageOffset bottomLeft(edgeOffset, edgeOffset);
	const PageOffset bottomRight(pageWidth - edgeOffset, edgeOffset);
	const PageOffset topLeft(edgeOffset, pageHeight - edgeOffset);
	const PageOffset topRight(pageWidth - edgeOffset, pageHeight - edgeOffset);

	const double markSize(0.3);// [in]
	std::ostringstream ss;
	ss << "% Alignment marks\n";
	ss << GenerateAlignmentMark(bottomLeft, MarkRotation::Normal, 0.5 * markSize * 25.4);
	ss << GenerateAlignmentMark(bottomRight, MarkRotation::Rotated, 0.5 * markSize * 25.4);
	ss << GenerateAlignmentMark(topLeft, MarkRotation::Rotated, 0.5 * markSize * 25.4);
	ss << GenerateAlignmentMark(topRight, MarkRotation::Normal, 0.5 * markSize * 25.4);
	
	return ss.str();
}

std::string LaTeXGenerator::GenerateAlignmentMark(const PageOffset& center, const MarkRotation& rotation, const double& halfSizeMM) const
{
	PageOffset offset(center);
	offset.x -= halfSizeMM / 25.4;
	offset.y -= halfSizeMM / 25.4;

	std::ostringstream ss;
	ss << GetBeginPictureString(offset);
	ss << "  \\tikz[radius=" << halfSizeMM << "mm] {\n";

	if (rotation == MarkRotation::Normal)
		ss << "    \\fill (0,0) -- ++ (" << halfSizeMM << "mm,0) arc [start angle=0, end angle=90] -- ++ (0,-" << 2.0 * halfSizeMM << "mm) arc [start angle=270, end angle=180];\n";
	else
		ss << "    \\fill (0,0) -- ++ (0," << halfSizeMM << "mm) arc [start angle=90, end angle=180] -- ++ (" << 2.0 * halfSizeMM << "mm,0) arc [start angle=0, end angle=-90];\n";

	ss << "    \\draw (0,0) circle;\n  }\n";
	ss << endPictureString;

	return ss.str();
}

std::string LaTeXGenerator::GeneratePageMatrix(const std::vector<PageOffset>& offsets, const PageOffset& currentOffset) const
{
	if (offsets.size() < 2)
		return std::string();

	PageOffset offset(margin + overlap, margin);// TODO:  Position such that the pattern cannot overlap

	double maxX(0.0);// [in]
	double maxY(0.0);// [in]
	std::vector<double> exes, wyes;
	for (const auto& o : offsets)
	{
		exes.push_back(o.x);
		wyes.push_back(o.y);

		if (o.x > maxX)
			maxX = o.x;
		if (o.y > maxY)
			maxY = o.y;
	}

	auto getSpacing([](std::vector<double>& values)
	{
		std::sort(values.begin(), values.end());
		values.erase(std::unique(values.begin(), values.end()), values.end());
		if (values.size() > 1)
			return values[1] - values[0];
		return 0.0;
	});

	double deltaX(getSpacing(exes));
	double deltaY(getSpacing(wyes));

	if (deltaX == 0.0)
	{
		assert(deltaY > 0.0);
		deltaX = deltaY * pageWidth / pageHeight;
	}
	if (deltaY == 0.0)
	{
		assert(deltaX > 0.0);
		deltaY = deltaX * pageHeight / pageWidth;
	}

	maxX += deltaX;// [in]
	maxY += deltaY;// [in]
	maxX *= 25.4;// now [mm]
	maxY *= 25.4;// now [mm]

	deltaX *= 25.4;// now [mm]
	deltaY *= 25.4;// now [mm]

	double scale;
	const double largestMatrixDimension(overlap * 25.4);// [mm] - dimension chosen as overlap to ensure this only appers within the overlap region
	if (maxX > maxY)
		scale = largestMatrixDimension / maxX;
	else
		scale = largestMatrixDimension / maxY;

	std::ostringstream ss;
	ss << "% Page arrangement matrix\n";
	ss << GetBeginPictureString(offset);
	ss << "  \\draw[xstep=" << deltaX * scale << ",ystep=" << deltaY * scale << ",very thin] (0,0) grid (" << maxX * scale << ',' << maxY * scale << ");\n";
	ss << "  \\fill (" << currentOffset.x * 25.4 * scale << ',' << currentOffset.y * 25.4 * scale << ") rectangle ("
		<< (currentOffset.x * 25.4 + deltaX) * scale << ',' << (currentOffset.y * 25.4 + deltaY) * scale << ");\n";
	ss << endPictureString;

	return ss.str();
}

LaTeXGenerator::Vector2DVectors LaTeXGenerator::GetBoundaryIntersections(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2,
	const PageOffset& offset, const unsigned int& expectedIsectCount) const
{
	const double availableWidth(pageWidth - 2.0 * margin);// [in]
	const double availableHeight(pageHeight - 2.0 * margin);// [in]

	const Eigen::Vector2d direction(p2 - p1);
	const Eigen::Vector2d up(0.0, 1.0);
	const Eigen::Vector2d right(1.0, 0.0);
	const Eigen::Vector2d lowerLeft(offset.x * 25.4, offset.y * 25.4);
	const Eigen::Vector2d lowerRight(lowerLeft + right * availableWidth * 25.4);
	const Eigen::Vector2d upperLeft(lowerLeft + up * availableHeight * 25.4);

	Vector2DVectors intersections;
	
	// First check the case of the direction being along a boundary.
	auto cross2DNorm([](const Eigen::Vector2d& v1, const Eigen::Vector2d& v2)
	{
		return fabs(v1(0) * v2(1) - v1(1) * v2(0));
	});
	
	// TODO:  This is suspect.  Need more robust method.
	const double epsilon(1.0e-10);
	if (cross2DNorm(direction, right) < epsilon)// direction is (nearly) parallel with x-axis
	{
		if (fabs(p1(1) - lowerLeft(1)) < epsilon || fabs(p1(1) - upperLeft(1)) < epsilon)// y-ordinate matches top or bottom
		{
			if (p1(0) >= lowerLeft(0) && p1(0) <= lowerRight(0))
				intersections.push_back(p1);
			if (p2(0) >= lowerLeft(0) && p2(0) <= lowerRight(0))
				intersections.push_back(p2);
		}
	}
	else if (cross2DNorm(direction, up) < epsilon)// direction is (nearly) parallel with y-axis
	{
		if (fabs(p1(0) - lowerLeft(0)) < epsilon || fabs(p1(0) - lowerRight(0)) < epsilon)// x-ordinate matches left or right
		{
			if (p1(1) >= lowerLeft(1) && p1(1) <= upperLeft(1))
				intersections.push_back(p1);
			if (p2(1) >= lowerLeft(0) && p2(1) <= upperLeft(1))
				intersections.push_back(p2);
		}
	}
	
	if (intersections.size() == 2)
		return intersections;

	// In case of rounding causing a valid result to be rejected, we store rejected results and append
	// the best matches to the result vector.
	std::vector<std::pair<double, Eigen::Vector2d>> isectCandidates;

	auto computeTError([](const double& t)
	{
		if (t < 0.0)
			return -t;
		else if (t > 1.0)
			return t - 1.0;
		return 0.0;
	});

	auto worstTError([&computeTError](const double& t1, const double& t2)
	{
		const double t1Error(computeTError(t1));
		const double t2Error(computeTError(t2));
		assert(t1Error >= 0.0 && t2Error >= 0.0);
		assert(t1Error > 0.0 || t2Error > 0.0);// Otherwise, why is this called?
		if (t1Error > t2Error)
			return t1Error;
		return t2Error;
	});

	// Now check for one intersection with each axis.
	// Method is to find intersections, then solve for t in equation P_line = p1 + direction * t.
	// If t is not between 0.0 and 1.0, reject (this check needs to be done for both line segments)
	if (fabs(direction.dot(right)) > epsilon)
	{
		const auto isectLeft(FindIntersection(p1, direction, lowerLeft, up));
		const auto tLeftPoints(SolveForT(p1, p2, isectLeft));
		const auto tLeftBorder(SolveForT(lowerLeft, upperLeft, isectLeft));
		if (tLeftPoints >= 0.0 && tLeftPoints <= 1.0 && tLeftBorder >= 0.0 && tLeftBorder <= 1.0)
			intersections.push_back(isectLeft);
		else
			isectCandidates.push_back(std::make_pair(worstTError(tLeftPoints, tLeftBorder), isectLeft));

		const auto isectRight(FindIntersection(p1, direction, lowerRight, up));
		const auto tRightPoints(SolveForT(p1, p2, isectRight));
		const auto tRightBorder(SolveForT(lowerRight, lowerRight + up * availableHeight * 25.4, isectRight));
		if (tRightPoints >= 0.0 && tRightPoints <= 1.0 && tRightBorder >= 0.0 && tRightBorder <= 1.0)
			intersections.push_back(isectRight);
		else
			isectCandidates.push_back(std::make_pair(worstTError(tRightPoints, tRightBorder), isectRight));
	}
	
	if (fabs(direction.dot(up)) > epsilon)
	{
		const auto isectBottom(FindIntersection(p1, direction, lowerLeft, right));
		const auto tBottomPoints(SolveForT(p1, p2, isectBottom));
		const auto tBottomBorder(SolveForT(lowerLeft, lowerRight, isectBottom));
		if (tBottomPoints >= 0.0 && tBottomPoints <= 1.0 && tBottomBorder >= 0.0 && tBottomBorder <= 1.0)
			intersections.push_back(isectBottom);
		else
			isectCandidates.push_back(std::make_pair(worstTError(tBottomPoints, tBottomBorder), isectBottom));

		const auto isectTop(FindIntersection(p1, direction, upperLeft, right));
		const auto tTopPoints(SolveForT(p1, p2, isectTop));
		const auto tTopBorder(SolveForT(upperLeft, upperLeft + right * availableWidth * 25.4, isectTop));
		if (tTopPoints >= 0.0 && tTopPoints <= 1.0 && tTopBorder >= 0.0 && tTopBorder <= 1.0)
			intersections.push_back(isectTop);
		else
			isectCandidates.push_back(std::make_pair(worstTError(tTopPoints, tTopBorder), isectTop));
	}

	if (intersections.size() < expectedIsectCount)
	{
		std::sort(isectCandidates.begin(), isectCandidates.end(), [](const std::pair<double, Eigen::Vector2d>& a, const std::pair<double, Eigen::Vector2d>& b)
		{
			return a.first < b.first;
		});

		while (!isectCandidates.empty() && intersections.size() < expectedIsectCount)
		{
			// Still keep a sanity check here
			assert(isectCandidates.front().first < epsilon);

			intersections.push_back(isectCandidates.front().second);
			isectCandidates.erase(isectCandidates.begin());
		}
	}
	
	return intersections;
}

Eigen::Vector2d LaTeXGenerator::GetBoundaryIntersection(const Eigen::Vector2d& p1,
	const Eigen::Vector2d& p2, const PageOffset& offset) const
{
	const auto isects(GetBoundaryIntersections(p1, p2, offset, 1));
	assert(isects.size() == 1);
	return isects.front();
}

Eigen::Vector2d LaTeXGenerator::FindIntersection(const Eigen::Vector2d& p1,
	const Eigen::Vector2d& dir1, const Eigen::Vector2d& p2, const Eigen::Vector2d& dir2)
{
	const double t2((p1(0) * dir1(1) + dir1(0) * (p2(1) - p1(1)) - p2(0) * dir1(1)) / (dir2(0) * dir1(1) - dir1(0) * dir2(1)));
	return p2 + dir2 * t2;
}

bool LaTeXGenerator::PointsCrossPage(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2,
	const PageOffset& offset, Eigen::Vector2d& isect1, Eigen::Vector2d& isect2) const
{
	const auto isects(GetBoundaryIntersections(p1, p2, offset, 0));// Could expect 0 or 2; choose lowest value
	if (isects.empty())
		return false;

	assert(isects.size() == 2);
	isect1 = isects[0];
	isect2 = isects[1];
	return true;
}

double LaTeXGenerator::SolveForT(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, const Eigen::Vector2d& p3)
{
	const Eigen::Vector2d dir(p2 - p1);
	
	// We assume that p1, p2 and p3 are colinear here
	Eigen::Vector3d v1, v2; v1.head<2>() = p2 - p1; v1(2) = 0.0; v2.head<2>() = p3 - p1; v2(2) = 0.0;
	assert(fabs(v1.cross(v2).norm()) < 1.0e-6);
	
	// p3 = p1 + (p2 - p1) * t
	// t = (p3 - p1) / (p2 - p1)
	// Chose path to ensure numerical stability
	if (fabs(dir(0)) > fabs(dir(1)))
		return (p3(0) - p1(0)) / dir(0);
	return (p3(1) - p1(1)) / dir(1);
}
