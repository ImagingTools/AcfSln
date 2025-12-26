#include <iipr/CImageNormalizeProcessorComp.h>


// STL includes
#include <cstring>

// ACF includes
#include <imath/CFixedPointManip.h>
#include <iprm/TParamsPtr.h>
#include <iimg/CPixelFormatList.h>

// ACF includes
#include <iipr/CPixelManip.h>


namespace iipr
{


/**
	Template filter funtion.
*/
template <	typename InputPixelType,
			typename OutputPixelType,
			typename WorkingType>
bool DoAdjustTemplate(
			double contrast,
			double brightness,
			int backgroundMode,
			const iimg::IBitmap& inputImage,
			const iimg::CScanlineMask& resultMask,
			iimg::IBitmap& outputImage)
{
	istd::CIndex2d inputImageSize = inputImage.GetImageSize();
	istd::CIndex2d outputImageSize = inputImageSize;

	if (outputImageSize.IsSizeEmpty()){
		return false;
	}

	WorkingType contrastFactor = WorkingType(contrast);
	WorkingType brightnessFactor = WorkingType(brightness);

	istd::CIntRange lineRange(0, outputImageSize.GetX());

	#pragma omp parallel for

	for (int y = 0; y < outputImageSize.GetY(); ++y){
		OutputPixelType* outputLinePtr = static_cast<OutputPixelType*>(outputImage.GetLinePtr(y));

		int x = 0;

		const istd::CIntRanges* outputRangesPtr = resultMask.GetPixelRanges(y);
		if (outputRangesPtr != nullptr){
			const InputPixelType* inputLinePtr = static_cast<const InputPixelType*>(inputImage.GetLinePtr(y));

			istd::CIntRanges::RangeList rangeList;
			outputRangesPtr->GetAsList(lineRange, rangeList);
			for (		istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter){
				const istd::CIntRange& rangeH = *iter;

				if (rangeH.GetMinValue() > x){
					if (backgroundMode == CImageProcessorCompBase::BFM_RESET){
						std::memset(outputLinePtr + x, 0, (rangeH.GetMinValue() - x) * sizeof(OutputPixelType));
					}
					else if (backgroundMode == CImageProcessorCompBase::BFM_INPUT){
						for (; x < rangeH.GetMinValue(); ++x){
							WorkingType inputValue = inputLinePtr[x];
							outputLinePtr[x] = inputValue;
						}
					}

					x = rangeH.GetMinValue();
				}

				for (; x < rangeH.GetMaxValue(); ++x){
					WorkingType pixelValue = inputLinePtr[x];
					pixelValue *= contrastFactor;
					pixelValue += brightnessFactor;

					outputLinePtr[x] = pixelValue;
				}
			}
		}

		if (outputImageSize.GetX() > x){
			if (backgroundMode == CImageProcessorCompBase::BFM_RESET){
				std::memset(outputLinePtr + x, 0, (outputImageSize.GetX() - x) * sizeof(OutputPixelType));
			}
			else if (backgroundMode == CImageProcessorCompBase::BFM_INPUT){
				const InputPixelType* inputLinePtr = static_cast<const InputPixelType*>(inputImage.GetLinePtr(y));

				for (; x < outputImageSize.GetX(); ++x){
					WorkingType inputValue = inputLinePtr[x];
					outputLinePtr[x] = inputValue;
				}
			}
		}
	}

	return true;
}


// public static methods

bool CImageNormalizeProcessorComp::DoAdjustFilter(
			double contrast,
			double brightness,
			int backgroundMode,
			iimg::IBitmap::PixelFormat outputPixelFormat,
			const iimg::IBitmap& inputImage,
			const iimg::CScanlineMask& resultMask,
			iimg::IBitmap& outputImage,
			istd::ILogger* loggerPtr)
{
	iimg::IBitmap::PixelFormat inputPixelFormat = inputImage.GetPixelFormat();
	if (outputPixelFormat <= iimg::IBitmap::PF_UNKNOWN){
		outputPixelFormat = inputPixelFormat;
	}

	istd::CIndex2d imageSize = inputImage.GetImageSize();
	if (!outputImage.CreateBitmap(
				outputPixelFormat,
				imageSize)){
		if (loggerPtr != nullptr){
			loggerPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, QObject::tr("Could not create output image"), QObject::tr("ImageNormalizeProcessor"));
		}

		return false;
	}

	switch (inputPixelFormat){
	case iimg::IBitmap::PF_GRAY:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			if ((brightness < 0) || (255 * contrast + brightness < 0)){
				if ((brightness > 255) || (255 * contrast + brightness > 255)){
					return DoAdjustTemplate<quint8, quint8, iipr::CPixelManip::GrayCropAccum32<qint32, 16, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);
				}
				else{
					return DoAdjustTemplate<quint8, quint8, iipr::CPixelManip::GrayCropAccum32<qint32, 23, true, false> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);
				}
			}
			else{
				if ((brightness > 255) || (255 * contrast + brightness > 255)){
					return DoAdjustTemplate<quint8, quint8, iipr::CPixelManip::GrayCropAccum32<qint32, 16, false, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);
				}
				else{
					return DoAdjustTemplate<quint8, quint8, iipr::CPixelManip::GrayCropAccum32<quint32, 24, false, false> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);
				}
			}

		case iimg::IBitmap::PF_RGB:
		case iimg::IBitmap::PF_RGBA:
			return DoAdjustTemplate<quint8, iipr::CPixelManip::Rgba, iipr::CPixelManip::RgbCropAccum32<qint32, 16, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

			
		case iimg::IBitmap::PF_RGB24:
			return DoAdjustTemplate<uchar, iipr::CPixelManip::Rgb, iipr::CPixelManip::RgbCropAccum32<uchar, 0, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY16:
			return DoAdjustTemplate<quint8, quint16, iipr::CPixelManip::GrayCropAccum32<qint32, 8, true, false> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY32:
			return DoAdjustTemplate<quint8, quint32, iipr::CPixelManip::GrayCropAccum32<qint64, 16, true, false> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT32:
			return DoAdjustTemplate<quint8, float, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoAdjustTemplate<quint8, double, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_RGB:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoAdjustTemplate<iipr::CPixelManip::Rgba, quint8, iipr::CPixelManip::RgbCropAccum32<qint32, 22, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_RGB:
		case iimg::IBitmap::PF_RGBA:
			return DoAdjustTemplate<iipr::CPixelManip::Rgba, iipr::CPixelManip::Rgba, iipr::CPixelManip::RgbCropAccum32<qint32, 22, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY16:
			return DoAdjustTemplate<iipr::CPixelManip::Rgba, quint16, iipr::CPixelManip::RgbCropAccum32<qint32, 14, true, false> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_RGBA:
		if (outputPixelFormat == iimg::IBitmap::PF_RGBA){
			return DoAdjustTemplate<iipr::CPixelManip::Rgba, iipr::CPixelManip::Rgba, iipr::CPixelManip::RgbaCropAccum32<qint32, 22, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);
		}
		break;

	case iimg::IBitmap::PF_GRAY16:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoAdjustTemplate<quint16, quint8, iipr::CPixelManip::GrayCropAccum32<qint32, 8, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY16:
			return DoAdjustTemplate<quint16, quint16, iipr::CPixelManip::GrayCropAccum32<qint32, 8, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY32:
			return DoAdjustTemplate<quint16, quint32, iipr::CPixelManip::GrayCropAccum32<qint64, 16, true, false> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT32:
			return DoAdjustTemplate<quint16, float, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoAdjustTemplate<quint16, double, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_GRAY32:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY32:
			return DoAdjustTemplate<quint32, quint32, iipr::CPixelManip::GrayCropAccum32<qint64, 16, true, true> >(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT32:
			return DoAdjustTemplate<quint32, float, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoAdjustTemplate<quint32, double, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_FLOAT32:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_FLOAT32:
			return DoAdjustTemplate<float, float, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoAdjustTemplate<float, double, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_FLOAT64:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_FLOAT32:
			return DoAdjustTemplate<double, float, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoAdjustTemplate<double, double, double>(contrast, brightness, backgroundMode, inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	default:
		if (loggerPtr != nullptr){
			loggerPtr->SendLogMessage(
						istd::IInformationProvider::IC_ERROR,
						0,
						QObject::tr("Input image format '%1' not supported").arg(iimg::CPixelFormatList::GetInstance().GetOptionName(inputPixelFormat)),
						QObject::tr("ImageNormalizeProcessor"));
		}
		return false;
	}

	if (loggerPtr != nullptr){
		const iimg::CPixelFormatList& formatList = iimg::CPixelFormatList::GetInstance();
		loggerPtr->SendLogMessage(
					istd::IInformationProvider::IC_ERROR,
					0,
					QObject::tr("Cannot produce output image in format '%1' from input format '%2'").arg(formatList.GetOptionName(outputPixelFormat)).arg(formatList.GetOptionName(inputPixelFormat)),
					QObject::tr("ImageNormalizeProcessor"));
	}

	return false;
}


// protected methods

// reimplemented (iipr::CImageProcessorCompBase)

bool CImageNormalizeProcessorComp::ProcessImage(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& inputImage,
			iimg::IBitmap& outputImage) const
{
	double contrast = 1;
	double brightness = 0;
	iprm::TParamsPtr<imeas::ILinearAdjustParams> adjustParamsPtr(paramsPtr, m_adjustParamsIdAttrPtr, m_defaultAdjustParamsCompPtr, false);
	if (adjustParamsPtr.IsValid()){
		contrast = adjustParamsPtr->GetScaleFactor();
		brightness = adjustParamsPtr->GetOffsetFactor() * 255;
	}

	istd::CIndex2d imageSize = inputImage.GetImageSize();

	i2d::CRect inputClipArea(imageSize);
	iimg::CScanlineMask resultMask;
	iprm::TParamsPtr<i2d::IObject2d> aoiObjectPtr(paramsPtr, m_aoiParamIdAttrPtr, m_defaultAoiCompPtr, false);
	if (aoiObjectPtr.IsValid()){
		resultMask.SetCalibration(inputImage.GetCalibration());

		if (!resultMask.CreateFromGeometry(*aoiObjectPtr.GetPtr(), &inputClipArea)){
			SendErrorMessage(0, QObject::tr("AOI type is not supported"));

			return false;
		}
	}
	else{
		resultMask.CreateFilled(inputClipArea);
	}

	iprm::TParamsPtr<imeas::INumericValue> clippingThresholdParamsPtr(m_defaultClippingThresholdCompPtr.GetPtr());
	if (m_clippingThresholdIdAttrPtr.IsValid()){
		clippingThresholdParamsPtr.Init(paramsPtr, *m_clippingThresholdIdAttrPtr);
	}

	double clippingThreshold = 0.0;
	if (clippingThresholdParamsPtr.IsValid()){
		imath::CVarVector values = clippingThresholdParamsPtr->GetValues();
		if (values.GetElementsCount() > 0){
			clippingThreshold = values[0];
		}
	}

	iimg::IBitmap::PixelFormat pixelFormat = inputImage.GetPixelFormat();

	if (*m_useHistogramAdjustmentCompPtr){
		if (pixelFormat != iimg::IBitmap::PF_GRAY){
			SendErrorMessage(0, "Only 8 bit grayscale images are supported for histogram analyse");

			return false;
		}

		// Calculate histogram of single channel
		qint32 channelHistogram[256] = {0};
		for (int y = 0; y < imageSize.GetY(); ++y){
			const istd::CIntRanges* outputRangesPtr = resultMask.GetPixelRanges(y);
			if (outputRangesPtr != nullptr){
				const quint8* inputLinePtr = static_cast<const quint8*>(inputImage.GetLinePtr(y));

				istd::CIntRanges::RangeList rangeList;
				outputRangesPtr->GetAsList(inputClipArea.GetHorizontalRange(), rangeList);

				for (		istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
							iter != rangeList.constEnd();
							++iter){
					const istd::CIntRange& rangeH = *iter;

					for (int x = 0; x < rangeH.GetMaxValue(); ++x){
						++channelHistogram[inputLinePtr[x]];
					}
				}
			}
		}

		int pixelCount = 0;
		for (int histogramIndex = 0; histogramIndex < 256; histogramIndex++){
			pixelCount += channelHistogram[histogramIndex];
		}

		// Get clipped histogram minimum:
		int histogramMin = -1;
		int histogramMax = -1;
		for (int histogramIndex = 0; histogramIndex < 256; histogramIndex++){
			quint32 binCount = channelHistogram[histogramIndex];

			if ((binCount > 0) && (binCount >= clippingThreshold * pixelCount)){
				if (histogramMin < 0){
					histogramMin = histogramIndex;
				}

				histogramMax = histogramIndex;
			}
		}

		if (histogramMax > histogramMin){
			double histContrast = 255.0 / (histogramMax - histogramMin);

			contrast = contrast * histContrast;
			brightness -= histContrast * histogramMin;
		}
	}

	return DoAdjustFilter(
				contrast,
				brightness,
				GetBackgroundMode(paramsPtr),
				GetOutputPixelFormat(paramsPtr),
				inputImage,
				resultMask,
				outputImage,
				const_cast<CImageNormalizeProcessorComp*>(this));
}


} // namespace iipr

