// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// ACF includes
#include <icam/CCameraDelegatorComp.h>


namespace icam
{


/**
	Camera delegator with integrated pre- and post-processing.
*/
class CProcessedCameraComp: public icam::CCameraDelegatorComp
{
public:
	typedef icam::CCameraDelegatorComp BaseClass;

	I_BEGIN_COMPONENT(CProcessedCameraComp);
		I_ASSIGN(m_preProcessorCompPtr, "PreProcessor", "Processor called before snap with camera parameters", false, "PreProcessor");
		I_ASSIGN(m_postProcessorCompPtr, "PostProcessor", "Processor called after snap with camera parameters", false, "PostProcessor");
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual int BeginTask(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

private:
	I_REF(iproc::IProcessor, m_preProcessorCompPtr);
	I_REF(iproc::IProcessor, m_postProcessorCompPtr);
};


} // namespace icam


