// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iproc/CIterativeProcessorComp.h>


// ACF includes
#include <iprm/ISelectionParam.h>
#include <iprm/TParamsPtr.h>

#include <ibase/IProgressManager.h>


namespace iproc
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CIterativeProcessorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	if (		(inputPtr == NULL) ||
				(paramsPtr == NULL) ||
				!m_paramsIdAttrPtr.IsValid()){
		return TS_INVALID;
	}

	iproc::IProcessor::TaskState retVal = TS_INVALID;

	iprm::TParamsPtr<iprm::ISelectionParam> processorParamsPtr(paramsPtr, *m_paramsIdAttrPtr);
	if (processorParamsPtr.IsValid() && m_slaveProcessorCompPtr.IsValid()){
		int iterationsCount = processorParamsPtr->GetSelectedOptionIndex();

		if (iterationsCount <= 0){
			const istd::IChangeable* inputObjectPtr = dynamic_cast<const istd::IChangeable*>(inputPtr);

			if ((inputObjectPtr != NULL) && outputPtr->CopyFrom(*inputObjectPtr)){
				return TS_OK;
			}

			return TS_INVALID;
		}

		auto progressLoggerPtr = StartProgressLogger(progressManagerPtr, true);

		for (int iterationIndex = 0; iterationIndex < iterationsCount; iterationIndex++){
			retVal = ProcessSlave(paramsPtr, inputPtr, outputPtr);
			if (retVal != TS_OK){
				break;
			}

			if (iterationIndex < iterationsCount - 1){
				if (!m_bufferObjectCompPtr.IsValid()){
					SendErrorMessage(MI_BAD_BUFFER, "No buffer object");

					retVal = TS_INVALID;

					break;
				}

				if (m_bufferObjectCompPtr->CopyFrom(*outputPtr)){
					inputPtr = m_bufferObjectCompPtr.GetPtr();
				}
				else{
					SendErrorMessage(MI_BAD_BUFFER, "Copy of used type is not supported by buffer object");

					retVal = TS_INVALID;

					break;
				}
			}

			if (progressLoggerPtr != nullptr){
				progressLoggerPtr->OnProgress(iterationIndex / double(iterationsCount - 1));

				if (progressLoggerPtr->IsCanceled()){
					break;
				}
			}
		}
	}

	return retVal;
}


// reimplemented (iprm::IOptionsList)

int CIterativeProcessorComp::GetOptionsFlags() const
{
	return SCF_NONE;
}


int CIterativeProcessorComp::GetOptionsCount() const
{
	return int(m_iterationNames.size());
}


QString CIterativeProcessorComp::GetOptionName(int index) const
{
	Q_ASSERT(index >= 0);
	Q_ASSERT(index < int(m_iterationNames.size()));

	return m_iterationNames[index];
}


QString CIterativeProcessorComp::GetOptionDescription(int /*index*/) const
{
	return QString();
}


QByteArray CIterativeProcessorComp::GetOptionId(int /*index*/) const
{
	return QByteArray();
}


bool CIterativeProcessorComp::IsOptionEnabled(int /*index*/) const
{
	return true;
}


// protected methods

iproc::IProcessor::TaskState CIterativeProcessorComp::ProcessSlave(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr)
{
	Q_ASSERT(m_slaveProcessorCompPtr.IsValid());

	iproc::IProcessor::TaskState retVal = TS_INVALID;

	int taskId = m_slaveProcessorCompPtr->BeginTask(paramsPtr, inputPtr, outputPtr);
	if (taskId >= 0){
		retVal = m_slaveProcessorCompPtr->WaitTaskFinished(taskId);
	}

	return retVal;
}


// reimplemented (icomp::CComponentBase)

void CIterativeProcessorComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	int optionsCount = qMax(int(0), *m_maxIterationsCountAttrPtr) + 1;
	m_iterationNames.resize(optionsCount);

	m_iterationNames[0] = "No iteration";
	if (optionsCount >= 1){
		m_iterationNames[1] = "1 iteration";

		for (int i = 2; i < optionsCount; ++i){
			m_iterationNames[i] = QString("%1").arg(i) + " iterations";
		}
	}
}


} // namespace iproc

