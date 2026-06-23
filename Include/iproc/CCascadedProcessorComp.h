// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iproc_CCascadedProcessorComp_included
#define iproc_CCascadedProcessorComp_included


#include <istd/IChangeable.h>

#include <icomp/CComponentBase.h>

#include <iproc/TSyncProcessorWrap.h>


namespace iproc
{
	

/**
	Processor starting sequence of cascaded processors to processing input object to the output one.
	To realize this task external objects are needed to store temporary results.
*/
class CCascadedProcessorComp:
			public icomp::CComponentBase,
			public CSyncProcessorBase
{
public:
	typedef icomp::CComponentBase BaseClass;
	typedef CSyncProcessorBase BaseClass2;

	I_BEGIN_COMPONENT(CCascadedProcessorComp);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_ASSIGN_MULTI_0(m_processorsCompPtr, "Processors", "List of processors", true);
		I_ASSIGN_MULTI_0(m_buffersCompPtr, "Buffers", "List of buffers storing data between processors", false);
		I_ASSIGN_MULTI_0(m_progressIdsAttrPtr, "ProgressIds", "List of progress ID's used for progress manager for each processor", false);
		I_ASSIGN_MULTI_0(m_progressDescriptionsAttrPtr, "ProgressDescriptions", "List of progress descriptions used for progress manager for each processor", false);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual IProcessor::ProcessorState GetProcessorState(const iprm::IParamsSet* paramsPtr) const;
	virtual bool AreParamsAccepted(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				const istd::IChangeable* outputPtr) const;
	virtual IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL);
	virtual void InitProcessor(const iprm::IParamsSet* paramsPtr);

protected:
	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	I_MULTIREF(iproc::IProcessor, m_processorsCompPtr);
	I_MULTIREF(istd::IChangeable, m_buffersCompPtr);
	I_MULTIATTR(QByteArray, m_progressIdsAttrPtr);
	I_MULTITEXTATTR(m_progressDescriptionsAttrPtr);
};


} // namespace iproc


#endif // !iproc_CCascadedProcessorComp_included

