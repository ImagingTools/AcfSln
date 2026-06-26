// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CCircleFindProcessorComp.h>


// Qt includes
#include <QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtCore/QtMath>
#else
#include <QtCore/qmath.h>
#endif

// ACF includes
#include <imath/CVarMatrix.h>
#include <ilog/CExtMessage.h>
#include <i2d/CAffine2d.h>
#include <i2d/CAnnulusSegment.h>
#include <iprm/CParamsSet.h>
#include <iprm/TParamsPtr.h>
#include <iprm/IVariableParam.h>

// ACF Solutions includes
#include <iipr/CCaliperFeature.h>
#include <iipr/CFeaturesContainer.h>
#include <iipr/CSingleFeatureConsumer.h>
#include <iinsp/CSupplierCompBase.h>


namespace iipr
{


CCircleFindProcessorComp::CCircleFindProcessorComp()
{
}


// reimplemented (iipr::IImageToFeatureProcessor)

iproc::IProcessor::TaskState CCircleFindProcessorComp::DoExtractFeatures(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& image,
			IFeaturesConsumer& results,
			ibase::IProgressManager* /*progressManagerPtr*/)
{
	if (!m_featuresMapperCompPtr.IsValid() || (paramsPtr == NULL)){
		return TS_INVALID;
	}

	Rays inRays;
	Rays outRays;

	iprm::TParamsPtr<istd::IChangeable> aoiObjectPtr(paramsPtr, *m_aoiParamIdAttrPtr);
	if (!aoiObjectPtr.IsValid()){
		SendErrorMessage(0, QObject::tr("Search region for the circle was not defined"));

		return TS_INVALID;
	}

	const istd::IChangeable* aoiPtr = aoiObjectPtr.GetPtr();
	const iprm::IVariableParam* variableAoiPtr = dynamic_cast<const iprm::IVariableParam*>(aoiPtr);
	if (variableAoiPtr != NULL){
		aoiPtr = variableAoiPtr->GetParameter();
	}

	if (aoiPtr == NULL){
		SendErrorMessage(0, QObject::tr("Search region for the circle was not defined"));

		return TS_INVALID;
	}

	iprm::TParamsPtr<iipr::ICircleFinderParams> circleFinderParamsPtr(paramsPtr, *m_circleFinderParamsIdAttrPtr);
	if (!circleFinderParamsPtr.IsValid()){
		SendErrorMessage(0, QObject::tr("Circle finder parameters were not set"));

		return TS_INVALID;
	}

	iprm::CParamsSet extendedParamsSet;
	i2d::CLine2d projectionLine;
	extendedParamsSet.SetEditableParameter(*m_slaveLineIdAttrPtr, &projectionLine);
	extendedParamsSet.SetSlaveSet(paramsPtr);

	i2d::CVector2d center;
	int totalRaysCount = 0;

	if (!AddAoiToRays(*aoiPtr, extendedParamsSet, image, *circleFinderParamsPtr, inRays, outRays, projectionLine, center, totalRaysCount)){
		return TS_INVALID;
	}

	if (*m_searchForAnnulusAttrPtr){
		istd::TDelPtr<AnnulusFeature> featurePtr(new AnnulusFeature());

		bool isOk = CalculateAnnulus(center, inRays, outRays, totalRaysCount, *featurePtr);
		AddIntermediateResults(inRays);
		AddIntermediateResults(outRays);

		if (isOk){
			if (m_resultConsumerCompPtr.IsValid()){
				i2d::CVector2d position = featurePtr->GetPosition();
				double radius = featurePtr->GetInnerRadius();
				double radius2 = featurePtr->GetOuterRadius();
				ilog::CExtMessage* resultMessagePtr = new ilog::CExtMessage(
								istd::IInformationProvider::IC_INFO,
								iinsp::CSupplierCompBase::MI_INTERMEDIATE,
								QObject::tr("Found annulus at (%1, %2) with radii %3; %4").arg(position.GetX(), 0, 'g', 3).arg(position.GetY(), 0, 'g', 3).arg(radius, 0, 'g', 3).arg(radius2, 0, 'g', 3),
								"CircleFinder");
				i2d::CAnnulus* resultObjectPtr = new imod::TModelWrap<i2d::CAnnulus>();
				resultObjectPtr->CopyFrom(*featurePtr, istd::IChangeable::CM_CONVERT);
				resultMessagePtr->InsertAttachedObject(resultObjectPtr);
				m_resultConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(resultMessagePtr));
			}

			results.AddFeature(featurePtr.PopPtr());

			return TS_OK;
		}
		else{
			SendErrorMessage(0, QObject::tr("Not enough points found"));
		}
	}
	else{
		istd::TDelPtr<CircleFeature> featurePtr(new CircleFeature());
		Rays& usedRays = (inRays.size() >= outRays.size())? inRays: outRays;

		bool isOk = CalculateCircle(
					center,
					circleFinderParamsPtr->IsOutlierEliminationEnabled(),
					circleFinderParamsPtr->GetMinOutlierDistance(),
					usedRays,
					totalRaysCount,
					*featurePtr);

		AddIntermediateResults(usedRays);

		if (isOk){
			if (m_resultConsumerCompPtr.IsValid()){
				i2d::CVector2d position = featurePtr->GetPosition();
				double radius = featurePtr->GetRadius();
				ilog::CExtMessage* resultMessagePtr = new ilog::CExtMessage(
								istd::IInformationProvider::IC_INFO,
								iinsp::CSupplierCompBase::MI_INTERMEDIATE,
								QObject::tr("Found circle at (%1, %2) with radius %3").arg(position.GetX(), 0, 'g', 3).arg(position.GetY(), 0, 'g', 3).arg(radius, 0, 'g', 3),
								"CircleFinder");
				i2d::CCircle* resultObjectPtr = new imod::TModelWrap<i2d::CCircle>();
				resultObjectPtr->CopyFrom(*featurePtr, istd::IChangeable::CM_CONVERT);
				resultMessagePtr->InsertAttachedObject(resultObjectPtr);
				m_resultConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(resultMessagePtr));
			}

			results.AddFeature(featurePtr.PopPtr());

			return TS_OK;
		}
		else{
			SendErrorMessage(0, QObject::tr("Not enough points found"));
		}
	}

	return TS_INVALID;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CCircleFindProcessorComp::DoProcessing(
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

bool CCircleFindProcessorComp::AddAoiToRays(
			const istd::IChangeable& aoiObject,
			const iprm::IParamsSet& params,
			const iimg::IBitmap& image,
			const iipr::ICircleFinderParams& circleFinderParams,
			Rays& inRays,
			Rays& outRays,
			i2d::CLine2d& projectionLine,
			i2d::CVector2d& center,
			int& totalRaysCount)
{
	totalRaysCount = 0;
	const iprm::CParamsSet* setPtr = dynamic_cast<const iprm::CParamsSet*>(&aoiObject);
	if (setPtr != NULL){
		int aoisCount = 0;

		const iprm::CParamsSet::ParameterInfos& infos = setPtr->GetParameterInfos();
		int paramsCount = infos.GetCount();
		for (int i = 0; i < paramsCount; ++i){
			const iprm::CParamsSet::ParameterInfo* infoPtr = infos.GetAt(i);
			if ((infoPtr != NULL) && infoPtr->parameterPtr.IsValid()){
				int raysCount = 0;

				if (!AddAoiToRays(*infoPtr->parameterPtr, params, image, circleFinderParams, inRays, outRays, projectionLine, center, raysCount)){
					return false;
				}

				totalRaysCount += raysCount;

				++aoisCount;
			}
		}

		return (aoisCount > 0);
	}

	const i2d::CAnnulus* annulusAoiPtr = dynamic_cast<const i2d::CAnnulus*>(&aoiObject);
	if (annulusAoiPtr != NULL){
		const i2d::ICalibration2d* aoiCalibrationPtr = annulusAoiPtr->GetCalibration();
		i2d::CLine2d aoiLine;
		aoiLine.SetCalibration(aoiCalibrationPtr);

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

		istd::TDelPtr<IFeaturesProvider> caliperFeaturesProviderPtr;
		IFeaturesConsumer* caliperFeaturesConsumerPtr;

		if (circleFinderParams.GetCaliperMode() == ICircleFinderParams::CCM_BEST){
			CFeaturesContainer* containerPtr = new CFeaturesContainer;

			caliperFeaturesProviderPtr.SetPtr(containerPtr);
			caliperFeaturesConsumerPtr = containerPtr;
		}
		else{
			CSingleFeatureConsumer* containerPtr = new CSingleFeatureConsumer(CSingleFeatureConsumer::FP_FIRST);

			caliperFeaturesProviderPtr.SetPtr(containerPtr);
			caliperFeaturesConsumerPtr = containerPtr;
		}

		double middleBowBitmapLength = (minRadius + maxRadius) * (endAngle - beginAngle) * 0.5;

		if (aoiCalibrationPtr != NULL){
			i2d::CAffine2d affineTransform;
			if (aoiCalibrationPtr->GetLocalTransform(center, affineTransform)){
				middleBowBitmapLength *= affineTransform.GetDeformMatrix().GetApproxScale();
			}
		}

		int stepsCount = int(middleBowBitmapLength + 1);
		if (circleFinderParams.GetRaysCount() >= 3){
			stepsCount = qMin(stepsCount, circleFinderParams.GetRaysCount());
		}

		for (int i = 0; i < stepsCount; ++i){
			double alpha = (i + 0.5) / stepsCount;
			double angle = alpha * (endAngle - beginAngle) + beginAngle;

			i2d::CVector2d directionVector;
			directionVector.Init(angle);

			aoiLine.SetPoint1(center + directionVector * minRadius);
			aoiLine.SetPoint2(center + directionVector * maxRadius);

			projectionLine.CopyFrom(aoiLine, istd::IChangeable::CM_CONVERT);

			caliperFeaturesConsumerPtr->ResetFeatures();
			m_slaveProcessorCompPtr->DoProcessing(&params, &image, caliperFeaturesConsumerPtr);

			AddProjectionResultsToRays(projectionLine, params, *caliperFeaturesProviderPtr, inRays, outRays);
		}

		totalRaysCount = stepsCount;

		return true;
	}

	return false;
}


bool CCircleFindProcessorComp::CalculateCircle(
			const i2d::CVector2d& center,
			bool removeOutliers,
			double minOutliersDistance,
			Rays& rays,
			int totalRaysCount,
			CircleFeature& result)
{
	int raysCount = int(rays.size());

	imath::CVarMatrix matrix;
	imath::CVarMatrix destVector;
	matrix.SetSizes(istd::CIndex2d(3, raysCount));
	destVector.SetSizes(istd::CIndex2d(1, raysCount));

	int usedRaysCount = 0;

	for (int i = 0; i < raysCount; ++i){
		Ray& ray = rays[i];
		Q_ASSERT(!ray.points.empty());
		Q_ASSERT(ray.usedIndex < int(ray.points.size()));

		if (ray.usedIndex >= 0){
			const Point& rayPoint = ray.points[ray.usedIndex];

			i2d::CVector2d position = rayPoint.position - center;
			double weight = rayPoint.weight;

			matrix.SetAt(istd::CIndex2d(0, i), position.GetX() * 2.0 * weight);
			matrix.SetAt(istd::CIndex2d(1, i), position.GetY() * 2.0 * weight);
			matrix.SetAt(istd::CIndex2d(2, i), -1 * weight);

			destVector.SetAt(istd::CIndex2d(0, i), position.GetLength2() * weight);

			++usedRaysCount;
		}
		else{
			matrix.SetAt(istd::CIndex2d(0, i), 0);
			matrix.SetAt(istd::CIndex2d(1, i), 0);
			matrix.SetAt(istd::CIndex2d(2, i), 0);
			destVector.SetAt(istd::CIndex2d(0, i), 0);
		}
	}

	i2d::CVector2d position(0, 0);
	double radius = 0;
	double radiusError2Sum = 0;

	bool doAgain;
	do{
		doAgain = false;

		if (usedRaysCount < 3){
			return false;	// at least 3 points needed to define circle
		}

		imath::CVarMatrix resultMatrix;
		if (!matrix.GetSolvedLSP(destVector, resultMatrix)){
			return false;
		}

		double relX = resultMatrix.GetAt(istd::CIndex2d(0, 0));
		double relY = resultMatrix.GetAt(istd::CIndex2d(0, 1));

		position = i2d::CVector2d(relX + center[0], relY + center[1]);

		double relPosNorm = relX * relX + relY * relY;
		radius = sqrt(relPosNorm - resultMatrix.GetAt(istd::CIndex2d(0, 2)));

		double worseRadiusDiff = minOutliersDistance;
		int worseIndex = -1;

		for (int i = 0; i < raysCount; ++i){
			Ray& ray = rays[i];

			if (ray.usedIndex >= 0){
				Point& rayPoint = ray.points[ray.usedIndex];

				double currentRadius = rayPoint.position.GetDistance(position);

				double radiusError2 = (currentRadius - radius) * (currentRadius - radius);

				if (radiusError2 > worseRadiusDiff){
					worseRadiusDiff = radiusError2;

					worseIndex = i;
				}

				radiusError2Sum += radiusError2;
			}
		}

		if (removeOutliers){
			if (worseIndex >= 0){
				Ray& ray = rays[worseIndex];
				Q_ASSERT(ray.usedIndex >= 0);
				Q_ASSERT(!ray.points[ray.usedIndex].wasChecked);

				ray.points[ray.usedIndex].wasChecked = true;

				int bestPointIndex = -1;

				double bestDiff = qInf();
				int pointsCount = int(ray.points.size());
				for (int pointIndex = 0; pointIndex < pointsCount; ++pointIndex){
					Point& rayPoint = ray.points[pointIndex];

					if (!rayPoint.wasChecked){
						double currentRadius = rayPoint.position.GetDistance(position);

						double pointDiff = qAbs(currentRadius - radius) / rayPoint.weight;
						if (pointDiff < bestDiff){
							bestDiff = pointDiff;

							bestPointIndex = pointIndex;
						}
					}
				}

				if (bestPointIndex >= 0){
					Point& bestRayPoint = ray.points[bestPointIndex];

					// switch to another point in matrix
					i2d::CVector2d bestPointPosition = bestRayPoint.position - center;
					double bestPointWeight = bestRayPoint.weight;

					matrix.SetAt(istd::CIndex2d(0, worseIndex), bestPointPosition.GetX() * 2.0 * bestPointWeight);
					matrix.SetAt(istd::CIndex2d(1, worseIndex), bestPointPosition.GetY() * 2.0 * bestPointWeight);
					matrix.SetAt(istd::CIndex2d(2, worseIndex), -1 * bestPointWeight);
					destVector.SetAt(istd::CIndex2d(0, worseIndex), bestPointPosition.GetLength2() * bestPointWeight);
				}
				else{
					// remove outlier entry from matrix
					matrix.SetAt(istd::CIndex2d(0, worseIndex), 0);
					matrix.SetAt(istd::CIndex2d(1, worseIndex), 0);
					matrix.SetAt(istd::CIndex2d(2, worseIndex), 0);
					destVector.SetAt(istd::CIndex2d(0, worseIndex), 0);

					--usedRaysCount;
				}

				ray.usedIndex = bestPointIndex;

				doAgain = true;
			}
		}
	} while (doAgain);

	result.SetPosition(position);
	result.SetRadius(radius);
	double circularity = qMax(0.0, 1 - radiusError2Sum / (0.25 * radius * radius * usedRaysCount));
	result.SetWeight(circularity * usedRaysCount / totalRaysCount);
	result.SetCalibration(m_resultCalibrationCompPtr.GetPtr());

	return true;
}


bool CCircleFindProcessorComp::CalculateAnnulus(
			const i2d::CVector2d& center,
			Rays& inRays,
			Rays& outRays,
			int /*totalRaysCount*/,
			AnnulusFeature& result)
{
	int inRaysCount = int(inRays.size());
	if (inRaysCount < 1){
		return false;	// at least 1 point needed to define cocentric circle
	}

	int outRaysCount = int(outRays.size());
	if (outRaysCount < 1){
		return false;	// at least 1 point needed to define cocentric circle
	}

	if (inRaysCount + outRaysCount < 4){
		return false;	// at least 4 point needed to define 2 cocentric circles
	}

	imath::CVarMatrix matrix;
	imath::CVarMatrix destVector;
	matrix.SetSizes(istd::CIndex2d(4, inRaysCount + outRaysCount));
	destVector.SetSizes(istd::CIndex2d(1, inRaysCount + outRaysCount));

	double weightSum = 0;

	for (int i = 0; i < inRaysCount; ++i){
		Ray& ray = inRays[i];
		Q_ASSERT(!ray.points.empty());
		Q_ASSERT(ray.usedIndex >= 0);
		Q_ASSERT(ray.usedIndex < int(ray.points.size()));

		i2d::CVector2d normPos = ray.points[ray.usedIndex].position - center;

		matrix.SetAt(istd::CIndex2d(0, i), normPos.GetX() * 2.0);
		matrix.SetAt(istd::CIndex2d(1, i), normPos.GetY() * 2.0);
		matrix.SetAt(istd::CIndex2d(2, i), -1);
		matrix.SetAt(istd::CIndex2d(3, i), 0);

		destVector.SetAt(istd::CIndex2d(0, i), normPos.GetLength2());

		weightSum += ray.points[ray.usedIndex].weight;
	}

	for (int i = 0; i < outRaysCount; ++i){
		Ray& ray = outRays[i];
		Q_ASSERT(!ray.points.empty());
		Q_ASSERT(ray.usedIndex >= 0);
		Q_ASSERT(ray.usedIndex < int(ray.points.size()));

		i2d::CVector2d normPos = ray.points[ray.usedIndex].position - center;

		matrix.SetAt(istd::CIndex2d(0, i + inRaysCount), normPos.GetX() * 2.0);
		matrix.SetAt(istd::CIndex2d(1, i + inRaysCount), normPos.GetY() * 2.0);
		matrix.SetAt(istd::CIndex2d(2, i + inRaysCount), -1);
		matrix.SetAt(istd::CIndex2d(3, i + inRaysCount), 0);

		destVector.SetAt(istd::CIndex2d(0, i + inRaysCount), normPos.GetLength2());

		weightSum += ray.points[ray.usedIndex].weight;
	}

	imath::CVarMatrix resultMatrix;
	if (!matrix.GetSolvedLSP(destVector, resultMatrix)){
		return false;
	}

	double relX = resultMatrix.GetAt(istd::CIndex2d(0, 0));
	double relY = resultMatrix.GetAt(istd::CIndex2d(0, 1));
	i2d::CVector2d position(relX + center[0], relY + center[1]);
	result.SetPosition(position);

	double relPosNorm = relX * relX + relY * relY;
	double radius1 = sqrt(relPosNorm - resultMatrix.GetAt(istd::CIndex2d(0, 2)));
	double radius2 = sqrt(relPosNorm - resultMatrix.GetAt(istd::CIndex2d(0, 3)));

	result.SetInnerRadius(qMin(radius1, radius2));
	result.SetOuterRadius(qMax(radius1, radius2));
	result.SetWeight(weightSum / (inRaysCount + outRaysCount));
	result.SetCalibration(m_resultCalibrationCompPtr.GetPtr());

	return true;
}


void CCircleFindProcessorComp::AddProjectionResultsToRays(
			const i2d::CLine2d& projectionLine,
			const iprm::IParamsSet& params,
			const IFeaturesProvider& container,
			Rays& inRays,
			Rays& outRays)
{
	Q_ASSERT(m_featuresMapperCompPtr.IsValid());	// validity of features mapper should be checked on the beginning

	Ray inRay;
	Ray outRay;

	double bestInWeight = -1;
	double bestOutWeight = -1;

	int featuresCount = container.GetFeaturesCount();
	for (int featureIndex = 0; featureIndex < featuresCount; featureIndex++){
		const CCaliperFeature* featurePtr = dynamic_cast<const CCaliperFeature*>(&container.GetFeature(featureIndex));
		if (featurePtr != NULL){
			Point point;
			point.weight = featurePtr->GetWeight();
			point.wasChecked = false;
			i2d::CVector2d bitmapPosition;
			m_featuresMapperCompPtr->GetImagePosition(*featurePtr, &params, bitmapPosition);

			if (m_resultCalibrationCompPtr.IsValid()){
				if (!m_resultCalibrationCompPtr->GetInvPositionAt(bitmapPosition, point.position)){
					continue;
				}
			}
			else{
				point.position = bitmapPosition;
			}

			int edgeType = featurePtr->GetEdgeMode();
			if (edgeType == CCaliperFeature::EM_FALLING){
				if (point.weight > bestInWeight){
					bestInWeight = point.weight;
					inRay.usedIndex = int(inRay.points.size());
				}
				inRay.points.push_back(point);
			}
			else{
				if (point.weight > bestOutWeight){
					bestOutWeight = point.weight;
					outRay.usedIndex = int(outRay.points.size());
				}
				outRay.points.push_back(point);
			}
		}
	}

	if (inRay.usedIndex >= 0){
		inRay.projectionLine.CopyFrom(projectionLine);
		inRays.push_back(inRay);
	}

	if (outRay.usedIndex >= 0){
		outRay.projectionLine.CopyFrom(projectionLine);
		outRays.push_back(outRay);
	}
}


void CCircleFindProcessorComp::AddIntermediateResults(Rays& outRays)
{
	if (!m_tempConsumerCompPtr.IsValid()){
		return;
	}

	istd::TDelPtr<ilog::CExtMessage> usedPointMessagePtr;
	istd::TDelPtr<ilog::CExtMessage> unusedPointMessagePtr;
	if (*m_sendUsedPointsToTempAttrPtr){
		usedPointMessagePtr.SetPtr(new ilog::CExtMessage(
					istd::IInformationProvider::IC_INFO,
					iinsp::CSupplierCompBase::MI_INTERMEDIATE,
					QObject::tr("Used found points"),
					"CircleFinder"));
		unusedPointMessagePtr.SetPtr(new ilog::CExtMessage(
					istd::IInformationProvider::IC_INFO,
					iinsp::CSupplierCompBase::MI_INTERMEDIATE,
					QObject::tr("Unused found points"),
					"CircleFinder"));
	}

	istd::TDelPtr<ilog::CExtMessage> projectionLinesMessagePtr;
	if (*m_sendLinesToTempAttrPtr){
		projectionLinesMessagePtr.SetPtr(new ilog::CExtMessage(
					istd::IInformationProvider::IC_INFO,
					iinsp::CSupplierCompBase::MI_INTERMEDIATE,
					QObject::tr("Projection lines"),
					"CircleFinder"));
	}

	for (uint rayIndex = 0; rayIndex < outRays.size(); rayIndex++){
		const Ray& ray = outRays.at(rayIndex);

		if (*m_sendUsedPointsToTempAttrPtr){
			int pointsCount = int(ray.points.size());
			for (int pointIndex = 0; pointIndex < pointsCount; ++pointIndex){
				const Point& rayPoint = ray.points[pointIndex];

				const i2d::CVector2d& position = rayPoint.position;

				i2d::CPosition2d* pointMessageObjectPtr = new imod::TModelWrap<i2d::CPosition2d>();
				pointMessageObjectPtr->SetPosition(position);
				pointMessageObjectPtr->SetCalibration(m_resultCalibrationCompPtr.GetPtr());
				if (pointIndex == ray.usedIndex){
					usedPointMessagePtr->InsertAttachedObject(pointMessageObjectPtr, QObject::tr("Point %1 at (%2, %3)").arg(rayIndex).arg(position.GetX()).arg(position.GetY()));
				}
				else{
					unusedPointMessagePtr->InsertAttachedObject(pointMessageObjectPtr, QObject::tr("Unused point %1 at (%2, %3)").arg(rayIndex).arg(position.GetX()).arg(position.GetY()));
				}
			}
		}

		if (projectionLinesMessagePtr.IsValid()){
			i2d::CLine2d* lineMessageObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
			lineMessageObjectPtr->SetPoint1(ray.projectionLine.GetPoint1());
			lineMessageObjectPtr->SetPoint2(ray.projectionLine.GetPoint2());
			projectionLinesMessagePtr->InsertAttachedObject(lineMessageObjectPtr, QString("Line %1").arg(rayIndex));
		}
	}

	if (usedPointMessagePtr.IsValid() && usedPointMessagePtr->GetAttachedObjectsCount() > 0){
		m_tempConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(usedPointMessagePtr.PopPtr()));
	}

	if (unusedPointMessagePtr.IsValid() && unusedPointMessagePtr->GetAttachedObjectsCount() > 0){
		m_tempConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(unusedPointMessagePtr.PopPtr()));
	}

	if (projectionLinesMessagePtr.IsValid() && projectionLinesMessagePtr->GetAttachedObjectsCount() > 0){
		m_tempConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(projectionLinesMessagePtr.PopPtr()));
	}
}


// reimplemented (icomp::CComponentBase)

void CCircleFindProcessorComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	// Init components for using in multithreading context:
	m_slaveProcessorCompPtr.IsValid();
	m_featuresMapperCompPtr.IsValid();
	m_resultCalibrationCompPtr.IsValid();
	m_tempConsumerCompPtr.IsValid();
}


} // namespace iipr


