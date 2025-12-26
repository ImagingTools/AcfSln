#ifndef iipr_CDirectionalOptimalPathSolver_included
#define iipr_CDirectionalOptimalPathSolver_included


// STL includes
#include <map>

// ACF includes
#include <istd/CIndex2d.h>
#include <iimg/CScanlineMask.h>
#include <iimg/CGeneralBitmap.h>


#ifdef __clang__
	#define __forceinline __attribute__((always_inline))
#elif defined(__GNUC__) && !defined(__forceinline)
	#define __forceinline inline
#endif


namespace iipr
{


class CDirectionalOptimalPathSolver
{
public:
	CDirectionalOptimalPathSolver();

	bool Initialize(
				const i2d::CVector2d& beginPoint,
				const iimg::IBitmap& costOffsetBitmap,
				const iimg::IBitmap& costXBitmap,
				const iimg::IBitmap& costYBitmap,
				const iimg::CScanlineMask& costBitmapMask,
				double dirFactor);

	bool IsInitialized() const;

	/**
		Calculates bitmap containing cost from begin point to each pixel (it stops when cost to all destination points is calculated.
		The output bitmap will be created as using elements of type float.
	*/
	bool CalculateCostData(
				const QVector<i2d::CVector2d>& destPoints,
				double maxCost,
				int minDestPoints);
	/**
		Get cost of optimal path.
		Optional this function returns set of points to the destination points.
	*/
	double GetOptimalPath(
				const i2d::CVector2d& destPoint,
				i2d::CPolypoint* optimalPathPtr = nullptr) const;

protected:
	typedef std::multimap<double, istd::CIndex2d> ProcessingQueue;

	struct CostElement
	{
		qint8 revDirCode;
		qint8 invMask;
		float costSum;
		float costX;
		float costY;
		float costOffset;
	};

	struct Neighbourhood
	{
		Neighbourhood(float distance, float dirX, float dirY, int revDirCode)
		{
			this->distance = distance;
			this->dirX = dirX;
			this->dirY = dirY;
			this->revDirCode = quint8(revDirCode);
			this->invMask = quint8(~(1 << revDirCode));
		}
		float distance;
		float dirX;
		float dirY;
		quint8 revDirCode;
		quint8 invMask;
	};

	void TryChangeNeighbour(
				const CostElement& element,
				const Neighbourhood& neigbourhood,
				const istd::CIndex2d& neighbourIndex,
				CostElement& neighbourElement);
	bool PopNextElement(istd::CIndex2d& elementIndex, float& elementCostSum);

private:
	bool m_isInitialized;
	float m_processedCostSum;
	istd::CIndex2d m_startIndex;
	ProcessingQueue m_processingQueue;
	istd::CIndex2d m_workMapPos;
	iimg::CGeneralBitmap m_workMap;
};


// inline public methods

inline bool CDirectionalOptimalPathSolver::IsInitialized() const
{
	return m_isInitialized;
}


// inline protected methods

__forceinline void CDirectionalOptimalPathSolver::TryChangeNeighbour(
			const CostElement& element,
			const Neighbourhood& neigbourhood,
			const istd::CIndex2d& neighbourIndex,
			CostElement& neighbourElement)
{
	Q_ASSERT(element.costSum >= 0);

	if (neighbourElement.costSum > 0){
		Q_ASSERT(neighbourElement.costSum >= 0);

		double edgeCost =
					element.costOffset + neighbourElement.costOffset +
					(element.costX + neighbourElement.costX) * neigbourhood.dirX +
					(element.costY + neighbourElement.costY) * neigbourhood.dirY;
		float costSum = (edgeCost > 0)? element.costSum + edgeCost * neigbourhood.distance: element.costSum;

		if (costSum < neighbourElement.costSum){
			m_processingQueue.insert(ProcessingQueue::value_type(costSum, neighbourIndex));

			neighbourElement.revDirCode = neigbourhood.revDirCode;
			neighbourElement.invMask &= neigbourhood.invMask;
			neighbourElement.costSum = costSum;
		}
	}
}


__forceinline bool CDirectionalOptimalPathSolver::PopNextElement(istd::CIndex2d& elementIndex, float& elementCostSum)
{
	ProcessingQueue::iterator nearestNodeIter = m_processingQueue.begin();
	if (nearestNodeIter != m_processingQueue.end()){
		elementIndex = nearestNodeIter->second;
		elementCostSum = nearestNodeIter->first;

		m_processingQueue.erase(nearestNodeIter);

		return true;
	}

	return false;
}


} // namespace iipr


#endif // !iipr_CDirectionalOptimalPathSolver_included


