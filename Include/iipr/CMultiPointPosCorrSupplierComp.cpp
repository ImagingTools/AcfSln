#include <iipr/CMultiPointPosCorrSupplierComp.h>


// ACF includes
#include <i2d/CPosition2d.h>
#include <ilog/CExtMessage.h>
#include <iprm/TParamsPtr.h>
#include <iprm/IParamsManager.h>

// ACF-Solutions includes
#include <icalib/CPerspectiveCalibration2d.h>
#include <iipr/CPerspCalibFinder.h>
#include <iipr/CSingleFeatureConsumer.h>


namespace iipr
{


// reimplemented (iinsp::TSupplierCompWrap)

const i2d::ICalibration2d* CMultiPointPosCorrSupplierComp::GetCalibration() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		return productPtr->GetPtr();
	}

	return nullptr;
}


// protected methods

iipr::IImageToFeatureProcessor* CMultiPointPosCorrSupplierComp::GetCompatibleProcessor(const iprm::IParamsSet* paramsSetPtr) const
{
	if (!m_paramsSetTypeIdsAttrPtr.IsValid() || (paramsSetPtr == nullptr)){
		if (m_featureProcessorsCompPtr.GetCount() > 0){
			return m_featureProcessorsCompPtr[0];
		}

		return nullptr;
	}

	QByteArray paramTypeId = paramsSetPtr->GetFactoryId();

	int typesCount = qMin(m_paramsSetTypeIdsAttrPtr.GetCount(), m_featureProcessorsCompPtr.GetCount());
	for (int typeIndex = 0; typeIndex < typesCount; typeIndex++){
		QByteArray typeId = m_paramsSetTypeIdsAttrPtr[typeIndex];

		if (typeId == paramTypeId){
			return m_featureProcessorsCompPtr[typeIndex];
		}
	}

	return nullptr;
}


// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CMultiPointPosCorrSupplierComp::ProduceObject(ProductType& result) const
{
	if (!m_featureProcessorsCompPtr.IsValid()){
		SendCriticalMessage(0, "Wrong component topology");

		return WS_FAILED;
	}

	const iimg::IBitmap* inputImagePtr = nullptr;
	if (m_bitmapProviderCompPtr.IsValid()){
		inputImagePtr = m_bitmapProviderCompPtr->GetBitmap();
	}

	if (inputImagePtr == nullptr){
		SendErrorMessage(0, QObject::tr("No input image"));

		return WS_FAILED;
	}

	iprm::TParamsPtr<iprm::IParamsManager> pointParamsManagerPtr(GetModelParametersSet(), *m_pointParamsManagerIdAttrPtr, true);
	if (!pointParamsManagerPtr.IsValid()){
		SendErrorMessage(0, QObject::tr("No manager of single points found"));

		return WS_FAILED;
	}

	QVector<i2d::CVector2d> destPoints;
	QVector<i2d::CVector2d> foundPoints;

	istd::TDelPtr<ilog::CExtMessage> foundPointsMessagePtr(new ilog::CExtMessage(
				istd::IInformationProvider::IC_INFO,
				MI_GEOMETRICAL_RESULT,
				"Found Points",
				"MultiPointReference"));

	int pointsCount = pointParamsManagerPtr->GetParamsSetsCount();
	for (int i = 0; i < pointsCount; ++i){
		iprm::IParamsSet* paramSetPtr = pointParamsManagerPtr->GetParamsSet(i);
		if (paramSetPtr != nullptr){
			iprm::TParamsPtr<i2d::CPosition2d> destPosPtr(paramSetPtr, *m_destPositionIdAttrPtr, false);
			if (destPosPtr.IsValid()){
				iipr::IImageToFeatureProcessor* processorPtr = GetCompatibleProcessor(paramSetPtr);
				if (processorPtr == nullptr){
					SendCriticalMessage(0, QString("No processor found for %1").arg(pointParamsManagerPtr->GetParamsSetName(i)));

					return WS_FAILED;
				}

				iipr::CSingleFeatureConsumer consumer(iipr::CSingleFeatureConsumer::FP_HEAVIEST);
				if (processorPtr->DoExtractFeatures(paramSetPtr, *inputImagePtr, consumer) == iproc::IProcessor::TS_OK){
					const i2d::CPosition2d* foundPositionPtr = nullptr;
					if (consumer.GetFeaturesCount() > 0){
						foundPositionPtr = dynamic_cast<const i2d::CPosition2d*>(&consumer.GetFeature(0));
					}

					if (foundPositionPtr != nullptr){
						destPoints.push_back(destPosPtr->GetPosition());
						foundPoints.push_back(foundPositionPtr->GetPosition());

						if (foundPointsMessagePtr.IsValid()){
							i2d::CPosition2d* positionPtr = new imod::TModelWrap<i2d::CPosition2d>();
							positionPtr->CopyFrom(*foundPositionPtr, istd::IChangeable::CM_CONVERT);

							foundPointsMessagePtr->InsertAttachedObject(positionPtr);
						}
					}
				}
				else{
					SendErrorMessage(0, QObject::tr("Cannot calculate position %1").arg(pointParamsManagerPtr->GetParamsSetName(i)));

					return WS_FAILED;
				}
			}
		}
	}

	AddMessage(foundPointsMessagePtr.PopPtr(), MCT_TEMP);

	const i2d::ICalibration2d* inputCalibrationPtr = nullptr;
	if (m_calibrationProviderCompPtr.IsValid()){
		inputCalibrationPtr = m_calibrationProviderCompPtr->GetCalibration();
	}

	Q_ASSERT(destPoints.size() == foundPoints.size());

	if (destPoints.size() > 0){
		icalib::CPerspectiveCalibration2d localCalib;
		iipr::CPerspCalibFinder finder;
		
		if (finder.FindPerspCalib(destPoints.constData(), foundPoints.constData(), destPoints.size(), localCalib, false)){
			if (inputCalibrationPtr != nullptr){
				result = inputCalibrationPtr->CreateCombinedCalibration(localCalib);
			}
			else{
				result = new icalib::CPerspectiveCalibration2d(localCalib);
			}

			ilog::CMessage* resultMessagePtr = new ilog::CMessage(
						istd::IInformationProvider::IC_INFO,
						MI_SUPPLIER_RESULTS_STATUS,
						QObject::tr("Calculated reference based on %1 points").arg(destPoints.size()),
						"MultiPointReference");
			AddMessage(resultMessagePtr);

			return WS_OK;
		}
		else{
			ilog::CMessage* errorMessagePtr = new ilog::CMessage(
						istd::IInformationProvider::IC_INFO,
						MI_SUPPLIER_RESULTS_STATUS,
						QObject::tr("Cannot calculate reference"),
						"MultiPointReference");
			AddMessage(errorMessagePtr);

			return WS_OK;
		}
	}

	ilog::CMessage* resultMessagePtr = new ilog::CMessage(
				istd::IInformationProvider::IC_INFO,
				MI_SUPPLIER_RESULTS_STATUS,
				QObject::tr("Empty reference used"),
				"MultiPointReference");
	AddMessage(resultMessagePtr);

	if (inputCalibrationPtr != nullptr){
		result.MoveCastedPtr(inputCalibrationPtr->CloneMe());
	}
	else{
		result.Reset();
	}

	return WS_OK;
}


// reimplemented (icomp::CComponentBase)

void CMultiPointPosCorrSupplierComp::OnComponentCreated()
{
	if (m_bitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr(), m_bitmapSupplierCompPtr.GetPtr());
	}

	BaseClass::OnComponentCreated();
}


} // namespace iipr


