#include <iipr/CDelegatedBitmapSupplierComp.h>


namespace iipr
{


CDelegatedBitmapSupplierComp::CDelegatedBitmapSupplierComp()
:	m_updateBridge(this)
{
}


// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CDelegatedBitmapSupplierComp::GetBitmap() const
{
	if (m_bitmapProviderCompPtr.IsValid()){
		return m_bitmapProviderCompPtr->GetBitmap();
	}

	if (m_bitmapCompPtr.IsValid()){
		return m_bitmapCompPtr.GetPtr();
	}

	return nullptr;
}

	
// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CDelegatedBitmapSupplierComp::GetCalibration() const
{
	if (m_calibrationProviderCompPtr.IsValid()){
		return m_calibrationProviderCompPtr->GetCalibration();
	}

	if (m_calibrationCompPtr.IsValid()){
		return m_calibrationCompPtr.GetPtr();
	}

	return nullptr;
}


// reimplemented (iinsp::ISupplier)

iinsp::ISupplier::WorkStatus CDelegatedBitmapSupplierComp::GetWorkStatus() const
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		return m_bitmapSupplierCompPtr->GetWorkStatus();
	}

	if (m_bitmapCompPtr.IsValid()){
		return WS_OK;
	}

	return WS_FAILED;
}


imod::IModel* CDelegatedBitmapSupplierComp::GetWorkStatusModel() const
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		return m_bitmapSupplierCompPtr->GetWorkStatusModel();
	}

	return nullptr;
}


void CDelegatedBitmapSupplierComp::InvalidateSupplier()
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		m_bitmapSupplierCompPtr->InvalidateSupplier();
	}
}


void CDelegatedBitmapSupplierComp::EnsureWorkInitialized()
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		m_bitmapSupplierCompPtr->EnsureWorkInitialized();
	}
}


void CDelegatedBitmapSupplierComp::EnsureWorkFinished()
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		m_bitmapSupplierCompPtr->EnsureWorkFinished();
	}
}


void CDelegatedBitmapSupplierComp::ClearWorkResults()
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		m_bitmapSupplierCompPtr->ClearWorkResults();
	}
}


const ilog::IMessageContainer* CDelegatedBitmapSupplierComp::GetWorkMessages(MessageContainerType containerType) const
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		return m_bitmapSupplierCompPtr->GetWorkMessages(containerType);
	}

	return nullptr;
}


iprm::IParamsSet* CDelegatedBitmapSupplierComp::GetModelParametersSet() const
{
	if (m_bitmapSupplierCompPtr.IsValid()){
		return m_bitmapSupplierCompPtr->GetModelParametersSet();
	}

	return nullptr;
}


// protected methods

// reimplemented (icomp::CComponentBase)

void CDelegatedBitmapSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapProviderCompPtr.IsValid()){
		if (m_bitmapProviderModelCompPtr.IsValid()){
			m_bitmapProviderModelCompPtr->AttachObserver(&m_updateBridge);
		}
	}
	else{
		if (m_bitmapModelCompPtr.IsValid()){
			m_bitmapModelCompPtr->AttachObserver(&m_updateBridge);
		}

		if (m_calibrationModelCompPtr.IsValid()){
			m_calibrationModelCompPtr->AttachObserver(&m_updateBridge);
		}
	}
}


void CDelegatedBitmapSupplierComp::OnComponentDestroyed()
{
	m_updateBridge.EnsureModelsDetached();

	BaseClass::OnComponentDestroyed();
}


} // namespace iipr


