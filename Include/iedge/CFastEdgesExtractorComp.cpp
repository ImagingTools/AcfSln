// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iedge/CFastEdgesExtractorComp.h>


// STL includes
#include <cmath>
#include <vector>

// Qt includes
#include <QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtCore/QtMath>
#else
#include <QtCore/qmath.h>
#endif
#include <QtCore/QMutexLocker>
#include <QtCore/QDebug>

// ACF includes
#include <istd/TDelPtr.h>
#include <imath/CGeneralUnitInfo.h>
#include <i2d/CPolypoint.h>
#include <i2d/CVector2d.h>
#include <iprm/TParamsPtr.h>

// ACF-Solutions includes
#include <iedge/CEdgeNode.h>
#include <iedge/CEdgeLine.h>
#include <iedge/CEdgeLineContainer.h>


namespace iedge
{


//	reimplemented (iedge::IEdgesExtractor)

bool CFastEdgesExtractorComp::DoContourExtraction(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& bitmap,
			CEdgeLineContainer& result) const
{
	istd::CChangeNotifier notifier(&result);

	result.Reset();

	ibase::CSize size = bitmap.GetImageSize();
	if ((size.GetX() < 3) || (size.GetY() < 3)){
		SendErrorMessage(0, "Image too small to calculate edges");

		return false;
	}

	imath::CVarVector thresholdValues;
	if ((paramsPtr != NULL) && m_thresholdParamIdAttrPtr.IsValid()){
		iprm::TParamsPtr<imeas::INumericValue> thresholdParamPtr(paramsPtr, *m_thresholdParamIdAttrPtr);
		if (thresholdParamPtr.IsValid()){
			thresholdValues = thresholdParamPtr->GetValues();
		}
	}
	else if (m_defaultThresholdParamCompPtr.IsValid()){
		thresholdValues = m_defaultThresholdParamCompPtr->GetValues();
	}

	double threshold = (thresholdValues.GetElementsCount() > 0)? thresholdValues.GetElement(0): 0.1;
	double scaleFactor = (thresholdValues.GetElementsCount() > 1)? thresholdValues.GetElement(1): 0;
	double weightScale = qPow(10, scaleFactor * 0.1);

	// create image mask
	i2d::CRect clipArea(size);
	iimg::CScanlineMask mask;
	iprm::TParamsPtr<i2d::IObject2d> aoiObjectPtr(paramsPtr, m_aoiParamIdAttrPtr, m_defaultAoiCompPtr, false);
	if (aoiObjectPtr.IsValid()){
		mask.SetCalibration(bitmap.GetCalibration());

		if (!mask.CreateFromGeometry(*aoiObjectPtr.GetPtr(), &clipArea)){
			SendErrorMessage(0, QObject::tr("AOI type is not supported"));
		}
	}
	else{
		mask.CreateFilled(clipArea);
	}

	const quint32 threshold2Factor = quint32(threshold * threshold * THRESHOLD_FACTOR * THRESHOLD_FACTOR / (weightScale * weightScale));

	int imageWidth = size.GetX();

	std::vector<PixelDescriptor> destLine1(imageWidth);
	PixelDescriptor* destLine1Ptr = &destLine1[0];
	memset(destLine1Ptr, 0, imageWidth * sizeof(PixelDescriptor));

	std::vector<PixelDescriptor> destLine2(imageWidth);
	PixelDescriptor* destLine2Ptr = &destLine2[0];
	memset(destLine2Ptr, 0, imageWidth * sizeof(PixelDescriptor));

	std::vector<PixelDescriptor> destLine3(imageWidth);
	PixelDescriptor* destLine3Ptr = &destLine3[0];
	memset(destLine3Ptr, 0, imageWidth * sizeof(PixelDescriptor));

	InternalContainer container(*m_maxNodesCountAttrPtr);

	i2d::CRect boundingRect = mask.GetBoundingRect();

	iimg::CScanlineMask resultMask = mask;
	resultMask.Erode(3, 0, 3, 0);

	const quint8* prevSourceLinePtr = (const quint8*)bitmap.GetLinePtr(boundingRect.GetTop());
	const quint8* sourceLinePtr = NULL;

	istd::CIntRange imageRangeH(0, size.GetX());

	for (int y = boundingRect.GetTop() + 1; y < boundingRect.GetBottom(); ++y){
		sourceLinePtr = (const quint8*)bitmap.GetLinePtr(y);

		const istd::CIntRanges* inputRangesPtr = mask.GetPixelRanges(y);
		if (inputRangesPtr != NULL){
			istd::CIntRanges::RangeList rangeList;
			inputRangesPtr->GetAsList(imageRangeH, rangeList);

			for (		istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter){
				const istd::CIntRange& rangeH = *iter;

				CalcDerivativeLine(prevSourceLinePtr, sourceLinePtr, rangeH.GetMinValue(), rangeH.GetMaxValue(), destLine3Ptr);
			}
		}

		const istd::CIntRanges* outputRangesPtr = resultMask.GetPixelRanges(y);
		if (outputRangesPtr != NULL){
			Q_ASSERT(y >= 3);

			istd::CIntRanges::RangeList rangeList;
			outputRangesPtr->GetAsList(imageRangeH, rangeList);

			for (istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter){
				const istd::CIntRange& rangeH = *iter;
				Q_ASSERT(rangeH.GetMinValue() >= 3);
				Q_ASSERT(rangeH.GetMaxValue() <= size.GetX());

				CalcOutputLine(rangeH.GetMinValue(), rangeH.GetMaxValue(), y, threshold2Factor, destLine1Ptr, destLine2Ptr, destLine3Ptr, container);
			}
		}

		// move line address in rolling line buffer
		PixelDescriptor* storedDestLine1Ptr = destLine1Ptr;
		destLine1Ptr = destLine2Ptr;
		destLine2Ptr = destLine3Ptr;
		destLine3Ptr = storedDestLine1Ptr;

		prevSourceLinePtr = sourceLinePtr;
	}

	container.ExtractLines(weightScale, result, *m_keepSingletonsAttrPtr);

	// copy calibration from bitmap
	const i2d::ICalibration2d* bitmapCalibrationPtr = bitmap.GetCalibration();
	if (bitmapCalibrationPtr != NULL){
		istd::TUniqueInterfacePtr<i2d::ICalibration2d> newCalibration;
		newCalibration.MoveCastedPtr(bitmapCalibrationPtr->CloneMe());

		result.SetCalibration(newCalibration.PopInterfacePtr(), true);
	}
	else{
		result.SetCalibration(NULL);
	}

	if (container.IsContainerFull()){
		SendErrorMessage(0, "Container of nodes is full");

		return false;
	}

	return true;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CFastEdgesExtractorComp::DoProcessing(
		const iprm::IParamsSet* paramsPtr,
		const istd::IPolymorphic* inputPtr,
		istd::IChangeable* outputPtr,
		ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	const iimg::IBitmap* bitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	CEdgeLineContainer* containerPtr = dynamic_cast<CEdgeLineContainer*>(outputPtr);

	if (		(bitmapPtr == NULL) ||
				(containerPtr == NULL)){
		return TS_INVALID;
	}

	return DoContourExtraction(paramsPtr, *bitmapPtr, *containerPtr)? TS_OK: TS_INVALID;
}


// reimplemented (imeas::INumericConstraints)

const iprm::IOptionsList& CFastEdgesExtractorComp::GetValueListInfo() const
{
	return *this;
}


const imath::IUnitInfo* CFastEdgesExtractorComp::GetNumericValueUnitInfo(int index) const
{
	static imath::CGeneralUnitInfo thresholdUnitInfo(imath::IUnitInfo::UT_RELATIVE, "%", 100, istd::CRange(0.01, 1));
	static imath::CGeneralUnitInfo scaleUnitInfo(imath::IUnitInfo::UT_RELATIVE, "dB", 1, istd::CRange(-10, 10));

	switch (index){
	case 0:
		return &thresholdUnitInfo;

	case 1:
		return &scaleUnitInfo;

	default:
		return NULL;
	}
}


// protected methods

// reimplemented (iprm::IOptionsList)

int CFastEdgesExtractorComp::GetOptionsFlags() const
{
	return SCF_SUPPORT_UNIQUE_ID;
}


int CFastEdgesExtractorComp::GetOptionsCount() const
{
	return 2;
}


QString CFastEdgesExtractorComp::GetOptionName(int index) const
{
	switch (index){
	case 0:
		return QObject::tr("Threshold", "Contour extraction threshold");

	case 1:
		return QObject::tr("Scale", "Contour scale factor");

	default:
		return "";
	}
}


QString CFastEdgesExtractorComp::GetOptionDescription(int index) const
{
	switch (index){
	case 0:
		return QObject::tr("Contour extraction threshold");

	case 1:
		return QObject::tr("Contour scale factor");

	default:
		return "";
	}
}


QByteArray CFastEdgesExtractorComp::GetOptionId(int index) const
{
	switch (index){
	case 0:
		return "Threshold";

	case 1:
		return "Scale";

	default:
		return "";
	}
}


bool CFastEdgesExtractorComp::IsOptionEnabled(int /*index*/) const
{
	return true;
}


// reimplemented (icomp::CComponentBase)

void CFastEdgesExtractorComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	// Force components initialization
	m_defaultThresholdParamCompPtr.EnsureInitialized();
	m_defaultAoiCompPtr.EnsureInitialized();
}


// private static methods

inline void CFastEdgesExtractorComp::TryConnectElements(
			const PixelDescriptor& neightborPixel,
			PixelDescriptor& pixel)
{
	ExtNode* neightborNodePtr = neightborPixel.listReference;
	if (neightborNodePtr == NULL){
		return;
	}

	ExtNode* nodePtr = pixel.listReference;
	Q_ASSERT(nodePtr != NULL);

	i2d::CVector2d displacement = neightborNodePtr->position - nodePtr->position;

	double derivativeDotProduct = nodePtr->derivative.GetDotProduct(neightborNodePtr->derivative);
	if (derivativeDotProduct > 0){
		double orientationFactor = (nodePtr->derivative + neightborNodePtr->derivative).GetCrossProductZ(displacement) / displacement.GetLength2();
		if (orientationFactor >= 0){
			double connectionWeight = orientationFactor * derivativeDotProduct * nodePtr->rawWeight * neightborNodePtr->rawWeight;
			if (connectionWeight > nodePtr->nextWeight){
				// connect as next
				nodePtr->nextPtr = neightborNodePtr;
				nodePtr->nextWeight = connectionWeight;
			}
		}
		else{
			double connectionWeight = -orientationFactor * derivativeDotProduct * nodePtr->rawWeight * neightborNodePtr->rawWeight;
			if (connectionWeight > nodePtr->prevWeight){
				// connect as previous
				nodePtr->prevPtr = neightborNodePtr;
				nodePtr->prevWeight = connectionWeight;
			}
		}
	}
}


inline void CFastEdgesExtractorComp::CalcDerivative(
			const quint8* prevSourceLine,
			const quint8* sourceLine,
			int x,
			PixelDescriptor& pixelDescriptor)
{
	Q_ASSERT(x > 0);

	int pixel11 = prevSourceLine[x - 1];
	int pixel21 = prevSourceLine[x];
	int pixel12 = sourceLine[x - 1];
	int pixel22 = sourceLine[x];

	int dx = pixel11 + pixel12 - pixel21 - pixel22;
	int dy = pixel11 + pixel21 - pixel12 - pixel22;

	pixelDescriptor.brightness = qint16(pixel11 + pixel21 + pixel12 + pixel22);
	pixelDescriptor.dx = qint16(dx);
	pixelDescriptor.dy = qint16(dy);
	pixelDescriptor.dirLength2 = quint32(dx * dx + dy * dy);
	pixelDescriptor.listReference = NULL;
}


inline void CFastEdgesExtractorComp::CalcDerivativeLine(
			const quint8* prevSourceLine,
			const quint8* sourceLine,
			int inputBeginX,
			int inputEndX,
			PixelDescriptor* destLine)
{
	for (int x = inputBeginX + 1; x < inputEndX; ++x){
		PixelDescriptor& pixelDescriptor = destLine[x];

		CalcDerivative(prevSourceLine, sourceLine, x, pixelDescriptor);
	}
}


inline void CFastEdgesExtractorComp::CalcPoint(
			int x,
			int y,
			quint32 threshold2Factor,
			PixelDescriptor* prevPrevDestLine,
			PixelDescriptor* prevDestLine,
			PixelDescriptor* destLine,
			InternalContainer& container)
{
	Q_ASSERT(x >= 3);
	Q_ASSERT(y >= 3);

	PixelDescriptor& centralDescriptor = prevDestLine[x - 1];

	quint32 dirLength2 = centralDescriptor.dirLength2;

	if (dirLength2 <= threshold2Factor){
		return;	// edge to weak
	}

	const PixelDescriptor& leftDescriptor = prevDestLine[x - 2];
	const PixelDescriptor& rightDescriptor = prevDestLine[x];
	const PixelDescriptor& topDescriptor = prevPrevDestLine[x - 1];
	const PixelDescriptor& bottomDescriptor = destLine[x - 1];

	int left2Diff = dirLength2 - leftDescriptor.dirLength2;
	int right2Diff = dirLength2 - rightDescriptor.dirLength2;
	int top2Diff = dirLength2 - topDescriptor.dirLength2;
	int bottom2Diff = dirLength2 - bottomDescriptor.dirLength2;

	if (((left2Diff < 0) || (right2Diff <= 0)) && ((top2Diff < 0) || (bottom2Diff <= 0))){
		return;	// it is not local maximum
	}
	double horizStrength2 = 0;
	if ((left2Diff >= 0) && (right2Diff > 0)){
		horizStrength2 = left2Diff + right2Diff;
	}

	double vertStrength2 = 0;
	if ((top2Diff >= 0) && (bottom2Diff > 0)){
		vertStrength2 = top2Diff + bottom2Diff;
	}
/*
	if (horizStrength2 + vertStrength2 <= threshold2Factor){
		return;	// not enough strong compared to noise
	}
*/
	ExtNode* nodePtr = container.AddElementToList();
	if (nodePtr == NULL){
		return;	// edge cannot be allocated
	}

	bool isHorizontal = (horizStrength2 > vertStrength2);

	// calculate new point values
	double dirLength = qSqrt(dirLength2);
	nodePtr->derivative = i2d::CVector2d(double(centralDescriptor.dx) / THRESHOLD_FACTOR, double(centralDescriptor.dy) / THRESHOLD_FACTOR);
	nodePtr->rawWeight = qSqrt(horizStrength2 + vertStrength2);
	nodePtr->prevPtr = NULL;
	nodePtr->nextPtr = NULL;
	nodePtr->prevWeight = 0;
	nodePtr->nextWeight = 0;
	nodePtr->isHorizontal = isHorizontal;

	if (isHorizontal){
		double leftDiff = qSqrt(dirLength) - qSqrt(qSqrt(leftDescriptor.dirLength2));
		Q_ASSERT(leftDiff >= 0);
		double rightDiff = qSqrt(dirLength) - qSqrt(qSqrt(rightDescriptor.dirLength2));
		Q_ASSERT(rightDiff >= 0);

		double shift = leftDiff / (leftDiff + rightDiff);

		nodePtr->position.SetX(x - 1.5 + shift);
		nodePtr->position.SetY(y - 1);
	}
	else{
		double topDiff = qSqrt(dirLength) - qSqrt(qSqrt(topDescriptor.dirLength2));
		Q_ASSERT(topDiff >= 0);
		double bottomDiff = qSqrt(dirLength) - qSqrt(qSqrt(bottomDescriptor.dirLength2));
		Q_ASSERT(bottomDiff >= 0);

		double shift = topDiff / (topDiff + bottomDiff);

		nodePtr->position.SetX(x - 1);
		nodePtr->position.SetY(y - 1.5 + shift);
	}

	PixelDescriptor& pixelDescriptor = destLine[x - 1];
	pixelDescriptor.listReference = nodePtr;

	// try connects with all already calculated neighbours
	TryConnectElements(prevPrevDestLine[x - 1], pixelDescriptor);

	TryConnectElements(prevDestLine[x - 2], pixelDescriptor);
	TryConnectElements(prevDestLine[x - 1], pixelDescriptor);
	TryConnectElements(prevDestLine[x], pixelDescriptor);

	TryConnectElements(destLine[x - 3], pixelDescriptor);
	TryConnectElements(destLine[x - 2], pixelDescriptor);
}


inline void CFastEdgesExtractorComp::CalcOutputLine(
			int outputBeginX,
			int outputEndX,
			int y,
			quint32 threshold2Factor,
			PixelDescriptor* prevPrevDestLine,
			PixelDescriptor* prevDestLine,
			PixelDescriptor* destLine,
			InternalContainer& container)
{
	Q_ASSERT(outputBeginX >= 3);
	Q_ASSERT(outputBeginX <= outputEndX);

	for (int x = outputBeginX; x < outputEndX; x++){
		CalcPoint(x, y, threshold2Factor, prevPrevDestLine, prevDestLine, destLine, container);
	}
}


// inline methods of embedded class InternalContainer

inline CFastEdgesExtractorComp::ExtNode* CFastEdgesExtractorComp::InternalContainer::AddElementToList()
{
	if (m_freeIndex < m_bufferSize){
		return &m_buffer[m_freeIndex++];
	}
	else{
		m_isContainerFull = true;

		return NULL;
	}
}


// public methods of embedded class InternalContainer

CFastEdgesExtractorComp::InternalContainer::InternalContainer(int size)
:	m_freeIndex(0),
	m_isContainerFull(false)
{
	m_buffer = (ExtNode*)(s_memoryManager.GetMemory(size * sizeof(ExtNode)));
	m_bufferSize = size;

	if (m_buffer == NULL){	// cannot allocate more memory there
		m_isContainerFull = true;
		m_bufferSize = 0;
	}
}


CFastEdgesExtractorComp::InternalContainer::~InternalContainer()
{
	if (m_buffer != NULL){
		//qDebug() << "Contour buffer size used: " << m_freeIndex;

		s_memoryManager.ReleaseMemory(m_buffer);
	}
}


bool CFastEdgesExtractorComp::InternalContainer::IsContainerFull() const
{
	return m_isContainerFull;
}


void CFastEdgesExtractorComp::InternalContainer::ExtractLines(
			double weightScale,
			CEdgeLineContainer& result,
			bool keepSingletons)
{
	result.Reserve(m_freeIndex);

	// mark all as not extracted and make connections symmetric
	for (int i = 0; i < m_freeIndex; ++i){
		ExtNode& node = m_buffer[i];
		node.isExtracted = false;

		if (node.nextWeight > node.prevWeight){
			if (node.nextPtr != NULL){
				Q_ASSERT(node.nextPtr < &node);	// only connections to earlier elements should be generated!

				if (node.nextPtr->prevWeight < node.nextWeight){
					if (node.nextPtr->prevPtr != NULL){
						Q_ASSERT(node.nextPtr->prevPtr->nextPtr == node.nextPtr);	// all earlier connections are already symmetric

						node.nextPtr->prevPtr->nextPtr = NULL;
						node.nextPtr->prevPtr->nextWeight = 0;
					}

					node.nextPtr->prevPtr = &node;
					node.nextPtr->prevWeight = node.nextWeight;
				}
				else{
					node.nextPtr = NULL;
					node.nextWeight = 0;
				}
			}

			if (node.prevPtr != NULL){
				Q_ASSERT(node.prevPtr < &node);	// only connections to earlier elements should be generated!

				if (node.prevPtr->nextWeight < node.prevWeight){
					if (node.prevPtr->nextPtr != NULL){
						Q_ASSERT(node.prevPtr->nextPtr->prevPtr == node.prevPtr);	// all earlier connections are already symmetric

						node.prevPtr->nextPtr->prevPtr = NULL;
						node.prevPtr->nextPtr->prevWeight = 0;
					}

					node.prevPtr->nextPtr = &node;
					node.prevPtr->nextWeight = node.prevWeight;
				}
				else{
					node.prevPtr = NULL;
					node.prevWeight = 0;
				}
			}
		}
		else{
			if (node.prevPtr != NULL){
				Q_ASSERT(node.prevPtr < &node);	// only connections to earlier elements should be generated!

				if (node.prevPtr->nextWeight < node.prevWeight){
					if (node.prevPtr->nextPtr != NULL){
						Q_ASSERT(node.prevPtr->nextPtr->prevPtr == node.prevPtr);	// all earlier connections are already symmetric

						node.prevPtr->nextPtr->prevPtr = NULL;
						node.prevPtr->nextPtr->prevWeight = 0;
					}

					node.prevPtr->nextPtr = &node;
					node.prevPtr->nextWeight = node.prevWeight;
				}
				else{
					node.prevPtr = NULL;
					node.prevWeight = 0;
				}
			}
			if (node.nextPtr != NULL){
				Q_ASSERT(node.nextPtr < &node);	// only connections to earlier elements should be generated!

				if (node.nextPtr->prevWeight < node.nextWeight){
					if (node.nextPtr->prevPtr != NULL){
						Q_ASSERT(node.nextPtr->prevPtr->nextPtr == node.nextPtr);	// all earlier connections are already symmetric

						node.nextPtr->prevPtr->nextPtr = NULL;
						node.nextPtr->prevPtr->nextWeight = 0;
					}

					node.nextPtr->prevPtr = &node;
					node.nextPtr->prevWeight = node.nextWeight;
				}
				else{
					node.nextPtr = NULL;
					node.nextWeight = 0;
				}
			}
		}
	}

	// looking for opened edges
	for (int i = 0; i < m_freeIndex; ++i){
		ExtNode& node = m_buffer[i];

		if (!node.isExtracted && (node.prevPtr == NULL)){	// check if it was not processed yet and is first node
			if (!keepSingletons && (node.nextPtr == NULL)){
				node.isExtracted = true;
				continue;	// skip singleton
			}

			CEdgeLine& resultLine = result.PushBack(CEdgeLine());
			resultLine.SetClosed(false);

			// count nodes
			int nodesCount = 0;
			for (const ExtNode* nodeElementPtr = &node; nodeElementPtr != NULL;	nodeElementPtr = nodeElementPtr->nextPtr){
				nodesCount++;
			}

			resultLine.SetNodesCount(nodesCount);

			int index = 0;
			for (		const ExtNode* nodeElementPtr = &node;
						nodeElementPtr != NULL;
						nodeElementPtr = nodeElementPtr->nextPtr){
				Q_ASSERT((nodeElementPtr->nextPtr == NULL) || (nodeElementPtr->nextPtr->prevPtr == nodeElementPtr));
				Q_ASSERT((nodeElementPtr->prevPtr == NULL) || (nodeElementPtr->prevPtr->nextPtr == nodeElementPtr));

				nodeElementPtr->isExtracted = true;

				CEdgeNode newNode(
							nodeElementPtr->position,
							nodeElementPtr->derivative.GetLength() * weightScale);

				resultLine.GetNodeRef(index++) = newNode;
			}
		}
	}

	// looking for closed edges
	for (int i = 0; i < m_freeIndex; ++i){
		ExtNode& node = m_buffer[i];

		if (!node.isExtracted){	// check if it was not processed yet
			if (!keepSingletons && (node.nextPtr == node.prevPtr)){
				node.isExtracted = true;
				continue;	// skip singleton
			}

			CEdgeLine& resultLine = result.PushBack(CEdgeLine());
			resultLine.SetClosed(true);

			// count nodes
			int nodesCount = 0;
			const ExtNode* nodeElementPtr = &node;
			do{
				nodeElementPtr = nodeElementPtr->nextPtr;
				nodesCount++;
			} while (nodeElementPtr != &node);

			resultLine.SetNodesCount(nodesCount);

			nodeElementPtr = &node;
			int index = 0;
			do{
				Q_ASSERT(nodeElementPtr->nextPtr != NULL);
				Q_ASSERT(nodeElementPtr->prevPtr != NULL);
				Q_ASSERT(nodeElementPtr->nextPtr->prevPtr == nodeElementPtr);
				Q_ASSERT(nodeElementPtr->prevPtr->nextPtr == nodeElementPtr);

				nodeElementPtr->isExtracted = true;

				CEdgeNode newNode(
							nodeElementPtr->position,
							nodeElementPtr->derivative.GetLength() * weightScale);

				resultLine.GetNodeRef(index++) = newNode;

				nodeElementPtr = nodeElementPtr->nextPtr;
			} while (nodeElementPtr != &node);
		}
	}
}

// private static members

CFastEdgesExtractorComp::MemoryPoolManager CFastEdgesExtractorComp::InternalContainer::s_memoryManager;


// public methods of the embedded class CMemoryPoolManager

CFastEdgesExtractorComp::MemoryPoolManager::~MemoryPoolManager()
{
	Dispose();
}


void* CFastEdgesExtractorComp::MemoryPoolManager::GetMemory(int bytes)
{
	if (bytes <= 0){
		return NULL;
	}

	QMutexLocker lock(&m_lock);

	// Look for available chunk:
	for (int i = 0; i < m_freeList.size(); i++){
		const ChunkInfo& chunk = m_freeList.at(i);
		if (chunk.bytes == bytes){
			ChunkInfo usedChunk = m_freeList.takeAt(i);

			m_usedList.append(usedChunk);

			return usedChunk.ptr;
		}
	}

	// No free chunks found -> try to allocate a new one:
	ChunkInfo usedChunk;
	usedChunk.ptr = malloc(bytes);
	if (usedChunk.ptr == NULL){
		// Cannot allocate more memory:
		return NULL;
	}

	usedChunk.bytes = bytes;
	m_usedList.append(usedChunk);

	return usedChunk.ptr;
}


bool CFastEdgesExtractorComp::MemoryPoolManager::ReleaseMemory(void* ptr)
{
	QMutexLocker lock(&m_lock);

	for (int i = 0; i < m_usedList.size(); i++){
		const ChunkInfo& chunk = m_usedList.at(i);
		if (chunk.ptr == ptr){
			ChunkInfo freeChunk = m_usedList.takeAt(i);

			m_freeList.append(freeChunk);

			return true;
		}
	}

	return false;
}


// this must be called at the very end of the program

void CFastEdgesExtractorComp::MemoryPoolManager::Dispose()
{
	for (int i = 0; i < m_freeList.size(); i++){
		const ChunkInfo& chunk = m_freeList.at(i);

		free(chunk.ptr);
	}

	m_freeList.clear();

	for (int i = 0; i < m_usedList.size(); i++){
		const ChunkInfo& chunk = m_usedList.at(i);

		free(chunk.ptr);
	}

	m_usedList.clear();
}


} // namespace iedge


