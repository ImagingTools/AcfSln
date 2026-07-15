// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CSnapBitmapSupplierCompBase.h>


// ACF includes
#include <imod/TModelWrap.h>
#include <imath/CGeneralUnitInfo.h>
#include <iprm/TParamsPtr.h>

// ACF-Solutions includes
#include <iproc/IProcessor.h>
#include <icalib/TCalibrationFactory.h>


namespace icam
{


// public methods

// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CSnapBitmapSupplierCompBase::GetBitmap() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != NULL){
		return productPtr->second.GetPtr();
	}

	return NULL;
}


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CSnapBitmapSupplierCompBase::GetCalibration() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != NULL){
		return productPtr->first.GetPtr();
	}

	return NULL;
}


// protected methods

// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CSnapBitmapSupplierCompBase::ProduceObject(ProductType& result) const
{
	result.first.Reset();

	if (!result.second.IsValid()){
		iimg::IBitmapUniquePtr instancePtr = CreateBitmap();

		result.second = iimg::IBitmapSharedPtr::CreateFromUnique(std::move(instancePtr));
		if (!result.second.IsValid()){
			SendErrorMessage(0, "Bitmap instance could not be created");

			return WS_FAILED;
		}
	}

	Timer performanceTimer(this, "Image acquisition");

	Q_ASSERT(result.second.IsValid());

	int status = DoSnap(GetModelParametersSet(), *result.second.GetPtr());
	switch (status){
		case iproc::IProcessor::TS_OK:
			{
				istd::CIndex2d bitmapSize = result.second->GetImageSize();

				const bool allowEmptyImage = m_allowEmptyImageResultAttrPtr.IsValid() ? *m_allowEmptyImageResultAttrPtr : false;

				if (bitmapSize.IsSizeEmpty() && !allowEmptyImage) {
					SendErrorMessage(0, QT_TR_NOOP("Image is empty"));
					return WS_FAILED;
				}

				i2d::CVector2d center = *m_moveCalibrationToCenterAttrPtr ? i2d::CVector2d(bitmapSize.GetX() * 0.5, bitmapSize.GetY() * 0.5) : i2d::CVector2d();

				i2d::CVector2d scale(1, 1);

				iprm::TParamsPtr<imeas::INumericValue> scaleParamPtr(GetModelParametersSet(), m_scaleParamIdAttrPtr, m_defaultScaleValueCompPtr, false);
				if (scaleParamPtr.IsValid()){
					imath::CVarVector scaleValues = scaleParamPtr->GetValues();
					if (scaleValues.GetElementsCount() >= 2){
						scale = i2d::CVector2d(scaleValues[0], scaleValues[1]);
					}
					else if (scaleValues.GetElementsCount() >= 1){
						scale = i2d::CVector2d(scaleValues[0], scaleValues[0]);
					}
				}

				iprm::TParamsPtr<i2d::ICalibration2d> calibrationPtr(GetModelParametersSet(), m_calibrationIdAttrPtr, m_defaultCalibrationCompPtr, false);
				if (calibrationPtr.IsValid()){
					//result.first.SetCastedOrRemove(calibrationPtr->CloneMe());
					result.first.MoveCastedPtr(icalib::FactorizeFrom(calibrationPtr.GetPtr()));
				}
				else{
					if (scale != i2d::CVector2d(1, 1)){
						icalib::CAffineCalibration2d* affineCalibrationPtr = new imod::TModelWrap<icalib::CAffineCalibration2d>();
						affineCalibrationPtr->Reset(center, 0, scale);

						result.first.SetPtr(affineCalibrationPtr);
					}
					else if (auto bitmapCalibPtr = result.second->GetCalibration()) {
						//result.first.SetCastedOrRemove(result.second->GetCalibration()->CloneMe());
						result.first.MoveCastedPtr(icalib::FactorizeFrom(bitmapCalibPtr));
					}
					else{
						result.first.Reset();
					}
				}
			}
			return WS_OK;

		case iproc::IProcessor::TS_CANCELED:
			return WS_CANCELED;

		default:
			return WS_FAILED;
	}

	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CSnapBitmapSupplierCompBase::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	// initialize components
	m_defaultCalibrationCompPtr.EnsureInitialized();
}


} // namespace icam

