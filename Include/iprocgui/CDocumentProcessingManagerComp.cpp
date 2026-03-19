// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iprocgui/CDocumentProcessingManagerComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>


namespace iprocgui
{


// protected methods

// reimplemented (iprocgui::CDocumentProcessingManagerCompBase)

bool CDocumentProcessingManagerComp::PrepareProcessing(
			const istd::IChangeable* inputDocumentPtr,
			const QByteArray& documentTypeId,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable*& outputDocumentPtr,
			istd::IChangeable*& changeTargetPtr)
{
	if (m_inPlaceProcessingAttrPtr.IsValid() && *m_inPlaceProcessingAttrPtr){
		return PrepareInPlaceProcessing(inputDocumentPtr, progressManagerPtr, outputDocumentPtr, changeTargetPtr);
	}
	else{
		return PrepareProcessingToOutput(inputDocumentPtr, documentTypeId, progressManagerPtr, outputDocumentPtr, changeTargetPtr);
	}
}


void CDocumentProcessingManagerComp::FinalizeProcessing(
			const istd::IChangeable* inputDocumentPtr,
			const QByteArray& documentTypeId,
			istd::IChangeable* outputDocumentPtr,
			int resultCode,
			double processingTime,
			istd::CChangeNotifier& changeNotifier)
{
	if (m_inPlaceProcessingAttrPtr.IsValid() && *m_inPlaceProcessingAttrPtr){
		FinalizeInPlaceProcessing(inputDocumentPtr, outputDocumentPtr, resultCode, processingTime, changeNotifier);
	}
	else{
		FinalizeProcessingToOutput(outputDocumentPtr, resultCode, processingTime, changeNotifier);
	}
}


// private methods

bool CDocumentProcessingManagerComp::PrepareProcessingToOutput(
			const istd::IChangeable* /*inputDocumentPtr*/,
			const QByteArray& documentTypeId,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable*& outputDocumentPtr,
			istd::IChangeable*& changeTargetPtr)
{
	bool ignoredFlag = false;
	if (!m_documentManagerCompPtr->InsertNewDocument(documentTypeId, false, "", &m_pendingOutputSharedPtr, true, &ignoredFlag)){
		if (!ignoredFlag){
			SendErrorMessage(0, "Output document could not be created", "Document processing manager");
		}

		return false;
	}

	Q_ASSERT(m_pendingOutputSharedPtr.IsValid());

	m_pendingDocumentIndex = -1;

	int documentCounts = m_documentManagerCompPtr->GetDocumentsCount();
	for (int docIndex = 0; docIndex < documentCounts; docIndex++){
		istd::IChangeable& document = m_documentManagerCompPtr->GetDocumentFromIndex(docIndex);
		if (&document == m_pendingOutputSharedPtr.GetPtr()){
			m_pendingDocumentIndex = docIndex;
			break;
		}
	}

	Q_ASSERT(m_pendingDocumentIndex >= 0);

	if (progressManagerPtr != nullptr){
		progressManagerPtr->ResetProgressManager();
	}

	outputDocumentPtr = m_pendingOutputSharedPtr.GetPtr();
	changeTargetPtr = m_pendingOutputSharedPtr.GetPtr();

	return true;
}


bool CDocumentProcessingManagerComp::PrepareInPlaceProcessing(
			const istd::IChangeable* inputDocumentPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable*& outputDocumentPtr,
			istd::IChangeable*& changeTargetPtr)
{
	if (inputDocumentPtr == NULL){
		SendErrorMessage(0, "No input document", "Document processing manager");

		return false;
	}

	m_pendingOutputUniquePtr = inputDocumentPtr->CloneMe();
	if (!m_pendingOutputUniquePtr.IsValid()){
		SendErrorMessage(0, "Result object could not be created", "Document processing manager");

		return false;
	}

	if (progressManagerPtr != nullptr){
		progressManagerPtr->ResetProgressManager();
	}

	outputDocumentPtr = m_pendingOutputUniquePtr.GetPtr();
	changeTargetPtr = const_cast<istd::IChangeable*>(inputDocumentPtr);

	m_pendingOutputSharedPtr.Reset();
	m_pendingDocumentIndex = -1;

	return true;
}


void CDocumentProcessingManagerComp::FinalizeProcessingToOutput(
			istd::IChangeable* outputDocumentPtr,
			int resultCode,
			double processingTime,
			istd::CChangeNotifier& changeNotifier)
{
	SendVerboseMessage(QObject::tr("Processing time: %1 ms").arg(processingTime, 2, 'f', 2), "Document processing manager");

	if (resultCode != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "Processing was failed", "Document processing manager");

		changeNotifier.Abort();

		m_documentManagerCompPtr->CloseDocument(m_pendingDocumentIndex, true);
	}
	else{
		istd::IPolymorphic* viewPtr = m_documentManagerCompPtr->AddViewToDocument(*m_pendingOutputSharedPtr);
		if (viewPtr == NULL){
			SendErrorMessage(0, "Output view could not be created", "Document processing manager");

			changeNotifier.Abort();

			m_documentManagerCompPtr->CloseDocument(m_pendingDocumentIndex, true);
		}
	}

	m_pendingOutputSharedPtr.Reset();
	m_pendingDocumentIndex = -1;
}


void CDocumentProcessingManagerComp::FinalizeInPlaceProcessing(
			const istd::IChangeable* inputDocumentPtr,
			istd::IChangeable* outputDocumentPtr,
			int resultCode,
			double processingTime,
			istd::CChangeNotifier& /*changeNotifier*/)
{
	SendInfoMessage(0, QObject::tr("Processing time: %1 ms").arg(processingTime * 1000, 2, 'f', 2), "Document processing manager");

	if (resultCode != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "Processing was failed", "Document processing manager");

		m_pendingOutputUniquePtr.Reset();

		return;
	}

	istd::IChangeable* mutableInputPtr = const_cast<istd::IChangeable*>(inputDocumentPtr);
	if (!mutableInputPtr->CopyFrom(*outputDocumentPtr)){
		SendErrorMessage(0, "Result object is incompatible", "Document processing manager");

		m_pendingOutputUniquePtr.Reset();

		return;
	}

	m_pendingOutputUniquePtr.Reset();
}


} // namespace iprocgui


