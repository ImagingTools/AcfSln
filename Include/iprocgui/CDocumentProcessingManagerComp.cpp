// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iprocgui/CDocumentProcessingManagerComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>
#include <istd/CGeneralTimeStamp.h>


namespace iprocgui
{


// protected methods

// reimplemented (iprocgui::CDocumentProcessingManagerCompBase)

void CDocumentProcessingManagerComp::DoDocumentProcessing(
			const istd::IChangeable* inputDocumentPtr,
			const QByteArray& documentTypeId,
			ibase::IProgressManager* progressManagerPtr)
{
	if (m_inPlaceProcessingAttrPtr.IsValid() && *m_inPlaceProcessingAttrPtr){
		DoInPlaceProcessing(const_cast<istd::IChangeable*>(inputDocumentPtr), progressManagerPtr);
	}
	else{
		DoProcessingToOutput(inputDocumentPtr, documentTypeId, progressManagerPtr);
	}
}


// private methods

void CDocumentProcessingManagerComp::DoProcessingToOutput(
			const istd::IChangeable* inputDocumentPtr,
			const QByteArray& documentTypeId,
			ibase::IProgressManager* progressManagerPtr)
{
	istd::IChangeableSharedPtr outputDocumentPtr;
	bool ignoredFlag = false;
	if (!m_documentManagerCompPtr->InsertNewDocument(documentTypeId, false, "", &outputDocumentPtr, true, &ignoredFlag)){
		if (!ignoredFlag){
			SendErrorMessage(0, "Output document could not be created", "Document processing manager");
		}

		return;
	}

	Q_ASSERT(outputDocumentPtr.IsValid());

	int documentIndex = -1;

	int documentCounts = m_documentManagerCompPtr->GetDocumentsCount();
	for (int docIndex = 0; docIndex < documentCounts; docIndex++){
		istd::IChangeable& document = m_documentManagerCompPtr->GetDocumentFromIndex(docIndex);
		if (&document == outputDocumentPtr.GetPtr()){
			documentIndex = docIndex;
			break;
		}
	}

	Q_ASSERT(documentIndex >= 0);

	istd::CChangeNotifier changePtr(outputDocumentPtr.GetPtr());

	if (progressManagerPtr != nullptr){
		progressManagerPtr->ResetProgressManager();
	}

	istd::CGeneralTimeStamp timer;

	int retVal = m_processorCompPtr->DoProcessing(
				m_paramsSetCompPtr.GetPtr(),
				inputDocumentPtr,
				outputDocumentPtr.GetPtr(),
				progressManagerPtr);

	double processingTime = timer.GetElapsed();

	SendVerboseMessage(QObject::tr("Processing time: %1 ms").arg(processingTime, 2, 'f', 2), "Document processing manager");

	if (retVal != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "Processing was failed", "Document processing manager");

		changePtr.Abort();

		m_documentManagerCompPtr->CloseDocument(documentIndex, true);
	}
	else{
		istd::IPolymorphic* viewPtr = m_documentManagerCompPtr->AddViewToDocument(*outputDocumentPtr);
		if (viewPtr == NULL){
			SendErrorMessage(0, "Output view could not be created", "Document processing manager");

			changePtr.Abort();
	
			m_documentManagerCompPtr->CloseDocument(documentIndex, true);
		}
	}
}


void CDocumentProcessingManagerComp::DoInPlaceProcessing(istd::IChangeable* inputDocumentPtr, ibase::IProgressManager* progressManagerPtr)
{
	if (inputDocumentPtr == NULL){
		SendErrorMessage(0, "No input document", "Document processing manager");

		return;
	}

	istd::CChangeNotifier changePtr(inputDocumentPtr);

	istd::IChangeableUniquePtr outputDocumentPtr(inputDocumentPtr->CloneMe());
	if (!outputDocumentPtr.IsValid()){
		SendErrorMessage(0, "Result object could not be created", "Document processing manager");
		
		return;
	}

	if (progressManagerPtr != nullptr){
		progressManagerPtr->ResetProgressManager();
	}

	istd::CGeneralTimeStamp timer;

	int retVal = m_processorCompPtr->DoProcessing(
				m_paramsSetCompPtr.GetPtr(),
				inputDocumentPtr,
				outputDocumentPtr.GetPtr(),
				progressManagerPtr);

	double processingTime = timer.GetElapsed();

	SendInfoMessage(0, QObject::tr("Processing time: %1 ms").arg(processingTime * 1000, 2, 'f', 2), "Document processing manager");

	if (retVal != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "Processing was failed", "Document processing manager");

		return;
	}

	if (!inputDocumentPtr->CopyFrom(*outputDocumentPtr.GetPtr())){
		SendErrorMessage(0, "Result object is incompatible", "Document processing manager");

		return;
	}
}


} // namespace iprocgui


