#include <iipr/CDirectionalOptimalPathSolver.h>


// STL includes
#include <limits>


namespace iipr
{


enum NeighbourCode
{
	CD_TOP_LEFT,
	CD_TOP,
	CD_TOP_RIGHT,
	CD_LEFT,
	CD_RIGHT,
	CD_BOTTOM_LEFT,
	CD_BOTTOM,
	CD_BOTTOM_RIGHT,
	CD_CENTER,
	CD_LAST = CD_CENTER
};

// static const attributes
static const float s_invalidCostSum = -1;
static const float s_infinityCostSum = std::numeric_limits<float>::infinity();
static const istd::CIndex2d s_neighbourDiff[CD_LAST + 1] = {
			istd::CIndex2d(-1, -1),
			istd::CIndex2d(0, -1),
			istd::CIndex2d(1, -1),
			istd::CIndex2d(-1, 0),
			istd::CIndex2d(1, 0),
			istd::CIndex2d(-1, 1),
			istd::CIndex2d(0, 1),
			istd::CIndex2d(1, 1),
			istd::CIndex2d(0, 0)
};




// public methods

CDirectionalOptimalPathSolver::CDirectionalOptimalPathSolver()
{
	m_isInitialized = false;
	m_processedCostSum = 0;
}


bool CDirectionalOptimalPathSolver::Initialize(
			const i2d::CVector2d& beginPoint,
			const iimg::IBitmap& costOffsetBitmap,
			const iimg::IBitmap& costXBitmap,
			const iimg::IBitmap& costYBitmap,
			const iimg::CScanlineMask& costBitmapMask,
			double polarity)
{
	m_isInitialized = false;
	m_processedCostSum = 0;

	if (		(costOffsetBitmap.GetPixelFormat() != iimg::IBitmap::PF_FLOAT32) ||
				(costXBitmap.GetPixelFormat() != iimg::IBitmap::PF_FLOAT32) ||
				(costYBitmap.GetPixelFormat() != iimg::IBitmap::PF_FLOAT32)){
		return false;
	}

	i2d::CRect maskBoundingRect = costBitmapMask.GetBoundingRect();

	m_workMapPos = maskBoundingRect.GetLeftTop();
	istd::CIndex2d imageSize = maskBoundingRect.GetSize();
	if (		imageSize.IsSizeEmpty() ||
				!m_workMap.CreateBitmap(iimg::IBitmap::PF_USER, imageSize, sizeof(CostElement) * 8, 1)){
		return false;
	}

	istd::CIntRange lineRange(maskBoundingRect.GetLeft(), maskBoundingRect.GetRight());
	for (int y = 0; y < imageSize.GetY(); ++y){
		CostElement* costElementLinePtr = (CostElement*)m_workMap.GetLinePtr(y);
		Q_ASSERT(costElementLinePtr != nullptr);

		int x = 0;

		const istd::CIntRanges* outputRangesPtr = costBitmapMask.GetPixelRanges(y + maskBoundingRect.GetTop());
		if (outputRangesPtr != nullptr){
			const float* costOffsetBitmapLinePtr = (const float*)costOffsetBitmap.GetLinePtr(maskBoundingRect.GetTop() + y) + maskBoundingRect.GetLeft();
			Q_ASSERT(costOffsetBitmapLinePtr != nullptr);
			const float* costXBitmapLinePtr = (const float*)costXBitmap.GetLinePtr(maskBoundingRect.GetTop() + y) + maskBoundingRect.GetLeft();
			Q_ASSERT(costXBitmapLinePtr != nullptr);
			const float* costYBitmapLinePtr = (const float*)costYBitmap.GetLinePtr(maskBoundingRect.GetTop() + y) + maskBoundingRect.GetLeft();
			Q_ASSERT(costYBitmapLinePtr != nullptr);

			istd::CIntRanges::RangeList rangeList;
			outputRangesPtr->GetAsList(lineRange, rangeList);
			for (		istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter){
				istd::CIntRange rangeH = iter->GetTranslated(-maskBoundingRect.GetLeft());
				Q_ASSERT(rangeH.GetMinValue() >= 0);
				Q_ASSERT(rangeH.GetMaxValue() <= imageSize.GetX());

				for (; x < rangeH.GetMinValue(); ++x){
					CostElement& costElement = costElementLinePtr[x];
					costElement.costSum = s_invalidCostSum;
					costElement.invMask = 0;
				}

				for (; x < rangeH.GetMaxValue(); ++x){
					CostElement& costElement = costElementLinePtr[x];
					costElement.costSum = s_infinityCostSum;
					int mask = 0;

					if (x > rangeH.GetMinValue()){
						if (y > 0){
							mask |= (1 << CD_TOP_LEFT);
						}

						mask |= (1 << CD_LEFT);

						if (y < imageSize.GetY() - 1){
							mask |= (1 << CD_BOTTOM_LEFT);
						}
					}

					if (y > 0){
						mask |= (1 << CD_TOP);
					}

					if (y < imageSize.GetY() - 1){
						mask |= (1 << CD_BOTTOM);
					}

					if (x < rangeH.GetMaxValue() - 1){
						if (y > 0){
							mask |= (1 << CD_TOP_RIGHT);
						}

						mask |= (1 << CD_RIGHT);

						if (y < imageSize.GetY() - 1){
							mask |= (1 << CD_BOTTOM_RIGHT);
						}
					}

					costElement.invMask = qint8(mask);
					costElement.costOffset = costOffsetBitmapLinePtr[x] / 2;
					costElement.costX = float(costXBitmapLinePtr[x] * polarity / 2);
					costElement.costY = float(costYBitmapLinePtr[x] * polarity / 2);
				}
			}
		}

		for (; x < imageSize.GetX(); ++x){
			CostElement& costElement = costElementLinePtr[x];
			costElement.costSum = s_invalidCostSum;
			costElement.invMask = 0;
		}
	}

	m_startIndex = beginPoint.ToIndex2d() - m_workMapPos;
	if (		(m_startIndex.GetX() >= 0) &&
				(m_startIndex.GetX() < imageSize.GetX()) &&
				(m_startIndex.GetY() >= 0) &&
				(m_startIndex.GetY() < imageSize.GetY())){
		CostElement* directionLinePtr = (CostElement*)m_workMap.CGeneralBitmap::GetLinePtr(m_startIndex.GetY());
		CostElement& element = directionLinePtr[m_startIndex.GetX()];

		if (element.costSum != s_infinityCostSum){
			return false;
		}

		m_processingQueue.insert(ProcessingQueue::value_type(m_processedCostSum, m_startIndex));

		element.revDirCode = CD_CENTER;
		element.costSum = m_processedCostSum;
	}
	else{
		return false;
	}

	m_isInitialized = true;

	return true;
}


bool CDirectionalOptimalPathSolver::CalculateCostData(
			const QVector<i2d::CVector2d>& destPoints,
			double maxCostSum,
			int minDestPoints)
{
	if (!m_isInitialized){
		return false;
	}

	static const double sqrt2 = 1.414213562373095f;
	static const double diagFactor = 1 / sqrt2;
	static const Neighbourhood neighbourhoodTopLeft(sqrt2 / 2, -diagFactor, -diagFactor, CD_BOTTOM_RIGHT);
	static const Neighbourhood neighbourhoodTop(0.5, 0, -1, CD_BOTTOM);
	static const Neighbourhood neighbourhoodTopRight(sqrt2 / 2, diagFactor, -diagFactor, CD_BOTTOM_LEFT);
	static const Neighbourhood neighbourhoodLeft(0.5, -1, 0, CD_RIGHT);
	static const Neighbourhood neighbourhoodRight(0.5, 1, 0, CD_LEFT);
	static const Neighbourhood neighbourhoodBottomLeft(sqrt2 / 2, -diagFactor, diagFactor, CD_TOP_RIGHT);
	static const Neighbourhood neighbourhoodBottom(0.5, 0, 1, CD_TOP);
	static const Neighbourhood neighbourhoodBottomRight(sqrt2 / 2, diagFactor, diagFactor, CD_TOP_LEFT);

	istd::CIndex2d imageSize = m_workMap.GetImageSize();

	QSet<istd::CIndex2d> leftDestPoints;
	for (		QVector<i2d::CVector2d>::ConstIterator iter = destPoints.constBegin();
				iter != destPoints.constEnd();
				++iter){
		istd::CIndex2d destIndex = iter->ToIndex2d() - m_workMapPos;

		if (		(destIndex.GetX() >= 0) &&
					(destIndex.GetX() < imageSize.GetX()) &&
					(destIndex.GetY() >= 0) &&
					(destIndex.GetY() < imageSize.GetY())){
			CostElement* linePtr = (CostElement*)m_workMap.GetLinePtr(destIndex.GetY());
			CostElement& currentCostElement = linePtr[destIndex.GetX()];
			if (currentCostElement.costSum == s_invalidCostSum){
				return false;
			}

			if (currentCostElement.costSum > m_processedCostSum){
				leftDestPoints.insert(destIndex);
			}
		}
		else{
			return false;
		}
	}

	int minLeftPoints = (minDestPoints > 0)? destPoints.size() - minDestPoints: 0;	// number of destination points can remain uncalculated
	if (leftDestPoints.size() <= minLeftPoints){	// nothing to do, all points already found
		return true;
	}

	CostElement* resultBitmapData = (CostElement*)m_workMap.GetLinePtr(0);
	int resultBitmapLinesDiff = m_workMap.GetLinesDifference();

	float realMaxCostSum = (maxCostSum > 0)? float(maxCostSum): std::numeric_limits<float>::infinity();

	istd::CIndex2d nodeIndex;
	float costSum;
	while (PopNextElement(nodeIndex, costSum)){
		CostElement* resultMiddleLinePtr = (CostElement*)((quint8*)resultBitmapData + nodeIndex.GetY() * resultBitmapLinesDiff);
		CostElement& middleCostElement = resultMiddleLinePtr[nodeIndex.GetX()];
		if (middleCostElement.costSum != costSum){
			continue;
		}

		if (costSum < m_processedCostSum){
			m_isInitialized = false;

			return false;
		}

		if ((middleCostElement.invMask & ((1 << CD_TOP_LEFT) | (1 << CD_TOP) | (1 << CD_TOP_RIGHT))) != 0){
			Q_ASSERT(nodeIndex.GetY() >= 1);
			CostElement* resultPrevLinePtr = (CostElement*)((quint8*)resultMiddleLinePtr - resultBitmapLinesDiff);

			if ((middleCostElement.invMask & (1 << CD_TOP_LEFT)) != 0){
				Q_ASSERT(nodeIndex.GetX() >= 1);

				CostElement& resultValue = resultPrevLinePtr[nodeIndex.GetX() - 1];

				TryChangeNeighbour(
							middleCostElement,
							neighbourhoodTopLeft,
							istd::CIndex2d(nodeIndex.GetX() - 1, nodeIndex.GetY() - 1),
							resultValue);
			}

			if ((middleCostElement.invMask & (1 << CD_TOP)) != 0){
				CostElement& resultValue = resultPrevLinePtr[nodeIndex.GetX()];

				TryChangeNeighbour(
							middleCostElement,
							neighbourhoodTop,
							istd::CIndex2d(nodeIndex.GetX(), nodeIndex.GetY() - 1),
							resultValue);
			}

			if ((middleCostElement.invMask & (1 << CD_TOP_RIGHT)) != 0){
				Q_ASSERT(nodeIndex.GetX() < imageSize.GetX() - 1);

				CostElement& resultValue = resultPrevLinePtr[nodeIndex.GetX() + 1];

				TryChangeNeighbour(
							middleCostElement,
							neighbourhoodTopRight,
							istd::CIndex2d(nodeIndex.GetX() + 1, nodeIndex.GetY() - 1),
							resultValue);
			}
		}

		if ((middleCostElement.invMask & (1 << CD_LEFT)) != 0){
			Q_ASSERT(nodeIndex.GetX() >= 1);

			CostElement& resultValue = resultMiddleLinePtr[nodeIndex.GetX() - 1];

			TryChangeNeighbour(
						middleCostElement,
						neighbourhoodLeft,
						istd::CIndex2d(nodeIndex.GetX() - 1, nodeIndex.GetY()),
						resultValue);
		}

		if ((middleCostElement.invMask & (1 << CD_RIGHT)) != 0){
			Q_ASSERT(nodeIndex.GetX() < imageSize.GetX() - 1);

			CostElement& resultValue = resultMiddleLinePtr[nodeIndex.GetX() + 1];

			TryChangeNeighbour(
						middleCostElement,
						neighbourhoodRight,
						istd::CIndex2d(nodeIndex.GetX() + 1, nodeIndex.GetY()),
						resultValue);
		}

		if ((middleCostElement.invMask & ((1 << CD_BOTTOM_LEFT) | (1 << CD_BOTTOM) | (1 << CD_BOTTOM_RIGHT))) != 0){
			Q_ASSERT(nodeIndex.GetY() < imageSize.GetY() - 1);

			CostElement* resultNextLinePtr = (CostElement*)((quint8*)resultMiddleLinePtr + resultBitmapLinesDiff);

			if ((middleCostElement.invMask & (1 << CD_BOTTOM_LEFT)) != 0){
				Q_ASSERT(nodeIndex.GetX() >= 1);

				CostElement& resultValue = resultNextLinePtr[nodeIndex.GetX() - 1];

				TryChangeNeighbour(
							middleCostElement,
							neighbourhoodBottomLeft,
							istd::CIndex2d(nodeIndex.GetX() - 1, nodeIndex.GetY() + 1),
							resultValue);
			}

			if ((middleCostElement.invMask & (1 << CD_BOTTOM)) != 0){
				CostElement& resultValue = resultNextLinePtr[nodeIndex.GetX()];

				TryChangeNeighbour(
							middleCostElement,
							neighbourhoodBottom,
							istd::CIndex2d(nodeIndex.GetX(), nodeIndex.GetY() + 1),
							resultValue);
			}

			if ((middleCostElement.invMask & (1 << CD_BOTTOM_RIGHT)) != 0){
				Q_ASSERT(nodeIndex.GetX() < imageSize.GetX() - 1);

				CostElement& resultValue = resultNextLinePtr[nodeIndex.GetX() + 1];

				TryChangeNeighbour(
							middleCostElement,
							neighbourhoodBottomRight,
							istd::CIndex2d(nodeIndex.GetX() + 1, nodeIndex.GetY() + 1),
							resultValue);
			}
		}

		m_processedCostSum = costSum;

		if (leftDestPoints.remove(nodeIndex) && (leftDestPoints.size() <= minLeftPoints)){
			return true;
		}

		if (costSum > realMaxCostSum){
			return true;
		}
	}

	return false;
}


double CDirectionalOptimalPathSolver::GetOptimalPath(const i2d::CVector2d& destPoint, i2d::CPolypoint* optimalPathPtr) const
{
	istd::CIndex2d destIndex = destPoint.ToIndex2d() - m_workMapPos;

	double retVal = s_invalidCostSum;

	istd::CIndex2d imageSize = m_workMap.GetImageSize();

	if (		(destIndex.GetX() >= 0) &&
				(destIndex.GetX() < imageSize.GetX()) &&
				(destIndex.GetY() >= 0) &&
				(destIndex.GetY() < imageSize.GetY())){
		const CostElement* linePtr = (const CostElement*)m_workMap.GetLinePtr(destIndex.GetY());
		retVal = linePtr[destIndex.GetX()].costSum;

		if (optimalPathPtr != nullptr){
			istd::CIndex2d nodeIndex = destIndex;
			while (nodeIndex != m_startIndex){
				if (nodeIndex != destIndex){
					istd::CIndex2d originalIndex = nodeIndex + m_workMapPos;

					optimalPathPtr->InsertNode(i2d::CVector2d(originalIndex.GetX() + 0.5, originalIndex.GetY() + 0.5));
				}

				const CostElement* directionLinePtr = (const CostElement*)m_workMap.GetLinePtr(nodeIndex.GetY());
				NeighbourCode revDirCode = NeighbourCode(directionLinePtr[nodeIndex.GetX()].revDirCode);
				Q_ASSERT(revDirCode >= 0);
				Q_ASSERT(revDirCode <= CD_LAST);

				if (revDirCode == CD_CENTER){
					break;
				}

				istd::CIndex2d nextNodeIndex = nodeIndex + s_neighbourDiff[revDirCode];
				Q_ASSERT(nextNodeIndex != nodeIndex);

				nodeIndex = nextNodeIndex;
			}
		}
	}

	return retVal;
}


} // namespace iipr


