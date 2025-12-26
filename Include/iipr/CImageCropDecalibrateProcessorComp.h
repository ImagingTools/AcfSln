#ifndef iipr_CImageCropDecalibrateProcessorComp_included
#define iipr_CImageCropDecalibrateProcessorComp_included


// Qt includes
#include <QtCore/QByteArray>

// ACF includes
#include <iimg/IBitmap.h>
#include <i2d/CRectangle.h>
#include <iprm/COptionsManager.h>

// ACF-Solutions includes
#include <icalib/CAffineCalibration2d.h>
#include <iproc/TSyncProcessorCompBase.h>


namespace iipr
{


/**
	Crop the image region and rectificate (de-calibrate) it.
*/
class CImageCropDecalibrateProcessorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;

	I_BEGIN_COMPONENT(CImageCropDecalibrateProcessorComp);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_REGISTER_SUBELEMENT(InterpolationModes);
		I_REGISTER_SUBELEMENT_INTERFACE(InterpolationModes, iprm::IOptionsList, ExtractInterpolationModes);
		I_REGISTER_SUBELEMENT_INTERFACE(InterpolationModes, istd::IChangeable, ExtractInterpolationModes);
		I_REGISTER_SUBELEMENT(OrientationModes);
		I_REGISTER_SUBELEMENT_INTERFACE(OrientationModes, iprm::IOptionsList, ExtractOrientationModes);
		I_REGISTER_SUBELEMENT_INTERFACE(OrientationModes, istd::IChangeable, ExtractOrientationModes);
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of area of interest in parameter set (i2d::CRectangle)", false, "AoiParams");
		I_ASSIGN(m_defaultAoiCompPtr, "DefaultAoi", "Area of interest used if not specified in parameters", false, "DefaultAoi");
		I_ASSIGN(m_interpolationModeParamIdAttrPtr, "InterpolationModeParamId", "ID of interpolation mode selection in parameter set (iprm::ISelectionParam)", false, "InterpolationMode");
		I_ASSIGN(m_defaultInterpolationModeCompPtr, "DefaultInterpolationMode", "Interpolation mode selection  used if not specified in parameters", false, "DefaultInterpolationMode");
		I_ASSIGN(m_orientationModeParamIdAttrPtr, "OrientationModeParamId", "ID of orientation mode selection in parameter set (iprm::ISelectionParam)", false, "OrientationMode");
		I_ASSIGN(m_defaultOrientationModeCompPtr, "DefaultOrientationMode", "Orientation mode selection  used if not specified in parameters", false, "DefaultOrientationMode");
		I_ASSIGN(m_cellSizeAttrPtr, "CellSize", "Size of single cell (inside of cell linear interpolation will be used)", true, 8);
	I_END_COMPONENT;
	
	enum OrientationMode
	{
		OM_SIMPLE,
		OM_VISUAL,
		OM_NO_REFLEXION
	};

	enum InterpolationMode
	{
		IM_SIMPLE,
		IM_LINEAR
	};

	CImageCropDecalibrateProcessorComp();

	static bool CropImage(
				const i2d::CRectangle& sourceAoi,
				int cellSize,
				const iimg::IBitmap& inputBitmap,
				iimg::IBitmap& outputBitmap,
				istd::CIndex2d* outputImageSizePtr = nullptr,
				int interpolationMode = IM_LINEAR,
				int orientationMode = OM_SIMPLE,
				ilog::IMessageConsumer* resultConsumerPtr = nullptr);

	static bool CalcCalibration(
				const i2d::CRectangle& sourceAoi,
				icalib::CAffineCalibration2d& outputCalib,
				istd::CIndex2d* outputImageSizePtr = nullptr,
				int orientationMode = OM_SIMPLE);

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

protected:
	static bool CalcOutputImageSize(
				const i2d::CRectangle& sourceAoi,
				istd::CIndex2d& result);

private:
	template <class InterfaceType>
	static InterfaceType* ExtractInterpolationModes(CImageCropDecalibrateProcessorComp& component)
	{
		return &component.m_interpolationModes;
	}

	template <class InterfaceType>
	static InterfaceType* ExtractOrientationModes(CImageCropDecalibrateProcessorComp& component)
	{
		return &component.m_orientationModes;
	}

	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_REF(i2d::CRectangle, m_defaultAoiCompPtr);
	I_ATTR(QByteArray, m_interpolationModeParamIdAttrPtr);
	I_REF(iprm::ISelectionParam, m_defaultInterpolationModeCompPtr);
	I_ATTR(QByteArray, m_orientationModeParamIdAttrPtr);
	I_REF(iprm::ISelectionParam, m_defaultOrientationModeCompPtr);
	I_ATTR(int, m_cellSizeAttrPtr);

	iprm::COptionsManager m_interpolationModes;
	iprm::COptionsManager m_orientationModes;
};


} // namespace iipr


#endif // !iipr_CImageCropDecalibrateProcessorComp_included


