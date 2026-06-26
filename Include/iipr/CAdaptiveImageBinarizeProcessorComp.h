// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CAdaptiveImageBinarizeProcessorComp_included
#define iipr_CAdaptiveImageBinarizeProcessorComp_included


// ACF includes
#include <iimg/IBitmap.h>
#include <iproc/TSyncProcessorCompBase.h>

// ACF-Solutions includes
#include <imeas/INumericValue.h>


namespace iipr
{


/**	
	Processor for image binarization using static threshold.
*/
class CAdaptiveImageBinarizeProcessorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CAdaptiveImageBinarizeProcessorComp);
		I_ASSIGN(m_binarizationParamsIdAttrPtr, "BinarizationParameterId", "ID of binarization parameters in the parameter set", false, "BinarizationParameterId");
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

private:
	bool ConvertImage(
				const iimg::IBitmap& inputBitmap,
				iimg::IBitmap& outputBitmap) const;

private:
	I_ATTR(QByteArray, m_binarizationParamsIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CAdaptiveImageBinarizeProcessorComp_included

