#ifndef iipr_CImageIntegralProcessorComp_included
#define iipr_CImageIntegralProcessorComp_included


// ACF includes
#include <istd/IPolymorphic.h>
#include <iprm/IParamsSet.h>
#include <iimg/IBitmap.h>
#include <iimg/CScanlineMask.h>
#include <iipr/CImageProcessorCompBase.h>


namespace iipr
{


/**
	Helper class allowing to find perspective calibration using list of nominal and found positions.
*/
class CImageIntegralProcessorComp: public CImageProcessorCompBase
{
public:
	/**
		Calculates integral image.
		\param	inputImage			input image.
		\param	outputImage			result image, will be initializated.
		\param	maskPtr				optional mask. Points out of mask will be not touched inside of input image.
		\param	outputPixelFormat	requested format of output image or \c iimg::IBitmap::PF_UNKNOWN if it should be automatically calculated.
		\param	fillOutOfMask		if true, the values out of mask will be calculated for output, if false they will be ignored (better performance).
	*/
	static bool CalculateIntegralImage(
				const iimg::IBitmap& inputImage,
				iimg::IBitmap& outputImage,
				const iimg::CScanlineMask* maskPtr = nullptr,
				iimg::IBitmap::PixelFormat outputPixelFormat = iimg::IBitmap::PF_UNKNOWN,
				bool fillOutOfMask = true);

	/**
		Calculates integral image in place.
		\param	image	image will be converted to its integral.
	*/
	static bool MakeIntegralImage(iimg::IBitmap& outputImage);

	/**
		Calculates integral reference image.
		This image will be generated for some binary mask and contains integral image of image function with all pixels equals to 1.
		It can be usefull to calculate area of mask subregions.
		\param	mask				Input mask image.
		\param	outputImage			result image, will not be initializated, pixel format and size must be set by caller.
	*/
	static bool CalculateIntegralReference(
				const iimg::CScanlineMask& mask,
				iimg::IBitmap& outputImage);

protected:
	// reimplemented (iipr::CImageProcessorCompBase)
	virtual bool ProcessImage(
				const iprm::IParamsSet* paramsPtr, 
				const iimg::IBitmap& inputImage,
				iimg::IBitmap& outputImage) const override;
};


} // namespace iipr


#endif // !iipr_CImageIntegralProcessorComp_included


