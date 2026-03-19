// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// ACF includes
#include <iprocgui/CDocumentProcessingManagerCompBase.h>


namespace iprocgui
{


/**
	Component to trigger some processing action for the current document.
*/
class CDocumentProcessingCommandComp: public iprocgui::CDocumentProcessingManagerCompBase
{
public:
	typedef iprocgui::CDocumentProcessingManagerCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CDocumentProcessingCommandComp);
		I_ASSIGN(m_outputDataCompPtr, "OutputData", "Result of document processing", true, "OutputData");
		I_ASSIGN(m_outputDataGuiCompPtr, "OutputDataGui", "UI to show the output data object", false, "OutputDataGui");
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
	I_REF(istd::IChangeable, m_outputDataCompPtr);
	I_REF(iqtgui::IGuiObject, m_outputDataGuiCompPtr);
};


} // namespace iprocgui


