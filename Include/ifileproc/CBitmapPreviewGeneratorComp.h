// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CBitmapPreviewGeneratorComp_included
#define ifileproc_CBitmapPreviewGeneratorComp_included


// ACF includes
#include <istd/TSmartPtr.h>
#include <iproc/TSyncProcessorCompBase.h>
#include <iimg/IBitmap.h>


namespace ifileproc
{


/**
	Generator of the bitmap preview.
*/
class CBitmapPreviewGeneratorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;

	I_BEGIN_COMPONENT(CBitmapPreviewGeneratorComp);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
};


} // namespace ifileproc


#endif // !ifileproc_CBitmapPreviewGeneratorComp_included


