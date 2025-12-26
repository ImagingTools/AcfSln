#include <iipr/CImageBinarizeProcessorComp.h>


 // ACF includes
#include <istd/CChangeNotifier.h>
#include <iprm/TParamsPtr.h>

#include <ibase/CSize.h>


namespace iipr
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CImageBinarizeProcessorComp::DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* /*progressManagerPtr*/)
{
	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	if (inputBitmapPtr == nullptr){
		return TS_INVALID;
	}

	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == nullptr){
		return TS_INVALID;
	}

	iprm::TParamsPtr<imeas::INumericValue> thresholdParamsPtr(paramsPtr, m_binarizationParamsIdAttrPtr, m_defaultBinarizationThresholdCompPtr);
	if (!thresholdParamsPtr.IsValid()){
		SendErrorMessage(0, "Threshold parameter is not set");

		return TS_INVALID;
	}

	return ConvertImage(*inputBitmapPtr, *thresholdParamsPtr, *outputBitmapPtr) ? TS_OK : TS_INVALID;
}


// private methods

bool CImageBinarizeProcessorComp::ConvertImage(
			const iimg::IBitmap& inputBitmap,
			const imeas::INumericValue& thresholdParams,
			iimg::IBitmap& outputBitmap) const
{
	if (inputBitmap.IsEmpty()){
		SendWarningMessage(0, "Input bitmap is empty.");

		return false;
	}

	if (!outputBitmap.CreateBitmap(inputBitmap.GetPixelFormat(), inputBitmap.GetImageSize())){
		return false;
	}

	int imageWidth = inputBitmap.GetImageSize().GetX();
	int imageHeight = inputBitmap.GetImageSize().GetY();

	imath::CVarVector values = thresholdParams.GetValues();
	if (values.GetElementsCount() < 1){
		SendErrorMessage(0, "Wrong processing parameters");

		return false;
	}

	double threshold = values[0] * 255;

	quint8 objectColor = *m_isInverseModeEnabledAttrPtr ? 0 : 255;
	quint8 backgroundColor = 255 - objectColor;
 
	for (int y = 0; y < imageHeight; ++y){
		quint8* outputImageBufferPtr = (quint8*)outputBitmap.GetLinePtr(y);
		quint8* inputImageBufferPtr = (quint8*)inputBitmap.GetLinePtr(y);

		for (int x = 0; x < imageWidth; ++x){
			*outputImageBufferPtr = *inputImageBufferPtr > threshold ? objectColor : backgroundColor;

			++outputImageBufferPtr, ++inputImageBufferPtr;
		}
	}

	const i2d::ICalibration2d* inputCalibrationPtr = inputBitmap.GetCalibration();
	if (inputCalibrationPtr != nullptr){
		istd::TUniqueInterfacePtr<i2d::ICalibration2d> outputCalibrationPtr;
		outputCalibrationPtr.MoveCastedPtr(inputCalibrationPtr->CloneMe());

		outputBitmap.SetCalibration(outputCalibrationPtr.PopInterfacePtr(), true);
	}

	return true;
}


} // namespace iipr


