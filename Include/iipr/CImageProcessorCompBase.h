// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CImageProcessorCompBase_included
#define iipr_CImageProcessorCompBase_included


// ACF includes
#include <istd/TSmartPtr.h>
#include <iprm/ISelectionParam.h>
#include <iimg/IBitmap.h>

// ACF-Solutions includes
#include <iproc/TSyncProcessorCompBase.h>


namespace iipr
{


/**
	Basic implementation for a simple bitmap processor.
*/
class CImageProcessorCompBase: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;

	/**
		Describes modes of AOI interpretation.
		It is placed here to allow the same value interpretation for all image processors.
	*/
	enum AoiMode
	{
		/**
			AOI mask contains the input bitmap pixels.
		*/
		AM_INPUT_PIXELS,
		/**
			AOI mask contains the output bitmap pixels.
		*/
		AM_OUTPUT_PIXELS
	};

	/**
		Describes modes for out-of-AOI background fill.
		It is placed here to allow the same value interpretation for all image processors.
	*/
	enum BackgroundFillMode
	{
		/**
			The state of background pixels will be unknown.
		*/
		BFM_NONE,
		/**
			The background pixels will be reset (typically set to 0).
		*/
		BFM_RESET,
		/**
			The state of background pixels should be unchanged to original bitmap.
		*/
		BFM_UNCHANGED,
		/**
			The state of background pixels should be set to input bitmap values.
		*/
		BFM_INPUT
	};

	I_BEGIN_BASE_COMPONENT(CImageProcessorCompBase);
		I_ASSIGN(m_defaultAoiCompPtr, "DefaultAoi", "Area of interest used if not specified in parameters", false, "DefaultAoi");
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of area of interest in parameter set", false, "AoiParams");
		I_ASSIGN(m_aoiModeAttrPtr, "AoiMode", "Mode of interpretation of AOI:\n\t0 - AOI contains input pixels,\n\t1 - AOI contains output pixels", true, 0);
		I_ASSIGN(m_backgroundModeAttrPtr, "BackgroundMode", "Mode of background filling:\n\t0 - ignored,\n\t1 - set to 0,\n\t3 - set to input bitmap", true, 1);
		I_ASSIGN(m_outputPixelTypeAttrPtr, "OutputPixelType", "Type of output pixel if not defined over parameters:\n\t0 - As input\n\t1 - Mono\n\t2 - Grayscale\n\t3 - RGB\n\t4 - RGBA\n\t5 - RGB24\n\t6 - Grayscale 16\n\t7 - Grayscale 32\n\t8 - Float 32\n\t9 - Float 64", true, 0);
		I_ASSIGN(m_outputPixelTypeIdAttrPtr, "OutputPixelTypeId", "ID of output pixel type in parameter set (type iprm::ISelectionParam)", false, "OutputPixelType");
		I_ASSIGN(m_defaultOutputPixelTypeParamCompPtr, "DefaultOutputPixelType", "Default output pixel type if not defined in parameter set", false, "DefaultOutputPixelType");
		I_ASSIGN(m_useInputCalibrationAttrPtr, "UseInputCalibration", "If enabled, the calibration of the input image will be overtaken for the output", true, true);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

protected:
	BackgroundFillMode GetBackgroundMode(const iprm::IParamsSet* paramsPtr) const;
	iimg::IBitmap::PixelFormat GetOutputPixelFormat(const iprm::IParamsSet* paramsPtr) const;
	AoiMode GetAoiMode(const iprm::IParamsSet* paramsPtr) const;

	// abstract methods
	virtual bool ProcessImage(
				const iprm::IParamsSet* paramsPtr, 
				const iimg::IBitmap& inputImage,
				iimg::IBitmap& outputImage) const = 0;

	I_REF(i2d::IObject2d, m_defaultAoiCompPtr);
	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);

private:
	I_ATTR(int, m_aoiModeAttrPtr);
	I_ATTR(int, m_backgroundModeAttrPtr);
	I_ATTR(int, m_outputPixelTypeAttrPtr);
	I_ATTR(QByteArray, m_outputPixelTypeIdAttrPtr);
	I_ATTR(bool, m_useInputCalibrationAttrPtr);
	I_REF(iprm::ISelectionParam, m_defaultOutputPixelTypeParamCompPtr);
};


} // namespace iipr


#endif // !iipr_CImageProcessorCompBase_included

