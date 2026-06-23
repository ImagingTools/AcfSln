// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iproc/CCascadedProcessorComp.h>


// ACF includes
#include <istd/TPointerVector.h>


namespace iproc
{


// reimplemented (iproc::IProcessor)

IProcessor::ProcessorState CCascadedProcessorComp::GetProcessorState(const iprm::IParamsSet* paramsPtr) const
{
	IProcessor::ProcessorState retVal = BaseClass2::GetProcessorState(paramsPtr);

	int processorsCount = m_processorsCompPtr.GetCount();
	for (int i = 0; i < processorsCount; ++i){
		const iproc::IProcessor* processorPtr = m_processorsCompPtr[i];
		if (processorPtr != NULL){
			retVal = qMax(retVal, processorPtr->GetProcessorState(paramsPtr));
		}
	}

	return retVal;
}


bool CCascadedProcessorComp::AreParamsAccepted(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			const istd::IChangeable* outputPtr) const
{
	int buffersCount = m_buffersCompPtr.GetCount();

	int processorsCount = m_processorsCompPtr.GetCount();
	for (int i = 0; i < processorsCount; ++i){
		const istd::IPolymorphic* processorInputPtr = NULL;
		if (i > 0){
			int bufferIndex = i - 1;
			if (bufferIndex < buffersCount){
				processorInputPtr = m_buffersCompPtr[bufferIndex];
			}
		}
		else{
			processorInputPtr = inputPtr;
		}

		const istd::IChangeable* processorOutputPtr = NULL;
		if (i < processorsCount - 1){
			if (i < buffersCount){
				processorOutputPtr = m_buffersCompPtr[i];
			}
		}
		else{
			processorOutputPtr = outputPtr;
		}

		const iproc::IProcessor* processorPtr = m_processorsCompPtr[i];

		if ((processorPtr != NULL) && !processorPtr->AreParamsAccepted(paramsPtr, processorInputPtr, processorOutputPtr)){
			return false;
		}
	}

	return true;
}


IProcessor::TaskState CCascadedProcessorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	int processorsCount = m_processorsCompPtr.GetCount();
	std::vector<std::unique_ptr<ibase::IProgressManager>> progressDelegators(processorsCount);

	if (progressManagerPtr != NULL){
		int managersCount = qMin(processorsCount, m_progressIdsAttrPtr.GetCount());
		for (int i = 0; i < managersCount; ++i){
			const QByteArray& progressId = m_progressIdsAttrPtr[i];
			if (!progressId.isEmpty()){
				QString description = progressId;
				if (i < m_progressDescriptionsAttrPtr.GetCount()){
					description = m_progressDescriptionsAttrPtr[i];
				}

				progressDelegators[i] = progressManagerPtr->CreateSubtaskManager(progressId, description);
			}
		}
	}

	int buffersCount = m_buffersCompPtr.GetCount();

	for (int i = 0; i < processorsCount; ++i){
		const istd::IPolymorphic* processorInputPtr = NULL;
		if (i > 0){
			int bufferIndex = i - 1;
			if (bufferIndex < buffersCount){
				processorInputPtr = m_buffersCompPtr[bufferIndex];
			}
		}
		else{
			processorInputPtr = inputPtr;
		}

		istd::IChangeable* processorOutputPtr = NULL;
		if (i < processorsCount - 1){
			if (i < buffersCount){
				processorOutputPtr = m_buffersCompPtr[i];
			}
		}
		else{
			processorOutputPtr = outputPtr;
		}

		iproc::IProcessor* processorPtr = m_processorsCompPtr[i];
		if (processorPtr == NULL){
			return TS_INVALID;
		}

		IProcessor::TaskState taskState = processorPtr->DoProcessing(paramsPtr, processorInputPtr, processorOutputPtr, progressDelegators[i].get(), processingReportPtr);
		if (taskState != TS_OK){
			return taskState;
		}
	}

	return TS_OK;
}


void CCascadedProcessorComp::InitProcessor(const iprm::IParamsSet* paramsPtr)
{
	BaseClass2::InitProcessor(paramsPtr);

	int processorsCount = m_processorsCompPtr.GetCount();
	for (int i = 0; i < processorsCount; ++i){
		iproc::IProcessor* processorPtr = m_processorsCompPtr[i];
		if (processorPtr != NULL){
			processorPtr->InitProcessor(paramsPtr);
		}
	}
}


// reimplemented (icomp::CComponentBase)

void CCascadedProcessorComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	// initialize components:
	if (m_processorsCompPtr.IsValid()){
		int processorsCount = m_processorsCompPtr.GetCount();
		for (int i = 0; i < processorsCount; ++i){
			iproc::IProcessor* processorPtr = m_processorsCompPtr[i];
			Q_UNUSED(processorPtr);
			Q_ASSERT(processorPtr != NULL);
		}
	}

	if (m_buffersCompPtr.IsValid()){
		int buffersCount = m_buffersCompPtr.GetCount();
		for (int i = 0; i < buffersCount; ++i){
			istd::IChangeable* bufferPtr = m_buffersCompPtr[i];
			Q_UNUSED(bufferPtr);
			Q_ASSERT(bufferPtr != NULL);
		}
	}
}


} // namespace iproc


