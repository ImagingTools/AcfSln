#include <iipr/CExtremumCaliperSupplierComp.h>


// ACF-Solutions includes
#include <imeas/CSimpleNumericValue.h>


namespace iipr
{


// public methods

// reimplemented (iipr::IFeaturesProvider)

int CExtremumCaliperSupplierComp::GetFeaturesCount() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr)
	{
		return productPtr->GetFeaturesCount();
	}

	return 0;
}


const imeas::INumericValue& CExtremumCaliperSupplierComp::GetFeature(int index) const
{
	static imeas::CSimpleNumericValue emptyValue;

	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr)
	{
		return productPtr->GetFeature(index);
	}

	return emptyValue;
}


// protected methods

// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CExtremumCaliperSupplierComp::ProduceObject(ProductType& result) const
{
	if (		m_sequenceProviderCompPtr.IsValid() &&
				m_extremumProcessorCompPtr.IsValid()){
		Timer performanceTimer(this, "Extrema calculation");
		const imeas::IDataSequence* sequencePtr = m_sequenceProviderCompPtr->GetDataSequence();

		int processingState = m_extremumProcessorCompPtr->DoProcessing(GetModelParametersSet(), sequencePtr, &result);
		switch (processingState){
		case iproc::IProcessor::TS_OK:
			return WS_OK;

		case iproc::IProcessor::TS_CANCELED:
			return WS_CANCELED;

		default:
			return WS_FAILED;
		}
	}

	SendCriticalMessage(0, "Bad component archtecture. Data sequence provider or extremum processor were not set");
	
	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CExtremumCaliperSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_sequenceProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_sequenceProviderModelCompPtr.GetPtr(), m_sequenceSupplierCompPtr.GetPtr());
	}
}


} // namespace iipr


