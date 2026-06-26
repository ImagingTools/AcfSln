// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CAdaptiveImageBinarizeProcessorComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>
#include <iprm/TParamsPtr.h>
#include <iimg/CGeneralBitmap.h>

// ACF-Solutions includes
#include <iipr/CRectImageSmoothProcessorComp.h>


namespace iipr
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CAdaptiveImageBinarizeProcessorComp::DoProcessing(
				const iprm::IParamsSet* /*paramsPtr*/,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	if (inputBitmapPtr == NULL){
		return TS_INVALID;
	}

	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == NULL){
		return TS_INVALID;
	}

	return ConvertImage(*inputBitmapPtr, *outputBitmapPtr) ? TS_OK : TS_INVALID;
}


// private methods

bool CAdaptiveImageBinarizeProcessorComp::ConvertImage(
			const iimg::IBitmap& inputBitmap,
			iimg::IBitmap& outputBitmap) const
{
	if (inputBitmap.IsEmpty()){
		SendWarningMessage(0, "Input bitmap is empty.");

		return false;
	}

	if (inputBitmap.GetPixelFormat() != iimg::IBitmap::PF_GRAY){
		SendWarningMessage(0, "Input bitmap is not grayscale.");

		return false;
	}

	iimg::CGeneralBitmap smoothedBitmap;
	CRectImageSmoothProcessorComp::DoRectFilter(3, 3, iimg::IBitmap::PF_GRAY, inputBitmap, smoothedBitmap, iipr::CRectImageSmoothProcessorComp::BM_STRETCH_KERNEL);

	if (!outputBitmap.CreateBitmap(iimg::IBitmap::PF_GRAY, smoothedBitmap.GetImageSize())){
		return false;
	}

	int imageWidth = smoothedBitmap.GetImageSize().GetX();
	int imageHeight = smoothedBitmap.GetImageSize().GetY();

	double threshold = 0.0;
	int pixelCount = 0;

	int minContrast = 0.05 * 255; // TODO: Use parameterization!

	for (int y = 0; y < imageHeight; ++y){
		quint8* inputImageBufferPtr = (quint8*)inputBitmap.GetLinePtr(y);
		quint8* smoothedImageBufferPtr = (quint8*)smoothedBitmap.GetLinePtr(y);

		for (int x = 0; x < imageWidth; ++x){
			int diff = labs(*inputImageBufferPtr - *smoothedImageBufferPtr);
			if (diff >= minContrast)
			{
				threshold += *inputImageBufferPtr;
				++pixelCount;
			}

			++inputImageBufferPtr, ++smoothedImageBufferPtr;
		}
	}

	if (pixelCount > 0){
		threshold /= double(pixelCount);
	}

	for (int y = 0; y < imageHeight; ++y){
		quint8* outputImageBufferPtr = (quint8*)outputBitmap.GetLinePtr(y);
		quint8* inputImageBufferPtr = (quint8*)inputBitmap.GetLinePtr(y);

		for (int x = 0; x < imageWidth; ++x){
			*outputImageBufferPtr = *inputImageBufferPtr > threshold ? 255 : 0;

			++outputImageBufferPtr, ++inputImageBufferPtr;
		}
	}

	return true;
}


} // namespace iipr


