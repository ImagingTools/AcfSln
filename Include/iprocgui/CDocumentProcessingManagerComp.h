// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iprocgui_CDocumentProcessingManagerComp_included
#define iprocgui_CDocumentProcessingManagerComp_included


// ACF includes
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
	void DoDocumentProcessing(
				const istd::IChangeable* inputDocumentPtr,
				const QByteArray& documentTypeId,
				ibase::IProgressManager* progressManagerPtr);

private:
	void DoProcessingToOutput(const istd::IChangeable* inputDocumentPtr, const QByteArray& documentTypeId, ibase::IProgressManager* progressManagerPtr);
	void DoInPlaceProcessing(istd::IChangeable* inputDocumentPtr, ibase::IProgressManager* progressManagerPtr);

private:
	I_ATTR(bool, m_inPlaceProcessingAttrPtr);
};


} // namespace iproc


#endif // !iprocgui_CDocumentProcessingManagerComp_included
