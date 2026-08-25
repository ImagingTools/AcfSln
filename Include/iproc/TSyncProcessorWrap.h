// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iproc_TSyncProcessorWrap_included
#define iproc_TSyncProcessorWrap_included


// Qt includes
#include <QtCore/QMap>

// ACF includes
#include <iprm/IParamsSet.h>
#include <iproc/IProcessor.h>


namespace iproc
{


/**
	Wrapper of \c iproc::IProcessor for simple synchrone processor implementations.
	Asynchrone processing with \c BeginTask and \c WaitTaskFinished is redirected to simple synchrone processing by method \c DoProcessing.
	All you have to implement from whole interface \c iproc::IProcessor is method \c DoProcessing.
*/
template <class Base>
class TSyncProcessorWrap: public Base
{
public:
	typedef Base BaseClass;

	TSyncProcessorWrap();

	// pseudo-reimplemented (iproc::IProcessor)
	virtual IProcessor::ProcessorState GetProcessorState(const iprm::IParamsSet* paramsPtr) const override;
	virtual bool AreParamsAccepted(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				const istd::IChangeable* outputPtr) const override;
	virtual int BeginTask(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual IProcessor::TaskState WaitTaskFinished(
					int taskId = -1,
					double timeoutTime = -1,
					bool killOnTimeout = true) override;
	virtual void CancelTask(int taskId = -1) override;
	virtual int GetReadyTask() override;
	virtual IProcessor::TaskState GetTaskState(int taskId = -1) const override;
	virtual void InitProcessor(const iprm::IParamsSet* paramsPtr) override;

protected:
	// static methods for convinient access progress manager functionality
	static std::unique_ptr<ibase::IProgressManager> CreateSubtaskManager(
			ibase::IProgressManager* managerPtr,
			const QByteArray& taskId,
			const QString& taskDescription,
			double weight = 1.0);
	static std::unique_ptr<ibase::IProgressLogger> StartProgressLogger(ibase::IProgressManager* managerPtr, bool isCancelable = false);


private:
	typedef QMap<int, IProcessor::TaskState> TaskToStateMap;

	TaskToStateMap m_taskToStateMap;
	int m_nextTaskId;
};


// public methods

template <class Base>
TSyncProcessorWrap<Base>::TSyncProcessorWrap()
:	m_nextTaskId(0)
{
}


// pseudo-reimplemented (iproc::IProcessor)

template <class Base>
IProcessor::ProcessorState TSyncProcessorWrap<Base>::GetProcessorState(const iprm::IParamsSet* /*paramsPtr*/) const
{
	return IProcessor::PS_READY;
}


template <class Base>
bool TSyncProcessorWrap<Base>::AreParamsAccepted(
			const iprm::IParamsSet* /*paramsPtr*/,
			const istd::IPolymorphic* /*inputPtr*/,
			const istd::IChangeable* /*outputPtr*/) const
{
	return true;
}


template <class Base>
int TSyncProcessorWrap<Base>::BeginTask(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	int retVal = m_nextTaskId;
	m_taskToStateMap[retVal] = this->DoProcessing(paramsPtr, inputPtr, outputPtr, progressManagerPtr, processingReportPtr);

	m_nextTaskId = (m_nextTaskId + 1) & 0x7fff;

	return retVal;
}


template <class Base>
IProcessor::TaskState TSyncProcessorWrap<Base>::WaitTaskFinished(
			int taskId,
			double /*timeoutTime*/,
			bool /*killOnTimeout*/)
{
	IProcessor::TaskState retVal = IProcessor::TS_NONE;

	if (taskId >= 0){
		TaskToStateMap::iterator foundIter = m_taskToStateMap.find(taskId);
		if (foundIter != m_taskToStateMap.end()){
			retVal = foundIter.value();

			m_taskToStateMap.erase(foundIter);
		}
	}
	else{
		for (		TaskToStateMap::const_iterator iter = m_taskToStateMap.begin();
					iter != m_taskToStateMap.end();
					++iter){	// we are looking for worst status value. This values are sorted worst => bigger.
			IProcessor::TaskState taskState = iter.value();
			if (taskState > retVal){
				retVal = taskState;
			}
		}

		m_taskToStateMap.clear();
	}

	return retVal;
}


template <class Base>
void TSyncProcessorWrap<Base>::CancelTask(int taskId)
{
	if (taskId >= 0){
		m_taskToStateMap.remove(taskId);
	}
	else{
		m_taskToStateMap.clear();
	}
}


template <class Base>
int TSyncProcessorWrap<Base>::GetReadyTask()
{
	if (!m_taskToStateMap.isEmpty()){
		return m_taskToStateMap.begin().key();
	}
	else{
		return -1;
	}
}


template <class Base>
IProcessor::TaskState TSyncProcessorWrap<Base>::GetTaskState(int taskId) const
{
	IProcessor::TaskState retVal = IProcessor::TS_NONE;

	if (taskId >= 0){
		TaskToStateMap::ConstIterator foundIter = m_taskToStateMap.constFind(taskId);
		if (foundIter != m_taskToStateMap.constEnd()){
			retVal = foundIter.value();
		}
	}
	else{
		for (		TaskToStateMap::const_iterator iter = m_taskToStateMap.begin();
					iter != m_taskToStateMap.end();
					++iter){	// we are looking for worst status value. This values are sorted worst => bigger.
			IProcessor::TaskState taskState = iter.value();
			if (taskState > retVal){
				retVal = taskState;
			}
		}
	}

	return retVal;
}


template <class Base>
void TSyncProcessorWrap<Base>::InitProcessor(const iprm::IParamsSet* /*paramsPtr*/)
{
}


// static protected methods

template <class Base>
std::unique_ptr<ibase::IProgressManager> TSyncProcessorWrap<Base>::CreateSubtaskManager(
		ibase::IProgressManager* managerPtr,
		const QByteArray& taskId,
		const QString& taskDescription,
		double weight)
{
	if (managerPtr != nullptr){
		return managerPtr->CreateSubtaskManager(taskId, taskDescription, weight);
	}

	return nullptr;
}


template <class Base>
std::unique_ptr<ibase::IProgressLogger> TSyncProcessorWrap<Base>::StartProgressLogger(ibase::IProgressManager* managerPtr, bool isCancelable)
{
	if (managerPtr != nullptr){
		return managerPtr->StartProgressLogger(isCancelable);
	}

	return nullptr;
}


// global typdefs

typedef TSyncProcessorWrap<IProcessor> CSyncProcessorBase;


} // namespace iproc


#endif // !iproc_TSyncProcessorWrap_included


