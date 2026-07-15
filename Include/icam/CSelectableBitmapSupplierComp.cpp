// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CSelectableBitmapSupplierComp.h>


namespace icam
{


// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CSelectableBitmapSupplierComp::GetBitmap() const
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

const i2d::ICalibration2d* CSelectableBitmapSupplierComp::GetCalibration() const
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

iinsp::ISupplier::WorkStatus CSelectableBitmapSupplierComp::ProduceObject(ProductType& result) const
{
	result.first.Reset();
	result.second.Reset();

	if (!m_bitmapCompFact.IsValid()){
		SendCriticalMessage(0, "Bad component architecture, 'BitmapFactory' component reference is not set");

		return WS_FAILED;
	}

	if (!m_multiBitmapProviderCompPtr.IsValid()){
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

		int bitmapsCount = m_multiBitmapProviderCompPtr->GetBitmapsCount();
		if (selectedIndex >= bitmapsCount){
			return WS_FAILED;
		}

		const iimg::IBitmap* outputBitmapPtr = m_multiBitmapProviderCompPtr->GetBitmap(selectedIndex);
		if (outputBitmapPtr != NULL){
			if (result.second->CopyFrom(*outputBitmapPtr)){
				const i2d::ICalibration2d* calibrationPtr = NULL;

				if (m_multiBitmapMultiCalibCompPtr.IsValid() && (selectedIndex < m_multiBitmapMultiCalibCompPtr->GetCalibrationsCount())){
					calibrationPtr = m_multiBitmapMultiCalibCompPtr->GetCalibration(selectedIndex);
				}

				if ((calibrationPtr == NULL) && m_multiBitmapCalibCompPtr.IsValid()){
					calibrationPtr = m_multiBitmapCalibCompPtr->GetCalibration();
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

void CSelectableBitmapSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapSelectionModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapSelectionModelCompPtr.GetPtr());
	}

	if (m_multiBitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_multiBitmapProviderModelCompPtr.GetPtr(), m_multiBitmapSupplierCompPtr.GetPtr());
	}
}


void CSelectableBitmapSupplierComp::OnComponentDestroyed()
{
	if (m_bitmapSelectionModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_bitmapSelectionModelCompPtr.GetPtr());
	}

	if (m_multiBitmapProviderModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_multiBitmapProviderModelCompPtr.GetPtr());
	}

	BaseClass::OnComponentDestroyed();
}


} // namespace icam
