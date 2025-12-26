#include <iipr/CPositionFromImageSupplierComp.h>


// ACF includes
#include <imod/TModelWrap.h>
#include <ilog/CExtMessage.h>
#include <i2d/CCircle.h>
#include <i2d/CPolyline.h>

// ACF-Solutions includes
#include <icalib/CAffineCalibration2d.h>
#include <iipr/CSingleFeatureConsumer.h>


namespace iipr
{


// reimplemented (iipr::IFeaturesProvider)

int CPositionFromImageSupplierComp::GetFeaturesCount() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		return productPtr->GetFeaturesCount();
	}

	return 0;
}


const imeas::INumericValue& CPositionFromImageSupplierComp::GetFeature(int index) const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		return productPtr->GetFeature(index);
	}

	static imeas::CSimpleNumericValue empty;

	return empty;
}


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CPositionFromImageSupplierComp::GetCalibration() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		return m_outputCalibrationPtr.GetPtr();
	}

	return nullptr;
}


// protected methods

// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CPositionFromImageSupplierComp::ProduceObject(ProductType& result) const
{
	result.ResetFeatures();

	m_outputCalibrationPtr.Reset();

	if (		m_bitmapProviderCompPtr.IsValid() &&
				m_processorCompPtr.IsValid()){
		const iimg::IBitmap* bitmapPtr = m_bitmapProviderCompPtr->GetBitmap();
		if (bitmapPtr == nullptr){
			AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("No input image"), "PositionFinder"));

			return WS_FAILED;
		}

		iprm::IParamsSet* paramsSetPtr = GetModelParametersSet();

		Timer performanceTimer(this, "Calculation of position");

		int positionState = m_processorCompPtr->DoProcessing(
						paramsSetPtr,
						bitmapPtr,
						&result);

		if (positionState != iproc::IProcessor::TS_OK){
			return WS_FAILED;
		}

		const i2d::ICalibration2d* inputCalibrationPtr = nullptr;
		if (m_calibrationProviderCompPtr.IsValid()){
			inputCalibrationPtr = m_calibrationProviderCompPtr->GetCalibration();
		}

		if (result.GetFeaturesCount() < 1){
			return WS_FAILED;
		}

		const i2d::CObject2dBase* positionPtr = dynamic_cast<const i2d::CObject2dBase*>(&result.GetFeature(0));
		if (positionPtr == nullptr){
			return WS_FAILED;
		}

		i2d::CVector2d resultVector;

		const i2d::CCircle* circlePtr = dynamic_cast<const i2d::CCircle*>(positionPtr);
		const i2d::CLine2d* linePtr = dynamic_cast<const i2d::CLine2d*>(positionPtr);
		const i2d::CPolyline* polylinePtr = dynamic_cast<const i2d::CPolyline*>(positionPtr);

		if (circlePtr != nullptr){
			i2d::CCircle transformedCircle;
			if (!transformedCircle.CopyFrom(*circlePtr, istd::IChangeable::CM_CONVERT)){
				return WS_FAILED;
			}

			resultVector = transformedCircle.GetPosition();

			ilog::CExtMessage* messagePtr = new ilog::CExtMessage(
						istd::IInformationProvider::IC_INFO,
						iinsp::CSupplierCompBase::MI_GEOMETRICAL_RESULT,
						QString("Radius: %3, Pos.: (%1, %2)")
							.arg(circlePtr->GetPosition().GetX())
							.arg(circlePtr->GetPosition().GetY())
							.arg(circlePtr->GetRadius()),
						"PositionFinder");

			i2d::CCircle* messageObjectPtr = new imod::TModelWrap<i2d::CCircle>();
			messageObjectPtr->i2d::CCircle::CopyFrom(*circlePtr, istd::IChangeable::CM_WITH_REFS);
			messagePtr->InsertAttachedObject(messageObjectPtr);

			AddMessage(messagePtr);
		}
		else if (linePtr != nullptr){
			i2d::CLine2d transformedLine;
			if (!transformedLine.CopyFrom(*linePtr, istd::IChangeable::CM_CONVERT)){
				return WS_FAILED;
			}

			resultVector = transformedLine.GetCenter();

			ilog::CExtMessage* messagePtr = new ilog::CExtMessage(
						istd::IInformationProvider::IC_INFO,
						iinsp::CSupplierCompBase::MI_GEOMETRICAL_RESULT,
						QString("Line: (%1, %2)->(%3, %4)")
							.arg(linePtr->GetPoint1().GetX()).arg(linePtr->GetPoint1().GetY())
							.arg(linePtr->GetPoint2().GetX()).arg(linePtr->GetPoint2().GetY()),
						"PositionFinder");

			i2d::CLine2d* messageObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
			messageObjectPtr->i2d::CLine2d::CopyFrom(*linePtr, istd::IChangeable::CM_CONVERT);
			messagePtr->InsertAttachedObject(messageObjectPtr);

			AddMessage(messagePtr);
		}
		else if (polylinePtr != nullptr){
			i2d::CPolyline transformedLine;
			if (!transformedLine.CopyFrom(*polylinePtr, istd::IChangeable::CM_CONVERT)){
				return WS_FAILED;
			}

			resultVector = transformedLine.GetCenter();

			ilog::CExtMessage* resultMessagePtr = new ilog::CExtMessage(
						istd::IInformationProvider::IC_INFO,
						iinsp::CSupplierCompBase::MI_INTERMEDIATE,
						QObject::tr("Polyline: %1 edges").arg(transformedLine.GetSegmentsCount()),
						"PositionFinder");

			i2d::CPolyline* resultObjectPtr = new imod::TModelWrap<i2d::CPolyline>();
			resultObjectPtr->CopyFrom(*polylinePtr, istd::IChangeable::CM_CONVERT);
			resultMessagePtr->InsertAttachedObject(resultObjectPtr);
			
			AddMessage(resultMessagePtr);
		}
		else{
			i2d::CPosition2d transformedPosition;
			if (!transformedPosition.CopyFrom(*positionPtr, istd::IChangeable::CM_CONVERT)){
				return WS_FAILED;
			}

			resultVector = transformedPosition.GetPosition();

			ilog::CExtMessage* messagePtr = new ilog::CExtMessage(
						istd::IInformationProvider::IC_INFO,
						iinsp::CSupplierCompBase::MI_GEOMETRICAL_RESULT,
						QString("Pos.: (%1, %2)")
							.arg(positionPtr->GetCenter().GetX())
							.arg(positionPtr->GetCenter().GetY()),
						"PositionFinder");

			i2d::CPosition2d* messageObjectPtr = new imod::TModelWrap<i2d::CPosition2d>();
			messageObjectPtr->i2d::CPosition2d::CopyFrom(*positionPtr, istd::IChangeable::CM_CONVERT);
			messagePtr->InsertAttachedObject(messageObjectPtr);

			AddMessage(messagePtr);
		}

		i2d::CVector2d originalZeroPos(0, 0);
		if (inputCalibrationPtr != nullptr){
			originalZeroPos = inputCalibrationPtr->GetValueAt(i2d::CVector2d(0, 0));
		}

		icalib::CAffineCalibration2d* outputTransformPtr = new imod::TModelWrap<icalib::CAffineCalibration2d>();
		outputTransformPtr->Reset(resultVector - originalZeroPos);
		m_outputCalibrationPtr.SetPtr(outputTransformPtr);

		if (inputCalibrationPtr != nullptr){
			m_outputCalibrationPtr = m_outputCalibrationPtr->CreateCombinedCalibration(*inputCalibrationPtr);
		}

		return WS_OK;
	}

	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CPositionFromImageSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapProviderCompPtr.IsValid() && m_bitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr(), m_bitmapSupplierCompPtr.GetPtr());
	}

	// Initialize components
	m_calibrationProviderCompPtr.IsValid();
	m_processorCompPtr.IsValid();
}


} // namespace iipr


