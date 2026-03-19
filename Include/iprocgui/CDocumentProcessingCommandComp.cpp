// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iprocgui/CDocumentProcessingCommandComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>

#include <iqtgui/CGuiComponentDialog.h>


namespace iprocgui
{


// protected methods

// reimplemented (iprocgui::CDocumentProcessingManagerCompBase)

bool CDocumentProcessingCommandComp::PrepareProcessing(
			const istd::IChangeable* /*inputDocumentPtr*/,
			const QByteArray& /*documentTypeId*/,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable*& outputDocumentPtr,
			istd::IChangeable*& changeTargetPtr)
{
	if (!m_outputDataCompPtr.IsValid()){
		SendErrorMessage(0, "Processing result data model not set");

		return false;
	}

	outputDocumentPtr = m_outputDataCompPtr.GetPtr();
	changeTargetPtr = m_outputDataCompPtr.GetPtr();

	return true;
}


void CDocumentProcessingCommandComp::FinalizeProcessing(
			const istd::IChangeable* /*inputDocumentPtr*/,
			const QByteArray& /*documentTypeId*/,
			istd::IChangeable* /*outputDocumentPtr*/,
			int resultCode,
			double processingTime,
			istd::CChangeNotifier& changeNotifier)
{
	if (resultCode != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "Processing was failed", "Document processing manager");

		return;
	}

	SendInfoMessage(0, QObject::tr("Processing time: %1 ms").arg(processingTime * 1000, 2, 'f', 2), "Document processing manager");

	changeNotifier.Reset();

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


