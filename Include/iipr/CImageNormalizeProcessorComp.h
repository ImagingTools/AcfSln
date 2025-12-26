#ifndef iipr_CImageNormalizeProcessorComp_included
#define iipr_CImageNormalizeProcessorComp_included


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
	Implementation of contrast and brightness adjustment processor supporting dynamic correction based on histogram analyse.
*/
class CImageNormalizeProcessorComp: public iipr::CImageProcessorCompBase
{
public:
	typedef iipr::CImageProcessorCompBase BaseClass;

	I_BEGIN_COMPONENT(CImageNormalizeProcessorComp);
		I_ASSIGN(m_adjustParamsIdAttrPtr, "AdjustParamsId", "ID of parameters for contrast and brightness in parameter set", false, "AdjustParamsId");
		I_ASSIGN(m_defaultAdjustParamsCompPtr, "DefaultAdjustParams", "Default adjust parameters, if no parameters are specified", false, "DefaultAdjustParams");
		I_ASSIGN(m_clippingThresholdIdAttrPtr, "HistogramClipThresholdId", "ID of the histogram clipping threshold in parameter set, only histogram parts over this area will be processed", false, "HistogramClipThreshold");
		I_ASSIGN(m_defaultClippingThresholdCompPtr, "DefaultHistogramClipThreshold", "Default parameter for clipping threshold, if no parameters are specified", false, "DefaultHistogramClipThreshold");
		I_ASSIGN(m_useHistogramAdjustmentCompPtr, "UseHistogramAdjustment", "If enabled the histogram adjustment will be done before appling the other parameters", true, false);
	I_END_COMPONENT;

	static bool DoAdjustFilter(
				double contrast,
				double brightness,
				int backgroundMode,
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

private:
	I_ATTR(QByteArray, m_adjustParamsIdAttrPtr);
	I_REF(imeas::ILinearAdjustParams, m_defaultAdjustParamsCompPtr);
	I_ATTR(QByteArray, m_clippingThresholdIdAttrPtr);
	I_REF(imeas::INumericValue, m_defaultClippingThresholdCompPtr);
	I_ATTR(bool, m_useHistogramAdjustmentCompPtr);
};


} // namespace iipr


#endif // !iipr_CImageNormalizeProcessorComp_included


