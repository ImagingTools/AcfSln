// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CBitmapSupplierMultiplexerComp.h>


namespace icam
{


// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CBitmapSupplierMultiplexerComp::GetBitmap() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != NULL){
		return productPtr->second.GetPtr();
	}
	else{
		return NULL;
	}
}


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CBitmapSupplierMultiplexerComp::GetCalibration() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != NULL){
		return productPtr->first.GetPtr();
	}
	else{
		return NULL;
	}
}


// protected methods

// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CBitmapSupplierMultiplexerComp::ProduceObject(ProductType& result) const
{
	result.first.Reset();
	result.second.Reset();

	if (!m_bitmapCompFact.IsValid()){
		SendCriticalMessage(0, "Bad component architecture, 'BitmapFactory' component reference is not set");

		return WS_FAILED;
	}

	if (!m_bitmapProvidersCompPtr.IsValid()){
		SendCriticalMessage(0, "Bad component architecture, 'MultiBitmapProvider' component reference is not set");

		return WS_FAILED;
	}

	if (!m_bitmapSelectionCompPtr.IsValid()){
		SendCriticalMessage(0, "Bad component architecture, 'BitmapSelection' component reference is not set");

		return WS_FAILED;
	}

	if (!result.second.IsValid()){
		iimg::IBitmapUniquePtr bitmapInstancePtr = m_bitmapCompFact.CreateInstance();

		result.second.FromUnique(std::move(bitmapInstancePtr));
		if (!result.second.IsValid()){
			SendErrorMessage(0, "Bitmap instance could not be created");

			return WS_FAILED;
		}
	}

	if (result.second.IsValid()){
		int selectedIndex = m_bitmapSelectionCompPtr->GetSelectedOptionIndex();
		if (selectedIndex < 0){
			SendVerboseMessage("Bitmap selection index invalid");

			return WS_FAILED;
		}

		if (selectedIndex >= m_bitmapProvidersCompPtr.GetCount()){
			return WS_FAILED;
		}

		const iimg::IBitmapProvider* selectedBitmapProviderPtr = m_bitmapProvidersCompPtr[selectedIndex];
		if (selectedBitmapProviderPtr == NULL){
			return WS_FAILED;
		}

		iinsp::ISupplier* selectedBitmapSupplierPtr = m_bitmapSuppliersCompPtr[selectedIndex];
		if (selectedBitmapSupplierPtr != NULL){
			selectedBitmapSupplierPtr->InvalidateSupplier();
			selectedBitmapSupplierPtr->EnsureWorkInitialized();
			selectedBitmapSupplierPtr->EnsureWorkFinished();
		}

		const iimg::IBitmap* outputBitmapPtr = selectedBitmapProviderPtr->GetBitmap();
		if (outputBitmapPtr != NULL){
			if (result.second->CopyFrom(*outputBitmapPtr)){
				const i2d::ICalibration2d* calibrationPtr = NULL;

				const i2d::ICalibrationProvider* calibrationProviderPtr = m_calibProvidersCompPtr[selectedIndex];
				if (calibrationProviderPtr != NULL){
					calibrationPtr = calibrationProviderPtr->GetCalibration();
				}

				if (calibrationPtr != NULL){
					result.first.MoveCastedPtr(calibrationPtr->CloneMe());
				}
	
				return WS_OK;
			}
		}
		else{
			return WS_FAILED;
		}
	}

	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CBitmapSupplierMultiplexerComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapSelectionModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapSelectionModelCompPtr.GetPtr());
	}

	for (int i = 0; i < m_bitmapProvidersCompPtr.GetCount(); ++i){
		iimg::IBitmapProvider* bitmapProviderPtr = m_bitmapProvidersCompPtr[i];
		if (bitmapProviderPtr != NULL){
			imod::IModel* bitmapProviderModelPtr = m_bitmapProviderModelsCompPtr[i];
			if (bitmapProviderModelPtr != NULL){
				RegisterSupplierInput(bitmapProviderModelPtr, m_bitmapSuppliersCompPtr[i]);
			}
		}
	}
}


void CBitmapSupplierMultiplexerComp::OnComponentDestroyed()
{
	if (m_bitmapSelectionModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_bitmapSelectionModelCompPtr.GetPtr());
	}

	for (int i = 0; i < m_bitmapProvidersCompPtr.GetCount(); ++i){
		iimg::IBitmapProvider* bitmapProviderPtr = m_bitmapProvidersCompPtr[i];
		if (bitmapProviderPtr != NULL){
			imod::IModel* bitmapProviderModelPtr = m_bitmapProviderModelsCompPtr[i];
	
			if (bitmapProviderModelPtr != NULL){
				UnregisterSupplierInput(bitmapProviderModelPtr);
			}
		}
	}

	BaseClass::OnComponentDestroyed();
}


} // namespace icam
