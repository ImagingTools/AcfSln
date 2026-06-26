// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef _qcam_CCameraDelegatorBase_included
#define _qcam_CCameraDelegatorBase_included


// ACF includes
#include <ilog/TLoggerCompWrap.h>
#include <iprm/IParamsSet.h>

// ACF-Solutions includes
#include <icam/IBitmapAcquisition.h>


namespace icam
{


/**
	Implementation of bitmap acquisition delegating all calls to some slave object.
	It is used to provide cascades of camera implementations.
*/
class CCameraDelegatorComp:
			public ilog::CLoggerComponentBase,
			public IBitmapAcquisition
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;

	I_BEGIN_COMPONENT(CCameraDelegatorComp);
		I_REGISTER_INTERFACE(IBitmapAcquisition);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_ASSIGN(m_slaveCameraCompPtr, "SlaveCamera", "Slave camera object", true, "SlaveCamera");
		I_ASSIGN(m_parameterSetCompPtr, "ParamsSet", "Parameter set used for overriding of the input parameters", false, "ParamsSet");
	I_END_COMPONENT;

	// reimplemented (iipr::IBitmapAcquisition)
	virtual istd::CIndex2d GetBitmapSize(const iprm::IParamsSet* paramsPtr) const override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::ProcessorState GetProcessorState(const iprm::IParamsSet* paramsPtr) const override;
	virtual bool AreParamsAccepted(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				const istd::IChangeable* outputPtr) const override;
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual int BeginTask(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual iproc::IProcessor::TaskState WaitTaskFinished(
					int taskId = -1,
					double timeoutTime = -1,
					bool killOnTimeout = true) override;
	virtual void CancelTask(int taskId = -1) override;
	virtual int GetReadyTask() override;
	virtual iproc::IProcessor::TaskState GetTaskState(int taskId = -1) const override;
	virtual void InitProcessor(const iprm::IParamsSet* paramsPtr) override;

protected:
	virtual const iprm::IParamsSet* GetWorkingParamsSet(const iprm::IParamsSet* inputParamsPtr) const;

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

protected:
	I_REF(IBitmapAcquisition, m_slaveCameraCompPtr);
	I_REF(iprm::IParamsSet, m_parameterSetCompPtr);
};


} // namespace icam


#endif // !_qcam_CCameraDelegatorBase_included


