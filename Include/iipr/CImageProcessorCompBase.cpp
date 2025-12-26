#include <iipr/CImageProcessorCompBase.h>


// ACF includes
#include <iprm/TParamsPtr.h>
#include <iimg/CGeneralBitmap.h>

// ACF-Solutions includes
#include <iipr/CImageProcessorCompBase.h>


namespace iipr
{


// protected methods

CImageProcessorCompBase::BackgroundFillMode CImageProcessorCompBase::GetBackgroundMode(const iprm::IParamsSet* /*paramsPtr*/) const
{
	return BackgroundFillMode(*m_backgroundModeAttrPtr);
}


iimg::IBitmap::PixelFormat CImageProcessorCompBase::GetOutputPixelFormat(const iprm::IParamsSet* paramsPtr) const
{
	iimg::IBitmap::PixelFormat outputPixelFormat = iimg::IBitmap::PixelFormat(*m_outputPixelTypeAttrPtr);
	iprm::TParamsPtr<iprm::ISelectionParam> outputFormatParamPtr(paramsPtr, m_outputPixelTypeIdAttrPtr, m_defaultOutputPixelTypeParamCompPtr, false);
	if (outputFormatParamPtr.IsValid()){
		outputPixelFormat = iimg::IBitmap::PixelFormat(outputFormatParamPtr->GetSelectedOptionIndex());
	}

	return outputPixelFormat;
}


CImageProcessorCompBase::AoiMode CImageProcessorCompBase::GetAoiMode(const iprm::IParamsSet* /*paramsPtr*/) const
{
	return AoiMode(*m_aoiModeAttrPtr);
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CImageProcessorCompBase::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/)
{
	if (outputPtr == nullptr){
		return TS_OK;
	}

	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);

	if ((inputBitmapPtr == nullptr) || (outputBitmapPtr == nullptr)){
		return TS_INVALID;
	}

	// do image processing:
	if (!ProcessImage(paramsPtr, *inputBitmapPtr, *outputBitmapPtr)){
		return TS_INVALID;
	}

	if (*m_useInputCalibrationAttrPtr){
		const i2d::ICalibration2d* inputCalibrationPtr = inputBitmapPtr->GetCalibration();
		if (inputCalibrationPtr != nullptr){
			istd::TUniqueInterfacePtr<i2d::ICalibration2d> outputCalibrationPtr;
			outputCalibrationPtr.MoveCastedPtr(inputCalibrationPtr->CloneMe());

			outputBitmapPtr->SetCalibration(outputCalibrationPtr.PopInterfacePtr(), true);
		}
	}

	return TS_OK;
}


} // namespace iipr


