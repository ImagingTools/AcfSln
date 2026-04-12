// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ialgo_TSimpleSpaceResultConsumer_included
#define ialgo_TSimpleSpaceResultConsumer_included


// Qt includes
#include <QtCore/QMultiMap>

// ACF includes
#include <imath/TVector.h>

// ACF-Solutions includes
#include <ialgo/TIHoughSpace.h>


namespace ialgo
{


/**
	Consumer of results generated for some multidimensional space (typically Hough space).
*/
template <int Dimensions, class Element = double>
class TSimpleSpaceResultConsumer:
			public iimg::CGeneralBitmap,
			virtual public TIHoughSpace<Dimensions, Element>::ResultsConsumer
{
public:
	typedef imath::TVector<Dimensions> PosVector;
	/**
		List of weights and positions.
	*/
	typedef QMultiMap<double, PosVector> PosMap;

	/**
		Construct the result processor.
		\param	maxPoints				maximal number of points beeing in result set.
		\param	maxConsideredPoints		maximal number of points stored internally.
										Setting this number to smaller value speeds up the processing.
		\param	minDistance				Minimal distance between the points.
										Please note that the neighbourshood will be analysed at end.
	*/
	TSimpleSpaceResultConsumer(int maxPoints, int maxConsideredPoints, double minDistance, double minMaxRatio);

	// reimplemented (ialgo::TIHoughSpace::ResultsConsumer)
	virtual QList<int> GetSupportedNeghboursCount() const override;
	virtual void OnProcessingBegin(
				const TIHoughSpace<Dimensions, Element>& space,
				const Element& minValue) override;
	virtual void OnProcessingEnd(const TIHoughSpace<Dimensions, Element>& space) override;
	virtual bool OnMaximumFound(
				const TIHoughSpace<Dimensions, Element>& space,
				const istd::TIndex<Dimensions>& position,
				const Element& value,
				const Element* neghboursPtr,
				int neghboursCount,
				Element& minValue) override;

	PosMap positions;

private:
	int m_maxPoints = 0;
	int m_maxConsideredPoints = 0;
	double m_minDistance = 0.0;
	double m_minMaxRatio = 0.0;

	double m_maxValue = 0.0;
};


// public methods

// reimplemented (ialgo::TIHoughSpace::ResultsConsumer)

template <int Dimensions, class Element>
TSimpleSpaceResultConsumer<Dimensions, Element>::TSimpleSpaceResultConsumer(int maxPoints, int maxConsideredPoints, double minDistance, double minMaxRatio)
:	m_maxPoints(maxPoints),
	m_maxConsideredPoints(maxConsideredPoints),
	m_minDistance(minDistance),
	m_minMaxRatio(minMaxRatio),
	m_maxValue(std::numeric_limits<double>::lowest())
{
}


template <int Dimensions, class Element>
QList<int> TSimpleSpaceResultConsumer<Dimensions, Element>::GetSupportedNeghboursCount() const
{
	return QList<int>() << Dimensions * 2;
}


template <int Dimensions, class Element>
void TSimpleSpaceResultConsumer<Dimensions, Element>::OnProcessingBegin(
			const TIHoughSpace<Dimensions, Element>& /*space*/,
			const Element& /*minValue*/)
{
	m_maxValue = std::numeric_limits<double>::lowest();
}


template <int Dimensions, class Element>
void TSimpleSpaceResultConsumer<Dimensions, Element>::OnProcessingEnd(const TIHoughSpace<Dimensions, Element>& space)
{
	PosMap finalPositions;

	if (m_maxPoints != 0){
		// remove elements beeing to close to each other
		for (		typename PosMap::ConstIterator pointIter = positions.constBegin();
					pointIter != positions.constEnd();
					++pointIter){
			const PosVector& point1 = pointIter.value();

			bool isToClose = false;

			for (		typename PosMap::ConstIterator searchClosedIter = finalPositions.constBegin();
						searchClosedIter != finalPositions.constEnd();
						++searchClosedIter){
				const PosVector& point2 = searchClosedIter.value();

				double dist2 = space.GetSpaceDistance2(point1, point2);
				if (dist2 <= m_minDistance * m_minDistance){
					isToClose = true;

					break;
				}
			}

			if (!isToClose){
				finalPositions.insertMulti(-pointIter.key(), point1);

				if ((m_maxPoints >= 0) && (finalPositions.size() >= m_maxPoints)){
					break;
				}
			}
		}
	}

	positions.swap(finalPositions);
}


template <int Dimensions, class Element>
bool TSimpleSpaceResultConsumer<Dimensions, Element>::OnMaximumFound(
			const TIHoughSpace<Dimensions, Element>& /*space*/,
			const istd::TIndex<Dimensions>& position,
			const Element& value,
			const Element* neghboursPtr,
			int neghboursCount,
			Element& minValue)
{
	Q_ASSERT(neghboursCount == Dimensions * 2);
	Q_UNUSED(neghboursCount);

	PosVector resultPos;

	for (int dimensionIndex = 0; dimensionIndex < Dimensions; ++dimensionIndex){
		Element diffLeft = double(value) - double(neghboursPtr[dimensionIndex * 2]);
		Element diffRight = double(value) - double(neghboursPtr[dimensionIndex * 2 + 1]);

		double normValue = diffLeft + diffRight;
		double correction = (normValue > I_BIG_EPSILON)? diffLeft / normValue: 0.5;

		resultPos[dimensionIndex] = position[dimensionIndex] + correction;
	}

	positions.insert(-value, resultPos);

	if (value > m_maxValue){
		m_maxValue = double(value);
		Element propValue = Element(value * m_minMaxRatio);
		if (minValue < propValue){
			minValue = propValue;

			// remove elements weeker than new calculated minValue
			while (!positions.isEmpty()){
				typename PosMap::Iterator lastIter = std::prev(positions.end());

				if (-lastIter.key() < minValue){
					positions.erase(lastIter);
				}
				else{
					break;
				}
			}
		}
	}

	// try remove the last one if we have too many points
	if (positions.count() > m_maxConsideredPoints){
		typename PosMap::Iterator lastIter = std::prev(positions.end());
		double lastValue = -lastIter.key();
		positions.erase(lastIter);

		if (lastValue > minValue){
			minValue = lastValue;
		}
	}

	return false;
}


} // namespace ialgo


#endif // !ialgo_TSimpleSpaceResultConsumer_included


