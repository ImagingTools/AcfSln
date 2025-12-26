#include <iipr/CTubeProjectionLinesProviderComp.h>


// ACF includes
#include <iprm/TParamsPtr.h>
#include <ilog/IMessageConsumer.h>
#include <ilog/CExtMessage.h>
#include <imeas/INumericValue.h>
#include <iipr/CTubeProjectionsGenerator.h>


namespace iipr
{


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CTubeProjectionLinesProviderComp::GetCalibration() const
{
	if (m_calibrationProviderCompPtr.IsValid()){
		return m_calibrationProviderCompPtr->GetCalibration();
	}

	return nullptr;
}


// reimplemented (iipr::CMultiLineSupplierCompBase)

iinsp::ISupplier::WorkStatus CTubeProjectionLinesProviderComp::ProduceObject(ProductType& result) const
{
	result.clear();

	iprm::IParamsSet* paramsPtr = GetModelParametersSet();
	if (paramsPtr == nullptr){
		return WS_FAILED;
	}

	iprm::TParamsPtr<i2d::CTubePolyline> tubeRegionPtr(paramsPtr, *m_tubeParamsIdAttrPtr);
	if (tubeRegionPtr.IsValid()){
		int projectionsCount = 2;
		iprm::TParamsPtr<imeas::INumericValue> linesCountPtr(paramsPtr, *m_linesCountParamsIdAttrPtr);
		if (linesCountPtr.IsValid()){
			const imath::CVarVector values = linesCountPtr->GetValues();
			if (values.GetElementsCount() > 0){
				projectionsCount = qMax<int>(2, (int)values.GetElement(0));
			}
		}

		Timer performanceTimer(this, "Tube generating");

		if (iipr::CTubeProjectionsGenerator::GenerateProjections(*tubeRegionPtr, projectionsCount, result)){
			if (m_calibrationProviderCompPtr.IsValid()){
				const i2d::ICalibration2d* transformPtr = m_calibrationProviderCompPtr->GetCalibration();
				if (transformPtr == nullptr){
					return WS_FAILED;
				}
				else{
					ilog::CExtMessage* pointMessagePtr = new ilog::CExtMessage(
								istd::IInformationProvider::IC_INFO,
								iinsp::CSupplierCompBase::MI_INTERMEDIATE,
								QObject::tr("Projection lines"),
								"Tube projection generator");
					for (int lineIndex = 0; lineIndex < result.count(); lineIndex++){
						i2d::CLine2d& line = result[lineIndex];

						if (!line.Transform(*transformPtr)){
							result.clear();

							return WS_FAILED;
						}
						
						i2d::CLine2d* pointMessageObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
						pointMessageObjectPtr->SetPoint1(line.GetPoint1());
						pointMessageObjectPtr->SetPoint2(line.GetPoint2());
						pointMessagePtr->InsertAttachedObject(pointMessagePtr, QObject::tr("Line %1").arg(lineIndex));
					}

					AddMessage(pointMessagePtr, MCT_TEMP);
				}
			}

			return WS_OK;
		}
	
		return WS_FAILED;
	}

	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CTubeProjectionLinesProviderComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_calibrationSupplierModelCompPtr.IsValid()){
		RegisterSupplierInput(m_calibrationSupplierModelCompPtr.GetPtr(), m_calibrationSupplierCompPtr.GetPtr());
	}
}


void CTubeProjectionLinesProviderComp::OnComponentDestroyed()
{
	BaseClass::OnComponentDestroyed();

	if (m_calibrationSupplierModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_calibrationSupplierModelCompPtr.GetPtr());
	}
}


} // namespace iipr


