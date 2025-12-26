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
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

private:
	bool ConvertImage(
				const iimg::IBitmap& inputBitmap,
				iimg::IBitmap& outputBitmap,
			double minContrastThreshold) const;

private:
	I_ATTR(QByteArray, m_binarizationParamsIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CAdaptiveImageBinarizeProcessorComp_included

