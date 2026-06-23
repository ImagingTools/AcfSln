// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CCameraMultiplexerComp.h>


// ACF includes
#include <iprm/ISelectionParam.h>
#include <iprm/IParamsManager.h>
#include <iprm/TParamsPtr.h>


namespace icam
{


// public methods

CCameraMultiplexerComp::CCameraMultiplexerComp() : m_currentParamsSetPtr(NULL)
{
	m_cameraList.SetParent(this);
}


// reimplemented (iipr::IBitmapAcquisition)

istd::CIndex2d CCameraMultiplexerComp::GetBitmapSize(const iprm::IParamsSet* paramsPtr) const
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);
	const IBitmapAcquisition* camera = GetSelectedCamera(paramsPtr);

	if (camera != NULL){
		return camera->GetBitmapSize(workingParamsPtr);
	}

	return istd::CIndex2d(0, 0);
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::ProcessorState CCameraMultiplexerComp::GetProcessorState(const iprm::IParamsSet* paramsPtr) const
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);
	const IBitmapAcquisition* camera = GetSelectedCamera(paramsPtr);

	if (camera != NULL){
		return camera->GetProcessorState(workingParamsPtr);
	}

	return PS_UNKNOWN;
}


bool CCameraMultiplexerComp::AreParamsAccepted(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			const istd::IChangeable* outputPtr) const
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);
	const IBitmapAcquisition* camera = GetSelectedCamera(paramsPtr);

	if (camera != NULL){
		return camera->AreParamsAccepted(workingParamsPtr, inputPtr, outputPtr);
	}

	return false;
}


iproc::IProcessor::TaskState CCameraMultiplexerComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);
	IBitmapAcquisition* camera = GetSelectedCamera(paramsPtr);

	if (camera != NULL){
		return camera->DoProcessing(workingParamsPtr, inputPtr, outputPtr, progressManagerPtr, processingReportPtr);
	}

	return TS_INVALID;
}


int CCameraMultiplexerComp::BeginTask(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);
	IBitmapAcquisition* camera = GetSelectedCamera(paramsPtr);

	if (camera != NULL){
		return camera->BeginTask(workingParamsPtr, inputPtr, outputPtr, progressManagerPtr, processingReportPtr);
	}

	return -1;
}


iproc::IProcessor::TaskState CCameraMultiplexerComp::WaitTaskFinished(
				int taskId,
				double timeoutTime,
				bool killOnTimeout)
{
	IBitmapAcquisition* camera = GetSelectedCamera(m_currentParamsSetPtr);

	if (camera != NULL){
		return camera->WaitTaskFinished(taskId, timeoutTime, killOnTimeout);
	}

	return TS_INVALID;
}


void CCameraMultiplexerComp::CancelTask(int taskId)
{
	IBitmapAcquisition* camera = GetSelectedCamera(m_currentParamsSetPtr);

	if (camera != NULL){
		camera->CancelTask(taskId);
	}
}


int CCameraMultiplexerComp::GetReadyTask()
{
	IBitmapAcquisition* camera = GetSelectedCamera(m_currentParamsSetPtr);

	if (camera != NULL){
		return camera->GetReadyTask();
	}

	return -1;
}


iproc::IProcessor::TaskState CCameraMultiplexerComp::GetTaskState(int taskId) const
{
	const IBitmapAcquisition* camera = GetSelectedCamera(m_currentParamsSetPtr);

	if (camera != NULL){
		return camera->GetTaskState(taskId);
	}

	return TS_INVALID;
}


void CCameraMultiplexerComp::InitProcessor(const iprm::IParamsSet* paramsPtr)
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);
	IBitmapAcquisition* camera = GetSelectedCamera(paramsPtr);

	if (camera != NULL){
		camera->InitProcessor(workingParamsPtr);
	}
}

// private methods

int CCameraMultiplexerComp::GetSelectedIndex(const iprm::IParamsSet* paramsPtr) const
{
	iprm::TParamsPtr<iprm::ISelectionParam> sourceSelectionParamPtr(paramsPtr != NULL ? paramsPtr : m_parameterSetCompPtr.GetPtr(), *m_sourceSelectionParamIdAttrPtr);
	if (sourceSelectionParamPtr.IsValid()){
		int sourceIndex = sourceSelectionParamPtr->GetSelectedOptionIndex();
		return sourceIndex;
	}

	return -1;
}


const iprm::IParamsSet* CCameraMultiplexerComp::GetWorkingParamsSet(const iprm::IParamsSet* paramsPtr) const
{
	iprm::TParamsPtr<iprm::IParamsManager> cameraParamsManagerPtr(paramsPtr != NULL ? paramsPtr : m_parameterSetCompPtr.GetPtr(), *m_cameraManagerParamIdAttrPtr);
	if (cameraParamsManagerPtr.IsValid()){
		int selectedCameraIndex = GetSelectedIndex(paramsPtr != NULL ? paramsPtr : m_parameterSetCompPtr.GetPtr());
		if ((selectedCameraIndex >= 0) && (selectedCameraIndex < cameraParamsManagerPtr->GetParamsSetsCount())){
			return cameraParamsManagerPtr->GetParamsSet(selectedCameraIndex);
		}

	}

	return NULL;
}


IBitmapAcquisition* CCameraMultiplexerComp::GetSelectedCamera(const iprm::IParamsSet* paramsPtr) const
{
	m_currentParamsSetPtr = paramsPtr != NULL ? paramsPtr : m_parameterSetCompPtr.GetPtr();
	int selectedCameraIndex = GetSelectedIndex(m_currentParamsSetPtr);

	if (selectedCameraIndex >= 0 && selectedCameraIndex < m_cameraListCompPtr.GetCount()){
		return m_cameraListCompPtr[selectedCameraIndex];
	}

	return NULL;
}


// public methods of the embedded class CameraList

CCameraMultiplexerComp::CameraList::CameraList()
	:m_parentPtr(NULL)
{
}


void CCameraMultiplexerComp::CameraList::SetParent(CCameraMultiplexerComp* parentPtr)
{
	m_parentPtr = parentPtr;
}


// reimplemented (iprm::IOptionsList)

int CCameraMultiplexerComp::CameraList::GetOptionsFlags() const
{
	return SCF_SUPPORT_UNIQUE_ID;
}


int CCameraMultiplexerComp::CameraList::GetOptionsCount() const
{
	Q_ASSERT(m_parentPtr != NULL);

	return m_parentPtr->m_cameraListCompPtr.GetCount();
}


QString CCameraMultiplexerComp::CameraList::GetOptionName(int index) const
{
	Q_ASSERT(m_parentPtr != NULL);
	Q_ASSERT(index >= 0);

	if (index < m_parentPtr->m_cameraNamesAttrPtr.GetCount()){
		return m_parentPtr->m_cameraNamesAttrPtr[index];
	}
	else{
		return "Unnamed";
	}
}


QString CCameraMultiplexerComp::CameraList::GetOptionDescription(int index) const
{
	Q_ASSERT(m_parentPtr != NULL);
	Q_ASSERT(index >= 0);
	Q_UNUSED(index);

	return QString();
}


QByteArray CCameraMultiplexerComp::CameraList::GetOptionId(int index) const
{
	Q_ASSERT(m_parentPtr != NULL);
	Q_ASSERT(index >= 0);

	if (index < m_parentPtr->m_cameraIdsAttrPtr.GetCount()){
		return m_parentPtr->m_cameraIdsAttrPtr[index];
	}
	else{
		return QString("Camera-%1").arg(index + 1).toUtf8();
	}
}


bool CCameraMultiplexerComp::CameraList::IsOptionEnabled(int index) const
{
	Q_UNUSED(index);

	return true;
}


} // namespace icam


