// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iprocgui_CDocumentProcessingManagerComp_included
#define iprocgui_CDocumentProcessingManagerComp_included


// ACF includes
#include <istd/IChangeable.h>
#include <iprocgui/CDocumentProcessingManagerCompBase.h>


namespace iprocgui
{


/**
	Component to trigger document-to-document processing action.

	You can use \c m_inPlaceProcessingAttrPtr to define, 
	weither the new document is created for the processing result or the document is processed "in-place".
*/
class CDocumentProcessingManagerComp: public iprocgui::CDocumentProcessingManagerCompBase
{
public:
	typedef iprocgui::CDocumentProcessingManagerCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CDocumentProcessingManagerComp);
		I_ASSIGN(m_inPlaceProcessingAttrPtr, "InPlaceProcessing", "If enabled, the input document will be the result of processing", false, false);
	I_END_COMPONENT;

protected:
	// reimplemented (iprocgui::CDocumentProcessingManagerCompBase)
	bool PrepareProcessing(
				const istd::IChangeable* inputDocumentPtr,
				const QByteArray& documentTypeId,
				ibase::IProgressManager* progressManagerPtr,
				istd::IChangeable*& outputDocumentPtr,
				istd::IChangeable*& changeTargetPtr) override;
	void FinalizeProcessing(
				const istd::IChangeable* inputDocumentPtr,
				const QByteArray& documentTypeId,
				istd::IChangeable* outputDocumentPtr,
				int resultCode,
				double processingTime,
				istd::CChangeNotifier& changeNotifier) override;

private:
	bool PrepareProcessingToOutput(
				const istd::IChangeable* inputDocumentPtr,
				const QByteArray& documentTypeId,
				ibase::IProgressManager* progressManagerPtr,
				istd::IChangeable*& outputDocumentPtr,
				istd::IChangeable*& changeTargetPtr);
	bool PrepareInPlaceProcessing(
				const istd::IChangeable* inputDocumentPtr,
				ibase::IProgressManager* progressManagerPtr,
				istd::IChangeable*& outputDocumentPtr,
				istd::IChangeable*& changeTargetPtr);

	void FinalizeProcessingToOutput(
				istd::IChangeable* outputDocumentPtr,
				int resultCode,
				double processingTime,
				istd::CChangeNotifier& changeNotifier);
	void FinalizeInPlaceProcessing(
				const istd::IChangeable* inputDocumentPtr,
				istd::IChangeable* outputDocumentPtr,
				int resultCode,
				double processingTime,
				istd::CChangeNotifier& changeNotifier);

private:
	I_ATTR(bool, m_inPlaceProcessingAttrPtr);

	// State preserved between PrepareProcessing and FinalizeProcessing:
	istd::IChangeableSharedPtr m_pendingOutputSharedPtr;
	istd::IChangeableUniquePtr m_pendingOutputUniquePtr;
	int m_pendingDocumentIndex;
};


} // namespace iproc


#endif // !iprocgui_CDocumentProcessingManagerComp_included
