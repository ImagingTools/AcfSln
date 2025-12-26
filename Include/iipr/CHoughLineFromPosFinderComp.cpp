#include <iipr/CHoughLineFromPosFinderComp.h>


// ACF include
#include <iimg/CScanlineMask.h>
#include <iimg/CGeneralBitmap.h>
#include <iprm/TParamsPtr.h>
#include <ilog/CExtMessage.h>
#include <i2d/CDirection2d.h>


namespace iipr
{


// reimplemented (iipr::IFeatureToFeatureProcessor)

iproc::IProcessor::TaskState CHoughLineFromPosFinderComp::DoConvertFeatures(
			const iprm::IParamsSet* paramsPtr,
			const IFeaturesProvider& container,
			IFeaturesConsumer& results)
{
	int maxLines = -1;

	iprm::TParamsPtr<imeas::INumericValue> maxLinesParamPtr(paramsPtr, m_maxLinesParamIdAttrPtr, m_defaultMaxLinesParamCompPtr, false);
	if (maxLinesParamPtr.IsValid()){
		imath::CVarVector maxLinesValues = maxLinesParamPtr->GetValues();
		if (maxLinesValues.GetElementsCount() > 0){
			maxLines = int(maxLinesValues.GetElement(0));
		}
	}

	i2d::CPolypoint positions;

	int featuresCount = container.GetFeaturesCount();
	for (int i = 0; i < featuresCount; ++i){
		const imeas::INumericValue& feature = container.GetFeature(i);

		const i2d::CPosition2d* positionPtr = dynamic_cast<const i2d::CPosition2d*>(&feature);
		if (positionPtr != nullptr){
			positions.InsertNode(positionPtr->GetPosition());
		}
		else{
			imath::CVarVector vector = feature.GetValues();
			if (vector.GetElementsCount() >= 2){
				positions.InsertNode(i2d::CVector2d(vector[0], vector[1]));
			}
		}
	}

	if (FindLines(paramsPtr, maxLines, positions, results)){
		return TS_OK;
	}
	else{
		return TS_INVALID;
	}
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CHoughLineFromPosFinderComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/)
{
	if (outputPtr == nullptr){
		return TS_OK;
	}

	const IFeaturesProvider* providerPtr = dynamic_cast<const IFeaturesProvider*>(inputPtr);
	IFeaturesConsumer* consumerPtr = dynamic_cast<IFeaturesConsumer*>(outputPtr);

	if (		(providerPtr == nullptr) ||
				(consumerPtr == nullptr)){
		return TS_INVALID;
	}

	return DoConvertFeatures(paramsPtr, *providerPtr, *consumerPtr/*, progressManagerPtr*/);
}


// protected methods

int CHoughLineFromPosFinderComp::FindLines(
			const iprm::IParamsSet* paramsPtr,
			int maxLines,
			const i2d::CPolypoint& points,
			IFeaturesConsumer& results)
{
	m_direction = i2d::CVector2d(1, 0);
	m_directionAngleTolerance = -1;

	iprm::TParamsPtr<i2d::IObject2d> directionObjectPtr(paramsPtr, m_directionParamIdAttrPtr, m_defaultDirectionParamCompPtr, false);
	if (directionObjectPtr.IsValid()){
		const i2d::CLine2d* linePtr = dynamic_cast<const i2d::CLine2d*>(directionObjectPtr.GetPtr());
		if (linePtr != nullptr){
			m_direction = linePtr->GetDiffVector().GetNormalized();
			m_directionAngleTolerance = I_PI * 0.1;
		}
		else{
			const i2d::CPosition2d* posPtr = dynamic_cast<const i2d::CPosition2d*>(directionObjectPtr.GetPtr());
			if (posPtr != nullptr){
				m_direction = posPtr->GetPosition().GetNormalized();
				m_directionAngleTolerance = I_PI * 0.1;
			}
		}
	}

	if (!CreateHoughSpace()){
		return TS_INVALID;
	}

	i2d::CVector2d areaCenter = points.GetBoundingBox().GetCenter();

	// iterate over all pairs
	int pointsCount = points.GetNodesCount();
	for (int point1Index = 0; point1Index < pointsCount - 1; ++point1Index){
		i2d::CVector2d point1 = points.GetNodePos(point1Index) - areaCenter;

		for (int point2Index = point1Index + 1; point2Index < pointsCount; ++point2Index){
			i2d::CVector2d point2 = points.GetNodePos(point2Index) - areaCenter;

			UpdateHoughSpace(point1, point2);
		}
	}

	m_houghSpace.SmoothHoughSpace(istd::CIndex2d(*m_defaultSmoothKernelAttrPtr, *m_defaultSmoothKernelAttrPtr));

	ialgo::CHoughSpace2d::StdConsumer posResults(maxLines, maxLines * 10, 20.0, 0.01);
	m_houghSpace.AnalyseHoughSpace(10, posResults);

	if (m_tempConsumerCompPtr.IsValid()){
		ilog::CExtMessage* spaceMessagePtr = new ilog::CExtMessage(
					istd::IInformationProvider::IC_NONE,
					HOUGH_SPACE,
					QString("Hough space"),
					"LineFinder");

		iimg::CGeneralBitmap* spaceMessageObjectPtr = new imod::TModelWrap<iimg::CGeneralBitmap>();
		spaceMessagePtr->InsertAttachedObject(spaceMessageObjectPtr);

		m_houghSpace.ExtractToBitmap(*spaceMessageObjectPtr);

		m_tempConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(spaceMessagePtr));
	}

	if (!posResults.positions.isEmpty()){
		ialgo::CHoughSpace2d::StdConsumer::PosMap::ConstIterator resultIter = posResults.positions.constBegin();
		double bestWeight = resultIter.key();

		int lineIndex = 0;
		for (; resultIter != posResults.positions.constEnd(); ++resultIter, ++lineIndex){
			const i2d::CVector2d& houghPos = resultIter.value();

			double weight = resultIter.key() / bestWeight;

			i2d::CLine2d line = CalcCorrespondingLine(houghPos);

			LineFeature* featurePtr = new LineFeature();
			featurePtr->SetPoint1(line.GetPoint1() + areaCenter);
			featurePtr->SetPoint2(line.GetPoint2() + areaCenter);
			featurePtr->SetWeight(weight);

			if (m_resultConsumerCompPtr.IsValid()){
				ilog::CExtMessage* pointMessagePtr = new ilog::CExtMessage(
							istd::IInformationProvider::IC_INFO,
							FOUND_LINE,
							QString("Line %1, angle %2").arg(lineIndex + 1).arg(i2d::CDirection2d(featurePtr->GetDiffVector()).ToDegree()),
							"LineFinder");
				i2d::CLine2d* pointMessageObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
				pointMessageObjectPtr->SetPoint1(featurePtr->GetPoint1());
				pointMessageObjectPtr->SetPoint2(featurePtr->GetPoint2());
				pointMessagePtr->InsertAttachedObject(pointMessageObjectPtr);

				m_resultConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(pointMessagePtr));
			}

			bool isFull = false;
			results.AddFeature(featurePtr, &isFull);

			if (isFull){
				break;
			}
		}
	}

	return TS_OK;
}


bool CHoughLineFromPosFinderComp::CreateHoughSpace()
{
	int angleGridSize = *m_defaultAngleResAttrPtr;
	int radiusGridSize = *m_defaultRadiusResAttrPtr;

	return m_houghSpace.CreateHoughSpace(istd::CIndex2d(angleGridSize, radiusGridSize), true, false, false);
}


i2d::CVector2d CHoughLineFromPosFinderComp::CalcHoughPos(const i2d::CVector2d& point1, const i2d::CVector2d& point2) const
{
	i2d::CVector2d angleVector = (point2 - point1).GetNormalized();
	double directionAngle = angleVector.GetAngle();
	double distanceToCenter = angleVector.GetCrossProductZ(point1);
	if (distanceToCenter >= 0){
		directionAngle = std::fmod(directionAngle + I_PI, I_2PI);
	}
	else{
		distanceToCenter = -distanceToCenter;
		directionAngle = std::fmod(directionAngle + I_2PI, I_2PI);
	}
	Q_ASSERT(directionAngle >= 0);

	istd::CIndex2d spaceSize = m_houghSpace.GetImageSize();
	return i2d::CVector2d(directionAngle / I_2PI * spaceSize.GetX(), distanceToCenter);
}


i2d::CLine2d CHoughLineFromPosFinderComp::CalcCorrespondingLine(const i2d::CVector2d& houghPos) const
{
	istd::CIndex2d spaceSize = m_houghSpace.GetImageSize();

	double directionAngle = houghPos.GetX() * I_2PI / spaceSize.GetX();
	double distanceToCenter = houghPos.GetY();

	i2d::CVector2d angleVector;
	angleVector.Init(directionAngle);

	i2d::CVector2d lineCenter = angleVector.GetOrthogonal() * distanceToCenter;
	i2d::CVector2d lineDir = angleVector * 1000;

	return i2d::CLine2d(lineCenter - lineDir, lineCenter + lineDir);
}


void CHoughLineFromPosFinderComp::UpdateHoughSpace(const i2d::CVector2d& point1, const i2d::CVector2d& point2)
{
	i2d::CVector2d direction = point2 - point1;

	i2d::CVector2d houghPos = CalcHoughPos(point1, point2);

	if (m_directionAngleTolerance >= 0){
		if (i2d::CDirection2d(m_direction).DistInRadian(i2d::CDirection2d(direction)) > m_directionAngleTolerance){
			return;
		}
	}

	m_houghSpace.IncreaseValueAt(houghPos, 1000000 / direction.GetLength());
}


// reimplemented (icomp::CComponentBase)

void CHoughLineFromPosFinderComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	CreateHoughSpace();
}


} // namespace iipr


