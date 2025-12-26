#include <iipr/CHoughLineFinderComp.h>


// ACF include
#include <iimg/CScanlineMask.h>
#include <iimg/CGeneralBitmap.h>
#include <iprm/TParamsPtr.h>
#include <ilog/CExtMessage.h>


namespace iipr
{


// reimplemented (iipr::IImageToFeatureProcessor)

iproc::IProcessor::TaskState CHoughLineFinderComp::DoExtractFeatures(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& image,
			IFeaturesConsumer& results,
			ibase::IProgressManager* progressManagerPtr)
{
	ibase::CSize size = image.GetImageSize();
	if ((size.GetX() < 3) || (size.GetY() < 3)){
		SendErrorMessage(0, QObject::tr("Image too small to calculate edges"));

		return TS_INVALID;
	}

	if (!CreateHoughSpace()){
		return TS_INVALID;
	}

	// calculate parameters
	int directionThreshold2 = int(255 * 255 * qMin(*m_defaultMinEdgeStrengthAttrPtr, 1.0));
	double radiusOffset = m_houghSpace.GetImageSize().GetX() * 0.5;

	iimg::CScanlineMask mask;

	i2d::CRect clipArea(size);
	iprm::TParamsPtr<i2d::IObject2d> aoiObjectPtr(paramsPtr, m_aoiParamIdAttrPtr, m_defaultAoiCompPtr, false);
	if (aoiObjectPtr.IsValid()){
		mask.SetCalibration(image.GetCalibration());

		mask.CreateFromGeometry(*aoiObjectPtr.GetPtr(), &clipArea);
	}
	else{
		mask.CreateFilled(clipArea);
	}

	// simple erosion
	mask.Intersection(mask.GetTranslated(-1, 0));
	mask.Intersection(mask.GetTranslated(1, 0));
	mask.Intersection(mask.GetTranslated(0, -1));
	mask.Intersection(mask.GetTranslated(0, 1));

	i2d::CVector2d imageCenter = mask.GetCenter();

	auto progressLoggerPtr = StartProgressLogger(progressManagerPtr, true);

	const quint8* prevLinePtr = (const quint8*)image.GetLinePtr(0);
	const quint8* linePtr = (const quint8*)image.GetLinePtr(1);
	for (int y = 1; y < size.GetY() - 1; ++y){
		const quint8* nextLinePtr = (const quint8*)image.GetLinePtr(y + 1);

		const istd::CIntRanges* inputRangesPtr = mask.GetPixelRanges(y);
		if (inputRangesPtr != nullptr){
			istd::CIntRanges::RangeList rangeList;
			inputRangesPtr->GetAsList(clipArea.GetHorizontalRange(), rangeList);

			for (		istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter){
				const istd::CIntRange& rangeH = *iter;

				for (int x = rangeH.GetMinValue(); x < rangeH.GetMaxValue(); ++x){
					int leftPixel = linePtr[x - 1];
					int rightPixel = linePtr[x + 1];
					int topPixel = prevLinePtr[x];
					int bottomPixel = nextLinePtr[x];

					int diffX = rightPixel - leftPixel;
					int diffY = bottomPixel - topPixel;

					if (diffX * diffX + diffY * diffY > directionThreshold2){
						i2d::CVector2d position(x - imageCenter.GetX() + 0.5, y - imageCenter.GetY() + 0.5);
						i2d::CVector2d direction(diffX, diffY);

						UpdateHoughSpace(position, direction, radiusOffset);
					}
				}
			}
		}

		prevLinePtr = linePtr;
		linePtr = nextLinePtr;

		if (progressLoggerPtr != nullptr){
			progressLoggerPtr->OnProgress(double(y) / size.GetY());

			if (progressLoggerPtr->IsCanceled()){
				break;
			}
		}
	}

	m_houghSpace.SmoothHoughSpace(istd::CIndex2d(*m_defaultSmoothKernelAttrPtr, *m_defaultSmoothKernelAttrPtr));

	ialgo::CHoughSpace2d::StdConsumer posResults(*m_defaultMaxLinesAttrPtr, *m_defaultMaxLinesAttrPtr * 10, 10.0, 0.5);
	m_houghSpace.AnalyseHoughSpace(100, posResults);

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

			if (m_tempConsumerCompPtr.IsValid()){
				ilog::CExtMessage* houghPosMessagePtr = new ilog::CExtMessage(
							istd::IInformationProvider::IC_INFO,
							FOUND_LINE,
							QObject::tr("Hough Position (%1, %2) with weight %3").arg(houghPos.GetX()).arg(houghPos.GetY()).arg(weight),
							"LineFinder");
				i2d::CPosition2d* houghPosMessageObjectPtr = new imod::TModelWrap<i2d::CPosition2d>();
				houghPosMessageObjectPtr->SetPosition(houghPos);
				houghPosMessagePtr->InsertAttachedObject(houghPosMessageObjectPtr);

				m_tempConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(houghPosMessagePtr));
			}

			LineFeature* featurePtr = new LineFeature();

			i2d::CVector2d direction;
			direction.Init((houghPos.GetY() - 0.5) * I_2PI / m_angleVectors.size());
			i2d::CVector2d centralPoint = direction * (houghPos.GetX() - radiusOffset);

			featurePtr->SetPoint1(imageCenter + centralPoint - direction.GetOrthogonal() * 100);
			featurePtr->SetPoint2(imageCenter + centralPoint + direction.GetOrthogonal() * 100);
			featurePtr->SetWeight(weight);

			if (m_resultConsumerCompPtr.IsValid()){
				ilog::CExtMessage* pointMessagePtr = new ilog::CExtMessage(
							istd::IInformationProvider::IC_INFO,
							FOUND_LINE,
							QObject::tr("Line %1, angle %2").arg(lineIndex + 1).arg(direction.GetAngle() * 360 / I_2PI),
							"LineFinder");
				i2d::CLine2d* pointMessageObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
				pointMessageObjectPtr->SetPoint1(featurePtr->GetPoint1());
				pointMessageObjectPtr->SetPoint2(featurePtr->GetPoint2());
				pointMessagePtr->InsertAttachedObject(pointMessageObjectPtr);

				m_resultConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(pointMessagePtr));
			}

			if (m_tempConsumerCompPtr.IsValid()){
				ilog::CExtMessage* pointMessagePtr = new ilog::CExtMessage(
							istd::IInformationProvider::IC_INFO,
							FOUND_LINE,
							QObject::tr("Auxiliary line %1").arg(lineIndex + 1),
							"LineFinder");
				i2d::CLine2d* pointMessageObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
				pointMessageObjectPtr->SetPoint1(featurePtr->GetCenter());
				pointMessageObjectPtr->SetPoint2(imageCenter);
				pointMessagePtr->InsertAttachedObject(pointMessageObjectPtr);

				m_tempConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(pointMessagePtr));
			}

			results.AddFeature(featurePtr);
		}
	}

	return TS_OK;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CHoughLineFinderComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr)
{
	if (outputPtr == nullptr){
		return TS_OK;
	}

	const iimg::IBitmap* imagePtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	IFeaturesConsumer* consumerPtr = dynamic_cast<IFeaturesConsumer*>(outputPtr);

	if (		(imagePtr == nullptr) ||
				(consumerPtr == nullptr)){
		return TS_INVALID;
	}

	return DoExtractFeatures(paramsPtr, *imagePtr, *consumerPtr, progressManagerPtr);
}


// protected methods

bool CHoughLineFinderComp::CreateHoughSpace()
{
	int angleGridSize = *m_defaultAngleResAttrPtr;
	int radiusGridSize = *m_defaultRadiusResAttrPtr;

	if (m_houghSpace.CreateHoughSpace(istd::CIndex2d(radiusGridSize, angleGridSize), false, true, false)){
		m_angleVectors.resize(angleGridSize);

		for (int i = 0; i < angleGridSize; ++i){
			double angle = i * I_2PI / angleGridSize;

			m_angleVectors[i].Init(angle);
		}

		return true;
	}
	else{
		m_angleVectors.clear();

		return false;
	}
}


void CHoughLineFinderComp::UpdateHoughSpace(const i2d::CVector2d& position, const i2d::CVector2d& direction, double radiusOffset)
{
	istd::CIndex2d spaceSize = m_houghSpace.GetImageSize();

	double directionAngle = direction.GetAngle();
	int firstAngleIndex = int((directionAngle / I_2PI + 1.95) * spaceSize.GetY() + 1) % spaceSize.GetY();
	int lastAngleIndex = int((directionAngle / I_2PI + 2.05) * spaceSize.GetY()) % spaceSize.GetY();

	for (int angleIndex = firstAngleIndex; angleIndex != lastAngleIndex; angleIndex = (angleIndex + 1) % spaceSize.GetY()){
		quint32* angleLinePtr = (quint32*)m_houghSpace.GetLinePtr(angleIndex);

		const i2d::CVector2d& angleVector = m_angleVectors[angleIndex];

		double radius = angleVector.GetDotProduct(position);

		int radiusIndex = qFloor(radius + radiusOffset);
		if ((radiusIndex >= 0) && (radiusIndex < spaceSize.GetX())){
			if (radiusIndex < spaceSize.GetX()){
				double weight = angleVector.GetDotProduct(direction);
				Q_ASSERT(weight >= 0);

				angleLinePtr[radiusIndex] += int(weight);
			}
		}
	}
}


// reimplemented (icomp::CComponentBase)

void CHoughLineFinderComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	CreateHoughSpace();
}


} // namespace iipr


