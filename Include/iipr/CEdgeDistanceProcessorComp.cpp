// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CEdgeDistanceProcessorComp.h>


// ACF includes
#include <i2d/CAnnulusSegment.h>
#include <iprm/CParamsSet.h>
#include <iprm/TParamsPtr.h>

// IACF includes
#include <iipr/CCaliperFeature.h>
#include <iipr/CCaliperParams.h>
#include <iipr/CFeaturesContainer.h>
#include <iipr/CSingleFeatureConsumer.h>
#include <iipr/CCaliperDistanceFeature.h>


namespace iipr
{


// reimplemented (iipr::IImageToFeatureProcessor)

iproc::IProcessor::TaskState CEdgeDistanceProcessorComp::DoExtractFeatures(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& image,
			IFeaturesConsumer& results,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (!m_featuresMapperCompPtr.IsValid() || (paramsPtr == NULL)){
		return TS_INVALID;
	}

	results.ResetFeatures();

	CaliperLines caliperLines;

	iprm::TParamsPtr<istd::IChangeable> aoiObjectPtr(paramsPtr, *m_aoiParamIdAttrPtr);
	if (!aoiObjectPtr.IsValid()){
		return TS_INVALID;
	}

	i2d::CLine2d projectionLine;
	iprm::CParamsSet extendedParamsSet;
	extendedParamsSet.SetEditableParameter(*m_slaveLineIdAttrPtr, &projectionLine);
	extendedParamsSet.SetSlaveSet(paramsPtr);

	i2d::CVector2d center;

	if (!CalculateCaliperLines(*aoiObjectPtr, extendedParamsSet, image, caliperLines, projectionLine, center)){
		return TS_INVALID;
	}

	int foundLinesCount = int(caliperLines.size());

	auto progressLoggerPtr = StartProgressLogger(progressManagerPtr, true);

	for (int lineIndex = 0; lineIndex < foundLinesCount; lineIndex++){
		if (progressLoggerPtr != nullptr){
			Q_ASSERT(progressManagerPtr != NULL);

			progressLoggerPtr->OnProgress(double(lineIndex) / foundLinesCount);

			if (progressLoggerPtr->IsCanceled()){
				return TS_CANCELED;
			}
		}

		CaliperLine& caliperLine = caliperLines[lineIndex];

		// Min. weight of both caliper points is taken as weight of the found distance:
		double featureWeight = qMin(caliperLine.points[0].weight, caliperLine.points[1].weight);

		CCaliperDistanceFeature* caliperDistanceFeature = new CCaliperDistanceFeature(i2d::CLine2d(caliperLine.points[0].position, caliperLine.points[1].position), featureWeight);

		results.AddFeature(caliperDistanceFeature);
	}

	return TS_OK;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CEdgeDistanceProcessorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	const iimg::IBitmap* imagePtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	IFeaturesConsumer* consumerPtr = dynamic_cast<IFeaturesConsumer*>(outputPtr);

	if (		(imagePtr == NULL) ||
				(consumerPtr == NULL)){
		return TS_INVALID;
	}

	return DoExtractFeatures(paramsPtr, *imagePtr, *consumerPtr, progressManagerPtr);
}


// protected methods

bool CEdgeDistanceProcessorComp::CalculateCaliperLines(
			const istd::IChangeable& aoiObject,
			const iprm::IParamsSet& params,
			const iimg::IBitmap& image,
			CaliperLines& caliperLines,
			i2d::CLine2d& projectionLine,
			i2d::CVector2d& center)
{
	const iprm::CParamsSet* setPtr = dynamic_cast<const iprm::CParamsSet*>(&aoiObject);
	if (setPtr != NULL){
		int aoisCount = 0;

		const iprm::CParamsSet::ParameterInfos& infos = setPtr->GetParameterInfos();
		int paramsCount = infos.GetCount();
		for (int i = 0; i < paramsCount; ++i){
			const iprm::CParamsSet::ParameterInfo* infoPtr = infos.GetAt(i);
			if ((infoPtr != NULL) && infoPtr->parameterPtr.IsValid()){
				if (!CalculateCaliperLines(*infoPtr->parameterPtr, params, image, caliperLines, projectionLine, center)){
					return false;
				}

				++aoisCount;
			}
		}

		return (aoisCount > 0);
	}

	iipr::CCaliperParams workingCaliperParams;
	iprm::CParamsSet workingCaliperParamsSet;
	workingCaliperParamsSet.SetEditableParameter(*m_slaveCaliperParamsIdAttrPtr, &workingCaliperParams);
	workingCaliperParamsSet.SetSlaveSet(&params);

	const i2d::CLine2d* lineAoiPtr = dynamic_cast<const i2d::CLine2d*>(&aoiObject);
	if (lineAoiPtr != NULL){
		// Set projection line for caliper calculation:
		projectionLine.CopyFrom(*lineAoiPtr);
		projectionLine.SetCalibration(lineAoiPtr->GetCalibration());

		CaliperLine caliperLine;

		// Do caliper calculation in backward direction:
		if (!CalculateCaliper(workingCaliperParamsSet, workingCaliperParams, ICaliperParams::DM_BACKWARD, image, caliperLine)){
			return false;
		}

		// Do caliper calculation in forward direction:
		if (!CalculateCaliper(workingCaliperParamsSet, workingCaliperParams, ICaliperParams::DM_FORWARD, image, caliperLine)){
			return false;
		}

		caliperLines.push_back(caliperLine);

		return true;
	}

	const i2d::CAnnulus* annulusAoiPtr = dynamic_cast<const i2d::CAnnulus*>(&aoiObject);
	if (annulusAoiPtr != NULL){
		projectionLine.SetCalibration(annulusAoiPtr->GetCalibration());

		double beginAngle = 0;
		double endAngle = 2 * I_PI;
		const i2d::CAnnulusSegment* segmentPtr = dynamic_cast<const i2d::CAnnulusSegment*>(annulusAoiPtr);
		if (segmentPtr != NULL){
			beginAngle = segmentPtr->GetBeginAngle();
			endAngle = segmentPtr->GetEndAngle();
		}

		double minRadius = annulusAoiPtr->GetInnerRadius();
		double maxRadius = annulusAoiPtr->GetOuterRadius();
		center = annulusAoiPtr->GetPosition();

		int stepsCount = int((minRadius + maxRadius) * (endAngle - beginAngle) * 0.5 + 1);

		for (int i = 0; i < stepsCount; ++i){
			CaliperLine caliperLine;

			double alpha = (i + 0.5) / stepsCount;
			double angle = alpha * (endAngle - beginAngle) + beginAngle;

			i2d::CVector2d directionVector;
			directionVector.Init(angle);

			projectionLine.SetPoint1(center + directionVector * minRadius);
			projectionLine.SetPoint2(center + directionVector * maxRadius);

			// Do caliper calculation in backward direction:
			if (!CalculateCaliper(workingCaliperParamsSet, workingCaliperParams, ICaliperParams::DM_BACKWARD, image, caliperLine)){
				return false;
			}

			// Do caliper calculation in forward direction:
			if (!CalculateCaliper(workingCaliperParamsSet, workingCaliperParams, ICaliperParams::DM_FORWARD, image, caliperLine)){
				return false;
			}

			caliperLines.push_back(caliperLine);
		}

		return true;
	}

	return false;
}


bool CEdgeDistanceProcessorComp::CalculateCaliper(
			const iprm::IParamsSet& params,
			iipr::ICaliperParams& workingCaliperParams,
			ICaliperParams::DirectionMode caliperDirectionMode,
			const iimg::IBitmap& image,
			CaliperLine& caliperLine) const
{
	CSingleFeatureConsumer consumer(CSingleFeatureConsumer::FP_FIRST);

	consumer.ResetFeatures();

	workingCaliperParams.SetDirectionMode(caliperDirectionMode);
	int caliperResult = m_slaveProcessorCompPtr->DoProcessing(&params, &image, &consumer);
	if (caliperResult != TS_OK){
		return false;
	}

	SetCaliperResults(params, consumer, caliperDirectionMode, caliperLine);

	return true;
}


void CEdgeDistanceProcessorComp::SetCaliperResults(
			const iprm::IParamsSet& params,
			const IFeaturesProvider& container,
			ICaliperParams::DirectionMode caliperDirectionMode,
			CaliperLine& caliperLine) const
{
	Q_ASSERT(m_featuresMapperCompPtr.IsValid());	// validity of features mapper should be checked on the beginning

	int featuresCount = container.GetFeaturesCount();
	Q_ASSERT ((featuresCount == 0) || (featuresCount == 1));

	if (featuresCount > 0){ 
		const CCaliperFeature* featurePtr = dynamic_cast<const CCaliperFeature*>(&container.GetFeature(0));
		if (featurePtr != NULL){
			Point point;
			point.weight = featurePtr->GetWeight();
			m_featuresMapperCompPtr->GetImagePosition(*featurePtr, &params, point.position);

			if (caliperDirectionMode == ICaliperParams::DM_FORWARD){
				caliperLine.points[0] = point;
			}
			else if (caliperDirectionMode == ICaliperParams::DM_BACKWARD){
				caliperLine.points[1] = point;
			}
		}
	}
}


} // namespace iipr


