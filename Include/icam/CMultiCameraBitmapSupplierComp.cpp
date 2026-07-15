// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CMultiCameraBitmapSupplierComp.h>


namespace icam
{


// public methods

// reimplemented (iimg::IMultiBitmapProvider)

const iprm::IOptionsList* CMultiCameraBitmapSupplierComp::GetBitmapListInfo() const
{
	return NULL;
}


int CMultiCameraBitmapSupplierComp::GetBitmapsCount() const
{
	if (m_cameraParamsManagerCompPtr.IsValid()){
		return m_cameraParamsManagerCompPtr->GetParamsSetsCount();
	}

	return 0;
}


const iimg::IBitmap* CMultiCameraBitmapSupplierComp::GetBitmap(int bitmapIndex) const
{
	QMutexLocker lock(&m_lock);

	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != NULL){
		return productPtr->at(bitmapIndex).GetPtr();
	}

	return NULL;
}


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CMultiCameraBitmapSupplierComp::GetCalibration() const
{
	return m_logTransformCompPtr.GetPtr();
}


// protected methods

// reimplemented (iinsp::TSupplierCompWrap)

bool CMultiCameraBitmapSupplierComp::InitializeWork()
{
	if (m_cameraParamsManagerCompPtr.IsValid()){
		int camerasCount = m_cameraParamsManagerCompPtr->GetParamsSetsCount();

		for (int cameraIndex = 0; cameraIndex < camerasCount; cameraIndex++){
			if (m_bitmapAcquisitionCompPtr.IsValid()){
				m_bitmapAcquisitionCompPtr->InitProcessor(m_cameraParamsManagerCompPtr->GetParamsSet(cameraIndex));
			}
		}

		return true;
	}

	return false;
}


iinsp::ISupplier::WorkStatus CMultiCameraBitmapSupplierComp::ProduceObject(ProductType& result) const
{
	result.clear();

	if (!m_bitmapCompFact.IsValid()){
		SendCriticalMessage(0, "Bad component architecture, 'BitmapFactory' component reference is not set");

		return WS_FAILED;
	}

	if (!m_bitmapAcquisitionCompPtr.IsValid()){
		SendCriticalMessage(0, "Bad component architecture, 'BitmapAcquisition' component reference is not set");

		return WS_FAILED;
	}

	if (!m_cameraParamsManagerCompPtr.IsValid()){
		SendCriticalMessage(0, "Bad component architecture, 'CameraParamsManager' component reference is not set");

		return WS_FAILED;
	}

	int camerasCount = m_cameraParamsManagerCompPtr->GetParamsSetsCount();
	if (camerasCount <= 0){
		return WS_FAILED;
	}

	iinsp::ISupplier::WorkStatus retVal = WS_OK;

	Timer performanceTimer(this, "Acquisition of image(s)");

	for (int cameraIndex = 0; cameraIndex < camerasCount; cameraIndex++){
		iimg::IBitmapUniquePtr cameraBitmapPtr = m_bitmapCompFact.CreateInstance();
		if (!cameraBitmapPtr.IsValid()){
			SendErrorMessage(0, "Bitmap instance could not be created");

			return WS_FAILED;
		}

		int status = m_bitmapAcquisitionCompPtr->DoProcessing(
					m_cameraParamsManagerCompPtr->GetParamsSet(cameraIndex), 
					NULL, 
					cameraBitmapPtr.GetPtr());

		switch (status){
				case iproc::IProcessor::TS_OK:
					result.push_back(iimg::IBitmapSharedPtr::CreateFromUnique(std::move(cameraBitmapPtr)));
					break;

				case iproc::IProcessor::TS_CANCELED:
					result.clear();
					retVal = WS_CANCELED;
					break;

				default:
					result.clear();
					retVal = WS_FAILED;
					break;
		}
	}

	return retVal;
}


} // namespace icam

