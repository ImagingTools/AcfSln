#ifndef iipr_CGrayToRgbProcessorComp_included
#define iipr_CGrayToRgbProcessorComp_included


// ACF includes
#include <iimg/IBitmap.h>

// ACF-Solutions includes
#include <iproc/TSyncProcessorCompBase.h>


namespace iipr
{


/**
	Implementation of a processor for the Grayscale-To-RGB image conversion.
*/
class CGrayToRgbProcessorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CGrayToRgbProcessorComp);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

private:
	bool ConvertImage(const iimg::IBitmap& inputBitmap, iimg::IBitmap& outputBitmap) const;
};


} // namespace iipr


#endif // !iipr_CGrayToRgbProcessorComp_included

