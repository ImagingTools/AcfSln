// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CCameraDelegatorComp.h>


// ACF includes
#include <istd/CIndex2d.h>
#include <iprm/CParamsSet.h>


namespace icam
{


// reimplemented (iipr::IBitmapAcquisition)

istd::CIndex2d CCameraDelegatorComp::GetBitmapSize(const iprm::IParamsSet* paramsPtr) const
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);

	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->GetBitmapSize(workingParamsPtr);
	}

	return istd::CIndex2d(0, 0);
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::ProcessorState CCameraDelegatorComp::GetProcessorState(const iprm::IParamsSet* paramsPtr) const
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);

	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->GetProcessorState(workingParamsPtr);
	}

	return PS_UNKNOWN;
}


bool CCameraDelegatorComp::AreParamsAccepted(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			const istd::IChangeable* outputPtr) const
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);

	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->AreParamsAccepted(workingParamsPtr, inputPtr, outputPtr);
	}

	return false;
}


iproc::IProcessor::TaskState CCameraDelegatorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);

	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->DoProcessing(workingParamsPtr, inputPtr, outputPtr, progressManagerPtr, processingReportPtr);
	}

	return TS_INVALID;
}


int CCameraDelegatorComp::BeginTask(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);

	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->BeginTask(workingParamsPtr, inputPtr, outputPtr, progressManagerPtr, processingReportPtr);
	}

	return -1;
}


iproc::IProcessor::TaskState CCameraDelegatorComp::WaitTaskFinished(
				int taskId,
				double timeoutTime,
				bool killOnTimeout)
{
	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->WaitTaskFinished(taskId, timeoutTime, killOnTimeout);
	}

	return TS_INVALID;
}


void CCameraDelegatorComp::CancelTask(int taskId)
{
	if (m_slaveCameraCompPtr.IsValid()){
		m_slaveCameraCompPtr->CancelTask(taskId);
	}
}


int CCameraDelegatorComp::GetReadyTask()
{
	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->GetReadyTask();
	}

	return -1;
}


iproc::IProcessor::TaskState CCameraDelegatorComp::GetTaskState(int taskId) const
{
	if (m_slaveCameraCompPtr.IsValid()){
		return m_slaveCameraCompPtr->GetTaskState(taskId);
	}

	return TS_INVALID;
}


void CCameraDelegatorComp::InitProcessor(const iprm::IParamsSet* paramsPtr)
{
	const iprm::IParamsSet* workingParamsPtr = GetWorkingParamsSet(paramsPtr);

	if (m_slaveCameraCompPtr.IsValid()){
		m_slaveCameraCompPtr->InitProcessor(workingParamsPtr);
	}
}


// protected methods

const iprm::IParamsSet* CCameraDelegatorComp::GetWorkingParamsSet(const iprm::IParamsSet* inputParamsPtr) const
{
	if (m_parameterSetCompPtr.IsValid()){
		iprm::CParamsSet* paramsSetImplPtr = dynamic_cast<iprm::CParamsSet*>(m_parameterSetCompPtr.GetPtr());
		if (paramsSetImplPtr != NULL){
			paramsSetImplPtr->SetSlaveSet(inputParamsPtr);
		}

		return m_parameterSetCompPtr.GetPtr();
	}

	return inputParamsPtr;
}


// reimplemented (icomp::CComponentBase)

void CCameraDelegatorComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	// initilaize components
	m_slaveCameraCompPtr.EnsureInitialized();
}


} // namespace icam


