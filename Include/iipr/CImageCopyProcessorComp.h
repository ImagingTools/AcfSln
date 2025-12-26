#ifndef iipr_CImageCopyProcessorComp_included
#define iipr_CImageCopyProcessorComp_included


// Qt includes
#include <QtCore/QString>

// ACF includes
#include <i2d/CRectangle.h>
#include <iimg/CScanlineMask.h>

// ACF-Solutions includes
#include <imeas/ILinearAdjustParams.h>
#include <imeas/INumericValue.h>
#include <iipr/CImageProcessorCompBase.h>


namespace iipr
{


/**
	Implementation of image copy with convertion of image type.
*/
class CImageCopyProcessorComp: public iipr::CImageProcessorCompBase
{
public:
	typedef iipr::CImageProcessorCompBase BaseClass;

	I_BEGIN_COMPONENT(CImageCopyProcessorComp);
	I_END_COMPONENT;

	static bool DoImageCopy(
				iimg::IBitmap::PixelFormat outputPixelFormat,
				const iimg::IBitmap& inputImage,
				const iimg::CScanlineMask& resultMask,
				iimg::IBitmap& outputImage,
				istd::ILogger* loggerPtr = nullptr);

protected:
	// reimplemented (iipr::CImageProcessorCompBase)
	virtual bool ProcessImage(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap& inputImage,
				iimg::IBitmap& outputImage) const;
};


} // namespace iipr


#endif // !iipr_CImageCopyProcessorComp_included


