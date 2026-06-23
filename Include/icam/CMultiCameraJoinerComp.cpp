// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CMultiCameraJoinerComp.h>


// ACF includes
#include <iprm/IOptionsList.h>
#include <iprm/TParamsPtr.h>


namespace icam
{


// reimplemented (iipr::IBitmapAcquisition)

istd::CIndex2d CMultiCameraJoinerComp::GetBitmapSize(const iprm::IParamsSet* /*paramsPtr*/) const
{
	const iimg::IBitmap* resultPtr = GetResultBitmap();
	if (resultPtr == NULL){
		return istd::CIndex2d::GetInvalid();
	}

	return resultPtr->GetImageSize();
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CMultiCameraJoinerComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	QMutexLocker lock(&m_lock);

	if (!m_cameraCompPtr.IsValid()){
		return TS_INVALID;
	}

	const iprm::IParamsManager* paramsManagerPtr = GetCameraParamsManager(paramsPtr);
	if (paramsManagerPtr == NULL){
		SendErrorMessage(0, "Camera parameters not set");

		return TS_INVALID;
	}

	int cameraCount = paramsManagerPtr->GetParamsSetsCount();

	bool retVal = PrepareSnapResult(cameraCount, outputPtr);
	if (!retVal){
		return TS_INVALID;
	}

	istd::CChangeNotifier changePtr(outputPtr);

	for (int i = 0; i < cameraCount; i++){
		const iprm::IParamsSet* cameraParamsSet = paramsManagerPtr->GetParamsSet(i);

		if (m_cameraCompPtr->DoProcessing(cameraParamsSet, inputPtr, (m_snapList[i].bitmapPtr.GetPtr()), progressManagerPtr, processingReportPtr) == TS_OK){
			m_snapList[i].isSnapOk = true;

			retVal = m_snapList[i].isSnapOk && retVal;
		}
		else{
			retVal = false;
		}
	}

	retVal = retVal && FinalizeSnapResult();

	return retVal ? TS_OK : TS_INVALID;
}


void CMultiCameraJoinerComp::InitProcessor(const iprm::IParamsSet* paramsPtr)
{
	const iprm::IParamsManager* paramsManagerPtr = GetCameraParamsManager(paramsPtr);
	if (paramsManagerPtr == NULL){
		return;
	}

	if (m_cameraCompPtr.IsValid()){
		int cameraCount = paramsManagerPtr->GetParamsSetsCount();

		for (int i = 0; i < cameraCount; i++){
			if (m_cameraCompPtr.IsValid()){
				m_cameraCompPtr->InitProcessor(paramsManagerPtr->GetParamsSet(i));
			}
		}
	}
}


const iprm::IParamsManager* CMultiCameraJoinerComp::GetCameraParamsManager(const iprm::IParamsSet* paramsPtr) const
{
	const iprm::IParamsManager* paramsManagerPtr = NULL;

	if (m_cameraParamsManagerCompPtr.IsValid()){
		paramsManagerPtr = m_cameraParamsManagerCompPtr.GetPtr();

		if (m_cameraSelectorCompPtr.IsValid()){
			int index = m_cameraSelectorCompPtr->GetSelectedOptionIndex();
			if (index < 0){
				return NULL;
			}

			iprm::IParamsSet* managerParamsSet = paramsManagerPtr->GetParamsSet(index);
			if (managerParamsSet != NULL){
				iprm::TParamsPtr<iprm::IParamsManager> paramsManagerParamPtr(managerParamsSet, *m_cameraParamsManagerIdAttrPtr);
				if (paramsManagerParamPtr.IsValid()){
					paramsManagerPtr = paramsManagerParamPtr.GetPtr();
				} 
			}
		}
	}
	else{
		iprm::TParamsPtr<iprm::IParamsManager> paramsManagerParamPtr(paramsPtr, *m_cameraParamsManagerIdAttrPtr);
		if (paramsManagerParamPtr.IsValid()){
			paramsManagerPtr = paramsManagerParamPtr.GetPtr();
		} 
	}

	if (paramsManagerPtr == NULL){
		SendErrorMessage(0, "Camera parameters not set");
	}
	
	return paramsManagerPtr;
}


} // namespace icam


