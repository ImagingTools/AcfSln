// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iprocgui/CDocumentProcessingWorkerThread.h>

#include <iprocgui/CDocumentProcessingManagerCompBase.h>


namespace iprocgui
{


CDocumentProcessingWorkerThread::CDocumentProcessingWorkerThread(
			CDocumentProcessingManagerCompBase* managerPtr,
			const istd::IChangeable* inputDocumentPtr,
			const QByteArray& documentTypeId,
			ibase::IProgressManager* progressPtr)
:	m_managerPtr(managerPtr),
	m_inputDocumentPtr(inputDocumentPtr),
	m_documentTypeId(documentTypeId),
	m_progressPtr(progressPtr)
{
}


// reimplemented (QThread)

void CDocumentProcessingWorkerThread::run()
{
	m_managerPtr->DoDocumentProcessing(m_inputDocumentPtr, m_documentTypeId, m_progressPtr);
}


} // namespace iprocgui


