// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iprocgui/CDocumentProcessingWorkerThread.h>

#include <istd/CGeneralTimeStamp.h>


namespace iprocgui
{


CDocumentProcessingWorkerThread::CDocumentProcessingWorkerThread(
			iproc::IProcessor* processorPtr,
			const iprm::IParamsSet* paramsSetPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressPtr)
:	m_processorPtr(processorPtr),
	m_paramsSetPtr(paramsSetPtr),
	m_inputPtr(inputPtr),
	m_outputPtr(outputPtr),
	m_progressPtr(progressPtr),
	m_resultCode(iproc::IProcessor::TS_INVALID),
	m_processingTime(0.0)
{
}


int CDocumentProcessingWorkerThread::GetResultCode() const
{
	return m_resultCode;
}


double CDocumentProcessingWorkerThread::GetProcessingTime() const
{
	return m_processingTime;
}


// reimplemented (QThread)

void CDocumentProcessingWorkerThread::run()
{
	istd::CGeneralTimeStamp timer;

	m_resultCode = m_processorPtr->DoProcessing(m_paramsSetPtr, m_inputPtr, m_outputPtr, m_progressPtr);

	m_processingTime = timer.GetElapsed();
}


} // namespace iprocgui


