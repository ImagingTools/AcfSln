#ifndef iipr_CImageCropProcessorComp_included
#define iipr_CImageCropProcessorComp_included


// ACF-Solutions includes
#include <istd/ILogger.h>

// ACF-Solutions includes
#include <imeas/IDiscreteDataSequence.h>
#include <iipr/CImageRegionProcessorCompBase.h>


namespace iipr
{


/**	
	Processor for image cropping.
*/
class CImageCropProcessorComp: public CImageRegionProcessorCompBase
{
public:
	typedef CImageRegionProcessorCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CImageCropProcessorComp);
	I_END_COMPONENT;

	static bool DoImageCrop(
				const iimg::IBitmap& inputBitmap,
				const i2d::IObject2d& aoi,
				iimg::IBitmap& outputBitmap,
				istd::ILogger* loggerPtr = nullptr);

protected:
	// reimplemented (CImageRegionProcessorCompBase)
	virtual bool ProcessImageRegion(
				const iimg::IBitmap& inputBitmap,
				const iprm::IParamsSet* paramsPtr,
				const i2d::IObject2d* aoiPtr,
				istd::IChangeable* outputPtr) const;
};


} // namespace iipr


#endif // !iipr_CImageCropProcessorComp_included


