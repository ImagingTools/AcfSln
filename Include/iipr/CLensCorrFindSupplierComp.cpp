#include <iipr/CLensCorrFindSupplierComp.h>


// STL includes
#include <cmath>

// ACF include
#include <imath/CVarMatrix.h>
#include <ilog/CExtMessage.h>
#include <i2d/CLine2d.h>
#include <i2d/CPosition2d.h>
#include <i2d/CPolypoint.h>
#include <iprm/TParamsPtr.h>

// ACF-Solutions includes
#include <ialgo/CHoughSpace2d.h>
#include <iipr/CPerspCalibFinder.h>


namespace iipr
{


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CLensCorrFindSupplierComp::GetCalibration() const
{
	return GetWorkProduct();
}


//protected methods

bool CLensCorrFindSupplierComp::CalculateCalibrationFactors(
			const AllCorrectionInfos& allCorrectionInfos,
			bool searchCenterFlag,
			const icalib::CSimpleLensCorrection& inputCorrection,
			icalib::CSimpleLensCorrection& result) const
{
	int allPointsCount = 0;
	for (		AllCorrectionInfos::ConstIterator lineIter = allCorrectionInfos.constBegin();
				lineIter != allCorrectionInfos.constEnd();
				++lineIter){
		const CorrectionLineInfo& lineInfo = *lineIter;

		allPointsCount += lineInfo.infos.size();
	}

	int varCount = allCorrectionInfos.size() + 2;
	int lineMatrixOffset = 2;
	if (searchCenterFlag){
		varCount += 2;
		lineMatrixOffset += 2;
	}

	imath::CVarMatrix solutionMatrix;
	solutionMatrix.SetSizes(istd::CIndex2d(varCount, allPointsCount * 2));
	solutionMatrix.Clear();
	imath::CVarMatrix vectorY;
	vectorY.SetSizes(istd::CIndex2d(1, allPointsCount * 2));

	int pointIndex = 0;
	int lineIndex = 0;
	for (		AllCorrectionInfos::ConstIterator lineIter = allCorrectionInfos.constBegin();
				lineIter != allCorrectionInfos.constEnd();
				++lineIter, ++lineIndex){
		const CorrectionLineInfo& lineInfo = *lineIter;
		double orthoLengt2 = lineInfo.orthoVector.GetLength2();
		double weight = orthoLengt2 * orthoLengt2;
		i2d::CVector2d normalizedOrhoVector = lineInfo.orthoVector.GetOrthogonal();

		for (		CorrectionInfos::ConstIterator pointIter = lineInfo.infos.constBegin();
					pointIter != lineInfo.infos.constEnd();
					++pointIter, ++pointIndex){
			const CorrectionInfo& pointInfo = *pointIter;

			i2d::CVector2d destPos = inputCorrection.GetInvValueAt(pointInfo.foundPos);
			const i2d::CVector2d& sourcePos = pointInfo.onLinePos;

			double positionDist = sourcePos.GetLength();

			solutionMatrix.SetAt(istd::CIndex2d(0, pointIndex * 2), sourcePos.GetX() * weight);
			solutionMatrix.SetAt(istd::CIndex2d(1, pointIndex * 2), sourcePos.GetX() * positionDist * weight);
			solutionMatrix.SetAt(istd::CIndex2d(lineMatrixOffset + lineIndex, pointIndex * 2), -normalizedOrhoVector.GetX() * weight);
			vectorY.SetAt(istd::CIndex2d(0, pointIndex * 2), destPos.GetX() * weight);

			solutionMatrix.SetAt(istd::CIndex2d(0, pointIndex * 2 + 1), sourcePos.GetY() * weight);
			solutionMatrix.SetAt(istd::CIndex2d(1, pointIndex * 2 + 1), sourcePos.GetY() * positionDist * weight);
			solutionMatrix.SetAt(istd::CIndex2d(lineMatrixOffset + lineIndex, pointIndex * 2 + 1), -normalizedOrhoVector.GetY() * weight);
			vectorY.SetAt(istd::CIndex2d(0, pointIndex * 2 + 1), destPos.GetY() * weight);

			if (searchCenterFlag){
				solutionMatrix.SetAt(istd::CIndex2d(2, pointIndex * 2), weight);
				solutionMatrix.SetAt(istd::CIndex2d(3, pointIndex * 2), 0);
				solutionMatrix.SetAt(istd::CIndex2d(2, pointIndex * 2 + 1), 0);
				solutionMatrix.SetAt(istd::CIndex2d(3, pointIndex * 2 + 1), weight);
			}
		}
	}

	imath::CVarMatrix resolvedX;
	imath::CVarMatrix::SolveRobustLSP(solutionMatrix, vectorY, resolvedX);

	Q_ASSERT(resolvedX.GetSizes() == istd::CIndex2d(1, varCount));

	double scaleFactor = resolvedX.GetAt(istd::CIndex2d(0, 0));
	double distFactor = resolvedX.GetAt(istd::CIndex2d(0, 1)) / scaleFactor;
	result.SetScaleFactor(scaleFactor);
	result.SetDistortionFactor(distFactor);
	if (searchCenterFlag){
		i2d::CVector2d centerOffset(resolvedX.GetAt(istd::CIndex2d(0, 2)), resolvedX.GetAt(istd::CIndex2d(0, 3)));
		result.SetOpticalCenter(centerOffset);
	}
	else{
		result.SetOpticalCenter(i2d::CVector2d::GetZero());
	}

	return true;
}


void CLensCorrFindSupplierComp::AddCorrectPointsMessage(
			const AllCorrectionInfos& allCorrectionInfos,
			const icalib::CSimpleLensCorrection& inputCorrection,
			const i2d::CVector2d& imageCenter) const
{
	ilog::CExtMessage* corrPointsMessagePtr = new ilog::CExtMessage(
				istd::IInformationProvider::IC_INFO,
				0,
				"Corrected positions",
				"LensCorrectionFinder");

	int lineIndex = 0;
	for (		AllCorrectionInfos::ConstIterator lineIter = allCorrectionInfos.constBegin();
				lineIter != allCorrectionInfos.constEnd();
				++lineIter, ++lineIndex){
		const CorrectionLineInfo& lineInfo = *lineIter;

		i2d::CPolypoint* polyPointObjectPtr = new imod::TModelWrap<i2d::CPolypoint>();

		for (		CorrectionInfos::ConstIterator pointIter = lineInfo.infos.constBegin();
					pointIter != lineInfo.infos.constEnd();
					++pointIter){
			const CorrectionInfo& pointInfo = *pointIter;

			polyPointObjectPtr->InsertNode(inputCorrection.GetInvValueAt(pointInfo.foundPos) + imageCenter);
		}

		corrPointsMessagePtr->InsertAttachedObject(polyPointObjectPtr, QObject::tr("Corrected point of line %1").arg(lineIndex + 1));
	}

	AddMessage(corrPointsMessagePtr, MCT_RESULTS);

}


bool CLensCorrFindSupplierComp::CalculateCalibration(const iimg::IBitmap& image, icalib::CSimpleLensCorrection& result) const
{
	result.Reset();

	if (!m_pointFinderCompPtr.IsValid()){
		AddMessage(new ilog::CMessage(istd::IInformationProvider::IC_CRITICAL, 0, QObject::tr("No processor set"), "LensCorrectionFinder"));

		return false;
	}

	FeaturesConsumer consumer;
	if (m_pointFinderCompPtr->DoExtractFeatures(GetModelParametersSet(), image, consumer) != iproc::IProcessor::TS_OK){
		AddMessage(new ilog::CMessage(istd::IInformationProvider::IC_ERROR, 0, QObject::tr("Cannot extract features from image"), "LensCorrectionFinder"));

		return false;
	}

	if (consumer.features.size() < 9){
		AddMessage(new ilog::CMessage(istd::IInformationProvider::IC_ERROR, 0, QObject::tr("No enough features found"), "LensCorrectionFinder"));

		return false;
	}

	const iprm::IParamsSet* paramsPtr = GetModelParametersSet();

	bool searchCenterFlag = true;
	iprm::TParamsPtr<iprm::IEnableableParam> searchCenterParamPtr(paramsPtr, m_searchCenterParamIdAttrPtr, m_defaultSearchCenterParamCompPtr, false);
	if (searchCenterParamPtr.IsValid()){
		searchCenterFlag = searchCenterParamPtr->IsEnabled();
	}

	int angleGridSize = 360;
	int radiusGridSize = 360;
	int maxLinesCount = 20;
	double minDistanceToLine = 100;
	iprm::TParamsPtr<imeas::INumericValue> gridSearchParamsPtr(paramsPtr, m_gridSearchParamsIdAttrPtr, m_defaultGridSearchParamsCompPtr, false);
	if (gridSearchParamsPtr.IsValid()){
		imath::CVarVector values = gridSearchParamsPtr->GetValues();
		if (values.GetElementsCount() >= 4){
			angleGridSize = int(s_angleGridUnitInfo.GetValueRange().GetClipped(values.GetElement(0)));
			radiusGridSize = int(s_distanceGridUnitInfo.GetValueRange().GetClipped(values.GetElement(1)));
			maxLinesCount = int(s_maxLinesUnitInfo.GetValueRange().GetClipped(values.GetElement(2)));
			minDistanceToLine = s_minDistanceUnitInfo.GetValueRange().GetClipped(values.GetElement(3));
		}
		else{
			SendErrorMessage(iproc::IProcessor::MI_BAD_PARAMS, QObject::tr("Wrong grid search parameters"));
		}
	}

	ialgo::CHoughSpace2d lineSpace;
	istd::CIndex2d lineSpaceSize(angleGridSize, radiusGridSize);

	if (!lineSpace.CreateHoughSpace(lineSpaceSize, true, false, false)){
		return false;
	}

	istd::CIndex2d imageSize = image.GetImageSize();
	i2d::CVector2d imageCenter(imageSize.GetX() * 0.5, imageSize.GetY() * 0.5);

	for (		Features::ConstIterator iter1 = consumer.features.constBegin();
				iter1 != consumer.features.constEnd();
				++iter1){
		const FeatureInfo& feature1 = *iter1;

		for (		Features::ConstIterator iter2 = iter1 + 1;
					iter2 != consumer.features.constEnd();
					++iter2){
			const FeatureInfo& feature2 = *iter2;

			UpdateHoughSpace(feature1.position, feature2.position, feature1.weight * feature2.weight, imageCenter, lineSpace);
		}
	}

	lineSpace.SmoothHoughSpace(istd::CIndex2d(*m_smoothKernelAttrPtr, *m_smoothKernelAttrPtr));

	ialgo::CHoughSpace2d::StdConsumer foundLinesResults(maxLinesCount, maxLinesCount * 10, *m_smoothKernelAttrPtr, 0.25);
	lineSpace.AnalyseHoughSpace(1, foundLinesResults);
	if (foundLinesResults.positions.size() < 2){
		AddMessage(new ilog::CMessage(istd::IInformationProvider::IC_ERROR, 0, QObject::tr("No lines found"), "LensCorrectionFinder"));

		return false;
	}

	istd::TDelPtr<ilog::CExtMessage> linesMessagePtr = new ilog::CExtMessage(
				istd::IInformationProvider::IC_INFO,
				0,
				"All detected lines",
				"LensCorrectionFinder");

	AllCorrectionInfos allCorrectionInfos;

	double linePosTolerance = imageCenter.GetLength() * *m_smoothKernelAttrPtr / lineSpaceSize.GetY();

	for (		ialgo::CHoughSpace2d::StdConsumer::PosMap::ConstIterator houghIter = foundLinesResults.positions.constBegin();
				houghIter != foundLinesResults.positions.constEnd();
				++houghIter){
		const i2d::CVector2d& foundSpacePos = houghIter.value();

		i2d::CLine2d line = CalcCorrespondingLine(foundSpacePos, imageCenter, lineSpaceSize);

		QSet<i2d::CVector2d> positionsOnLine;

		for (		Features::ConstIterator featureIter = consumer.features.constBegin();
					featureIter != consumer.features.constEnd();
					++featureIter){
			const FeatureInfo& feature = *featureIter;

			if (line.GetExtendedDistance(feature.position) < linePosTolerance){
				positionsOnLine.insert(feature.position);
			}
		}

		if (positionsOnLine.size() >= 4){
			i2d::CLine2d representationLine;
			if (!representationLine.ApproxFromPoints(positionsOnLine)){
				continue;
			}

			CorrectionLineInfo lineInfo;
			lineInfo.orthoVector = representationLine.GetNearestPoint(imageCenter) - imageCenter;
			if (lineInfo.orthoVector.GetLength() < minDistanceToLine){
				continue;
			}

			for (		QSet<i2d::CVector2d>::ConstIterator pointIter = positionsOnLine.constBegin();
						pointIter != positionsOnLine.constEnd();
						++pointIter){
				const i2d::CVector2d& pos = *pointIter;

				i2d::CVector2d posOnLine;
				if (representationLine.GetExtendedIntersection(i2d::CLine2d(imageCenter, pos), posOnLine)){
					CorrectionInfo info;
					info.foundPos = pos;
					info.onLinePos = posOnLine - imageCenter;

					lineInfo.infos.push_back(info);
				}
			}

			allCorrectionInfos.push_back(lineInfo);


			i2d::CLine2d* lineObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
			*lineObjectPtr = representationLine;
			linesMessagePtr->InsertAttachedObject(lineObjectPtr, QObject::tr("Detected line %1 using %2 points at Hough position (%3, %4)")
						.arg(allCorrectionInfos.size() + 1)
						.arg(positionsOnLine.size())
						.arg(foundSpacePos.GetX())
						.arg(foundSpacePos.GetY()));
		}
	}

	if (linesMessagePtr->GetAttachedObjectsCount() > 0){
		AddMessage(linesMessagePtr.PopPtr(), MCT_TEMP);
	}

	if (allCorrectionInfos.size() < 2){
		AddMessage(new ilog::CMessage(istd::IInformationProvider::IC_ERROR, 0, QObject::tr("Not enogh usable lines found"), "LensCorrectionFinder"));

		return false;
	}

	icalib::CSimpleLensCorrection inputCorrection;
	inputCorrection.SetOpticalCenter(imageCenter);

	CalculateCalibrationFactors(allCorrectionInfos, searchCenterFlag, inputCorrection, result);

	result.SetOpticalCenter(imageCenter + result.GetOpticalCenter());

//	AddCorrectPointsMessage(allCorrectionInfos, result, imageCenter);

	ilog::CMessage* resultMessagePtr = new ilog::CMessage(
				istd::IInformationProvider::IC_INFO,
				0,
				QObject::tr("Found lens coeeficients: dist=%1, scale=%2").arg(result.GetDistortionFactor() * 1000000).arg(result.GetScaleFactor()),
				"LensCorrectionFinder");
	AddMessage(resultMessagePtr, MCT_RESULTS);

	return true;
}


i2d::CVector2d CLensCorrFindSupplierComp::CalcHoughPos(
			const i2d::CVector2d& point1,
			const i2d::CVector2d& point2,
			const i2d::CVector2d& imageCenter,
			const istd::CIndex2d& spaceSize) const
{
	i2d::CVector2d angleVector = (point2 - point1).GetNormalized();
	double directionAngle = angleVector.GetAngle();
	double distanceToCenter = angleVector.GetCrossProductZ(point1 - imageCenter);
	if (distanceToCenter >= 0){
		directionAngle = std::fmod(directionAngle + I_2PI, I_2PI);
	}
	else{
		distanceToCenter = -distanceToCenter;
		directionAngle = std::fmod(directionAngle + I_PI, I_2PI);
	}
	Q_ASSERT(directionAngle >= 0);

	return i2d::CVector2d(directionAngle / I_2PI * spaceSize.GetX(), distanceToCenter * spaceSize.GetY() / imageCenter.GetLength());
}


i2d::CLine2d CLensCorrFindSupplierComp::CalcCorrespondingLine(
			const i2d::CVector2d& houghPos,
			const i2d::CVector2d& imageCenter,
			const istd::CIndex2d& spaceSize) const
{
	double directionAngle = houghPos.GetX() * I_2PI / spaceSize.GetX();
	double distanceToCenter = houghPos.GetY() * imageCenter.GetLength() / spaceSize.GetY();

	i2d::CVector2d angleVector;
	angleVector.Init(directionAngle);

	i2d::CVector2d lineCenter = imageCenter - angleVector.GetOrthogonal() * distanceToCenter;
	i2d::CVector2d lineDir = angleVector * 100;

	return i2d::CLine2d(lineCenter - lineDir, lineCenter + lineDir);
}


void CLensCorrFindSupplierComp::UpdateHoughSpace(
			const i2d::CVector2d& point1,
			const i2d::CVector2d& point2,
			double weight,
			const i2d::CVector2d& imageCenter,
			ialgo::CHoughSpace2d& space) const
{
	istd::CIndex2d spaceSize = space.GetImageSize();

	i2d::CVector2d houghPos = CalcHoughPos(point1, point2, imageCenter, spaceSize);
	int radiusIndex = qMin(qFloor(houghPos.GetX()), spaceSize.GetX() - 1);
	Q_ASSERT(radiusIndex >= 0);

	int angleIndex = qFloor(houghPos.GetY());

	if ((angleIndex >= 0) && (angleIndex < spaceSize.GetY())){
		quint32* angleLinePtr = (quint32*)space.GetLinePtr(angleIndex);
		double kernelFactor = *m_smoothKernelAttrPtr;
		double posFactor = qSqrt(qAbs(point1.GetCrossProductZ(point2)));
		angleLinePtr[radiusIndex] += int(weight * posFactor * kernelFactor * kernelFactor);
	}
}


// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CLensCorrFindSupplierComp::ProduceObject(ProductType& result) const
{
	if (!m_bitmapProviderCompPtr.IsValid()){
		return WS_FAILED;
	}

	const iimg::IBitmap* bitmapPtr = m_bitmapProviderCompPtr->GetBitmap();
	if (bitmapPtr == nullptr){
		AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("Input image could not be provided"), "LensCorrectionFinder"));

		return WS_FAILED;
	}

	Timer performanceTimer(this, "Lens correction finder");

	if (!CalculateCalibration(*bitmapPtr, result)){
		return WS_FAILED;
	}

	return WS_OK;
}


// reimplemented (icomp::CComponentBase)

void CLensCorrFindSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	m_searchParamsContraints.Reset();
	m_searchParamsContraints.InsertValueInfo(QObject::tr("Angle Grid"), QObject::tr("Decide how exact the angle will be calculated"), "AngleGrid", s_angleGridUnitInfo);
	m_searchParamsContraints.InsertValueInfo(QObject::tr("Distance Grid"), QObject::tr("Decide how exact the position of line will be calculated"), "DistanceGrid", s_distanceGridUnitInfo);
	m_searchParamsContraints.InsertValueInfo(QObject::tr("Max lines"), QObject::tr("Maximal number of lines"), "MaxLines", s_maxLinesUnitInfo);
	m_searchParamsContraints.InsertValueInfo(QObject::tr("Min distance"), QObject::tr("Minimal distance to center"), "MinDistance",  s_minDistanceUnitInfo);

	if (m_bitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr(), m_bitmapProviderSupplierCompPtr.GetPtr());
	}
}


// public methods of embedded class FeaturesConsumer

// reimplemented (iipr::IFeaturesConsumer)

void CLensCorrFindSupplierComp::FeaturesConsumer::ResetFeatures()
{
	features.clear();
}


bool CLensCorrFindSupplierComp::FeaturesConsumer::AddFeature(const imeas::INumericValue* featurePtr, bool* /*isFullPtr*/)
{
	if (featurePtr != nullptr){
		const i2d::CPosition2d* positionPtr = dynamic_cast<const i2d::CPosition2d*>(featurePtr);
		if (positionPtr != nullptr){
			FeatureInfo info;

			info.position = positionPtr->GetPosition();
			info.weight = 1;

			features.push_back(info);
		}
		else{
			imath::CVarVector vector = featurePtr->GetValues();
			if (vector.GetElementsCount() >= 2){
				FeatureInfo info;

				info.position = i2d::CVector2d(vector[0], vector[1]);
				if (vector.GetElementsCount() > 2){
					info.weight = vector[2];
				}
				else{
					info.weight = 1;
				}

				features.push_back(info);
			}
		}
	}

	delete featurePtr;

	return true;
}


// static attributes
imath::CFixedPointManip CLensCorrFindSupplierComp::s_integerManip(0);
imath::CGeneralUnitInfo CLensCorrFindSupplierComp::s_angleGridUnitInfo(imath::IUnitInfo::UT_COUNTER, "", 1, istd::CRange(10, 10000), &s_integerManip);
imath::CGeneralUnitInfo CLensCorrFindSupplierComp::s_distanceGridUnitInfo(imath::IUnitInfo::UT_COUNTER, "", 1, istd::CRange(10, 10000), &s_integerManip);
imath::CGeneralUnitInfo CLensCorrFindSupplierComp::s_maxLinesUnitInfo(imath::IUnitInfo::UT_COUNTER, "", 1, istd::CRange(2, 100), &s_integerManip);
imath::CGeneralUnitInfo CLensCorrFindSupplierComp::s_minDistanceUnitInfo(imath::IUnitInfo::UT_COUNTER, QObject::tr("px"), 1, istd::CRange(1, 1000), &s_integerManip);

} // namespace iipr


