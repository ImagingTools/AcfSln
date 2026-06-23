// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CImageBinarizeProcessorComp_included
#define iipr_CImageBinarizeProcessorComp_included


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
class CImageBinarizeProcessorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CImageBinarizeProcessorComp);
		I_ASSIGN(m_binarizationParamsIdAttrPtr, "BinarizationParameterId", "ID of binarization parameters in the parameter set", false, "BinarizationParameterId");
		I_ASSIGN(m_defaultBinarizationThresholdCompPtr, "DefaultThreshold", "Default value of the binarization threshold", false, "DefaultThreshold");
		I_ASSIGN(m_isInverseModeEnabledAttrPtr, "InverseMode", "If enabled, object color will be set to 0", true, false);
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
				const imeas::INumericValue& thresholdParams,
				iimg::IBitmap& outputBitmap) const;

private:
	I_ATTR(QByteArray, m_binarizationParamsIdAttrPtr);
	I_REF(imeas::INumericValue, m_defaultBinarizationThresholdCompPtr);
	I_ATTR(bool, m_isInverseModeEnabledAttrPtr);
};


} // namespace iipr


#endif // !iipr_CImageBinarizeProcessorComp_included

