// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iproc_CIterativeProcessorComp_included
#define iproc_CIterativeProcessorComp_included


// Qt includes
#include <QtCore/QString>
#include <QtCore/QVector>

// ACF includes
#include <ilog/TLoggerCompWrap.h>

#include <iprm/IOptionsList.h>

#include <iproc/TSyncProcessorWrap.h>


namespace iproc
{
	

/**
	Processor wrapper for multiple executing of a slave processor.
	Input at k-iteration for the slave operator is processor output at (k-1)-iteration.
*/
class CIterativeProcessorComp:
			public ilog::CLoggerComponentBase,
			public iproc::CSyncProcessorBase,
			virtual public iprm::IOptionsList
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;
	typedef iproc::CSyncProcessorBase BaseClass2;

	enum MessageId
	{
		MI_BAD_BUFFER = 0x6b30
	};

	I_BEGIN_COMPONENT(CIterativeProcessorComp);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_REGISTER_INTERFACE(iprm::IOptionsList);
		I_ASSIGN(m_paramsIdAttrPtr, "IterationParamId", "ID of parameter defining number of iterations (type iprm::ISelectionParam)", true, "ParamsId");
		I_ASSIGN(m_maxIterationsCountAttrPtr, "MaxIterations", "Maximal number of iterations", true, 10);
		I_ASSIGN(m_bufferObjectCompPtr, "BufferObject", "Object used as buffer between single processing steps", true, "BufferObject");
		I_ASSIGN(m_slaveProcessorCompPtr, "SlaveProcessor", "Slave image processor", true, "SlaveProcessor");
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (iprm::IOptionsList)
	virtual int GetOptionsFlags() const override;
	virtual int GetOptionsCount() const override;
	virtual QString GetOptionName(int index) const override;
	virtual QString GetOptionDescription(int index) const override;
	virtual QByteArray GetOptionId(int index) const override;
	virtual bool IsOptionEnabled(int index) const override;

protected:
	iproc::IProcessor::TaskState ProcessSlave(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr);

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	I_ATTR(QByteArray, m_paramsIdAttrPtr);
	I_ATTR(int, m_maxIterationsCountAttrPtr);
	I_REF(istd::IChangeable, m_bufferObjectCompPtr);
	I_REF(iproc::IProcessor, m_slaveProcessorCompPtr);

	QVector<QString> m_iterationNames;
};


} // namespace iproc


#endif // !iproc_CIterativeProcessorComp_included


