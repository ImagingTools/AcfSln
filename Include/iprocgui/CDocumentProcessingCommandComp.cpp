// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iprocgui/CDocumentProcessingCommandComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>
#include <istd/CGeneralTimeStamp.h>

#include <iqtgui/CGuiComponentDialog.h>


namespace iprocgui
{


// protected methods

// reimplemented (iprocgui::CDocumentProcessingManagerCompBase)

void CDocumentProcessingCommandComp::DoDocumentProcessing(const istd::IChangeable* inputDocumentPtr, const QByteArray& /*documentTypeId*/, ibase::IProgressManager* progressManagerPtr)
{
	if (!m_outputDataCompPtr.IsValid()){
		SendErrorMessage(0, "Processing result data model not set");

		return;
	}

	istd::CChangeNotifier changePtr(m_outputDataCompPtr.GetPtr());

	istd::CGeneralTimeStamp timer;

	int retVal = m_processorCompPtr->DoProcessing(
				m_paramsSetCompPtr.GetPtr(),
				inputDocumentPtr,
				m_outputDataCompPtr.GetPtr(),
				progressManagerPtr);
	
	if (retVal != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "Processing was failed", "Document processing manager");

		return;
	}

	double processingTime = timer.GetElapsed();

	SendInfoMessage(0, QObject::tr("Processing time: %1 ms").arg(processingTime * 1000, 2, 'f', 2), "Document processing manager");

	changePtr.Reset();

	// show results in the dialog:
	istd::TDelPtr<iqtgui::CGuiComponentDialog> dialogPtr;

	if (m_outputDataGuiCompPtr.IsValid()){
		dialogPtr.SetPtr(
					new iqtgui::CGuiComponentDialog(
								m_outputDataGuiCompPtr.GetPtr(),
								QDialogButtonBox::Ok,
								true));

		dialogPtr->exec();
	}
}


} // namespace iprocgui


