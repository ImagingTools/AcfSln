// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iedge/CEdgesReductorComp.h>


// ACF includes
#include <istd/istd.h>
#include <iprm/TParamsPtr.h>


namespace iedge
{

CEdgesReductorComp::CEdgesReductorComp()
:	m_distanceUnitInfo(imath::IUnitInfo::UT_RELATIVE, "px", 1, istd::CRange(0, 10))
{
}


void CEdgesReductorComp::GetReducedLine(
			const CEdgeLine& edgeLine,
			double positionTolerance,
			double weightTolerance,
			CEdgeLine& result) const
{
	bool isClosed = edgeLine.IsClosed();

	int nodesCount = edgeLine.GetNodesCount();
	if (isClosed){
		if (nodesCount <= 3){
			result = edgeLine;

			return;	// not enough nodes to reduce
		}
	}
	else{
		if (nodesCount <= 2){
			result = edgeLine;

			return;	// not enough nodes to reduce
		}
	}

	m_nodesToRemove.fill(false, nodesCount);

	int firstInsideIndex = 0;
	int lastInsideIndex = 0;
	int reducedCount = ReduceNodes(
				edgeLine,
				positionTolerance,
				weightTolerance,
				0,
				isClosed? nodesCount: nodesCount - 1,
				m_nodesToRemove,
				firstInsideIndex,
				lastInsideIndex);

	if (nodesCount - reducedCount >= (isClosed? 3: 2)){
		// copy the nodes could not be reduced
		result.Clear();

		for (		int nodeIndex = 0;
					nodeIndex < nodesCount;
					++nodeIndex){
			if (!m_nodesToRemove[nodeIndex]){
				const CEdgeNode& node = edgeLine.GetNode(nodeIndex);

				result.InsertNode(node);
			}
		}

		result.SetClosed(isClosed);
	}
	else{
		// no reduction done, simple copy can be used
		result = edgeLine;
	}
}


void CEdgesReductorComp::GetReducedLines(
			const CEdgeLineContainer& edgeLines,
			double positionTolerance,
			double weightTolerance,
			CEdgeLineContainer& result) const
{
	int linesCount = edgeLines.GetItemsCount();

	result.Resize(linesCount);

	for (int i = 0; i < linesCount; ++i){
		const CEdgeLine& edgeLine = edgeLines.GetAt(i);
		CEdgeLine& resultLine = result.GetAt(i);
		resultLine.Clear();

		GetReducedLine(edgeLine, positionTolerance, weightTolerance, resultLine);
	}
}


// reimplemented (iedge::IEdgeLinesProcessor)

bool CEdgesReductorComp::DoLinesProcessing(
			const iprm::IParamsSet* paramsPtr,
			const CEdgeLineContainer& edgeLines,
			CEdgeLineContainer& result) const
{
	imath::CVarVector toleranceValues;
	if ((paramsPtr != NULL) && m_toleranceParamsIdAttrPtr.IsValid()){
		iprm::TParamsPtr<imeas::INumericValue> toleranceParamsPtr(paramsPtr, *m_toleranceParamsIdAttrPtr);
		if (toleranceParamsPtr.IsValid()){
			toleranceValues = toleranceParamsPtr->GetValues();
		}
	}
	else if (m_defaultToleranceParamsCompPtr.IsValid()){
		toleranceValues = m_defaultToleranceParamsCompPtr->GetValues();
	}

	double positionTolerance = 0.1;
	double weightTolerance = 0.1;

	if (toleranceValues.GetElementsCount() >= 1){
		positionTolerance = toleranceValues[0];
	}

	if (toleranceValues.GetElementsCount() >= 2){
		weightTolerance = toleranceValues[1];
	}

	GetReducedLines(edgeLines, positionTolerance, weightTolerance, result);

	return true;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CEdgesReductorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	const CEdgeLineContainer* edgeLinesPtr = dynamic_cast<const CEdgeLineContainer*>(inputPtr);
	CEdgeLineContainer* containerPtr = dynamic_cast<CEdgeLineContainer*>(outputPtr);

	if (		(edgeLinesPtr == NULL) ||
				(containerPtr == NULL)){
		return TS_INVALID;
	}

	return DoLinesProcessing(paramsPtr, *edgeLinesPtr, *containerPtr)? TS_OK: TS_INVALID;
}


// reimplemented (imeas::INumericConstraints)

const iprm::IOptionsList& CEdgesReductorComp::GetValueListInfo() const
{
	return *this;
}


const imath::IUnitInfo* CEdgesReductorComp::GetNumericValueUnitInfo(int index) const
{
	static imath::CGeneralUnitInfo weightUnitInfo(imath::IUnitInfo::UT_RELATIVE, "%", 100, istd::CRange(0.00, 1));

	switch (index){
	case 0:
			return &m_distanceUnitInfo;
	case 1:
		return &weightUnitInfo;

	default:
		return NULL;
	}
}


// protected methods

int CEdgesReductorComp::ReduceNodes(
			const CEdgeLine& edgeLine,
			double positionTolerance,
			double weightTolerance,
			int firstIndex,
			int lastIndex,
			NodesToRemove& nodesToRemove,
			int& firstInsideIndex,
			int& lastInsideIndex) const
{
	Q_ASSERT(firstIndex >= 0);
	Q_ASSERT(lastIndex > firstIndex);

	int removedPoints = 0;

	int nodesInsideCount = lastIndex - firstIndex - 1;

	if (nodesInsideCount >= 1){
		int nodesCount = edgeLine.GetNodesCount();

		int nodeIndex = firstIndex + nodesInsideCount / 2 + 1;
		int centerNodeIndex = nodeIndex % nodesCount;
		int leftNodeIndex = (nodeIndex - 1) % nodesCount;
		int rightNodeIndex = (nodeIndex + 1) % nodesCount;

		if (nodeIndex > firstIndex + 1){
			removedPoints += ReduceNodes(
						edgeLine,
						positionTolerance,
						weightTolerance,
						firstIndex,
						nodeIndex,
						nodesToRemove,
						firstInsideIndex,
						leftNodeIndex);
		}
		else{
			firstInsideIndex = firstIndex + 1;
		}

		if (nodeIndex < lastIndex - 1){
			removedPoints += ReduceNodes(
						edgeLine,
						positionTolerance,
						weightTolerance,
						nodeIndex,
						lastIndex,
						nodesToRemove,
						rightNodeIndex,
						lastInsideIndex);
		}
		else{
			lastInsideIndex = lastIndex - 1;
		}

		const CEdgeNode& node = edgeLine.GetNode(centerNodeIndex);
		const CEdgeNode& prevNode = edgeLine.GetNode(leftNodeIndex);
		const CEdgeNode& nextNode = edgeLine.GetNode(rightNodeIndex);

		i2d::CVector2d nextPrevDelta = nextNode.GetPosition() - prevNode.GetPosition();
		i2d::CVector2d prevDelta = node.GetPosition() - prevNode.GetPosition();

		double removedDistance = 0;
		double removedWeightProp = 0;

		double nextPrevDistance = nextPrevDelta.GetLength();
		if (nextPrevDistance >= I_BIG_EPSILON){
			removedDistance = qFabs(prevDelta.GetCrossProductZ(nextPrevDelta) / nextPrevDistance);

			double segmentAlpha = prevDelta.GetDotProduct(nextPrevDelta) / nextPrevDelta.GetLength2();
			double interpolatedWeight = (1 - segmentAlpha) * prevNode.GetWeight() + segmentAlpha * nextNode.GetWeight();

			removedWeightProp = node.GetWeight() / interpolatedWeight;
		}
		else{
			removedDistance = prevDelta.GetLength();

			double averageWeight = (prevNode.GetWeight() + nextNode.GetWeight()) * 0.5;

			removedWeightProp = node.GetWeight() / averageWeight;
		}

		if ((removedDistance < positionTolerance) && (qFabs(removedWeightProp - 1) < weightTolerance)){
			nodesToRemove[centerNodeIndex] = true;

			if (firstInsideIndex == nodeIndex){
				firstInsideIndex = rightNodeIndex;
			}

			if (lastInsideIndex == nodeIndex){
				lastInsideIndex = leftNodeIndex;
			}

			++removedPoints;
		}
	}
	else{
		firstInsideIndex = firstIndex + 1;
		lastInsideIndex = lastIndex - 1;
	}

	return removedPoints;
}


// reimplemented (iprm::IOptionsList)

int CEdgesReductorComp::GetOptionsFlags() const
{
	return SCF_SUPPORT_UNIQUE_ID;
}


int CEdgesReductorComp::GetOptionsCount() const
{
	return 2;
}


QString CEdgesReductorComp::GetOptionName(int index) const
{
	switch (index){
	case 0:
		return QObject::tr("Position tolerance");

	case 1:
		return QObject::tr("Weight tolerance");

	default:
		return "";
	}
}


QString CEdgesReductorComp::GetOptionDescription(int index) const
{
	switch (index){
	case 0:
		return QObject::tr("Tolerance of position for edge node reduction");

	case 1:
		return QObject::tr("Tolerance of weight for edge node reduction");

	default:
		return "";
	}
}


QByteArray CEdgesReductorComp::GetOptionId(int index) const
{
	switch (index){
	case 0:
		return "PositionTolerance";

	case 1:
		return "WeightTolerance";

	default:
		return "";
	}
}


bool CEdgesReductorComp::IsOptionEnabled(int /*index*/) const
{
	return true;
}


// reimplemented (icomp::CComponentBase)

void CEdgesReductorComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	// Force components initialization
	m_defaultToleranceParamsCompPtr.EnsureInitialized();
	m_distanceUnitInfoCompPtr.EnsureInitialized();

	if (m_distanceUnitInfoCompPtr.IsValid()){
		m_distanceUnitInfo.SetUnitType(m_distanceUnitInfoCompPtr->GetUnitType());
		m_distanceUnitInfo.SetUnitName(m_distanceUnitInfoCompPtr->GetUnitName());
		m_distanceUnitInfo.SetDisplayMultiplicationFactor(m_distanceUnitInfoCompPtr->GetDisplayMultiplicationFactor());
		m_distanceUnitInfo.SetValueRange(istd::CRange(0, 1));
	}
}


} // namespace iedge



