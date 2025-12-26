#include <iipr/CImageCopyProcessorComp.h>


// ACF includes
#include <imath/CFixedPointManip.h>
#include <iprm/TParamsPtr.h>
#include <iimg/CPixelFormatList.h>

// ACF includes
#include <iipr/CPixelManip.h>


namespace iipr
{


/**
	Template copy funtion.
*/
template <	typename InputPixelType,
			typename OutputPixelType,
			typename WorkingType>
bool DoCopyTemplate(
			const iimg::IBitmap& inputImage,
			const iimg::CScanlineMask& resultMask,
			iimg::IBitmap& outputImage)
{
	istd::CIndex2d imageSize = outputImage.GetImageSize();

	if (imageSize.IsSizeEmpty()){
		return false;
	}

	istd::CIntRange lineRange(0, imageSize.GetX());

	for (int y = 0; y < imageSize.GetY(); ++y){
		const istd::CIntRanges* outputRangesPtr = resultMask.GetPixelRanges(y);
		if (outputRangesPtr != nullptr){
			const InputPixelType* inputLinePtr = static_cast<const InputPixelType*>(inputImage.GetLinePtr(y));

			OutputPixelType* outputLinePtr = static_cast<OutputPixelType*>(outputImage.GetLinePtr(y));

			istd::CIntRanges::RangeList rangeList;
			outputRangesPtr->GetAsList(lineRange, rangeList);
			for (		istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter){
				const istd::CIntRange& rangeH = *iter;

				for (int x = rangeH.GetMinValue(); x < rangeH.GetMaxValue(); ++x){
					WorkingType pixelValue = inputLinePtr[x];

					outputLinePtr[x] = pixelValue;
				}
			}
		}
	}

	return true;
}



/**
	Template copy function with conversion.
*/
template <	typename InputPixelType,
	typename OutputPixelType>
bool DoCopyTemplate(
	const iimg::IBitmap& inputImage,
	const iimg::CScanlineMask& resultMask,
	iimg::IBitmap& outputImage,
	std::function< OutputPixelType(const InputPixelType&)> f)
{
	istd::CIndex2d imageSize = outputImage.GetImageSize();

	if (imageSize.IsSizeEmpty()) {
		return false;
	}

	istd::CIntRange lineRange(0, imageSize.GetX());

	for (int y = 0; y < imageSize.GetY(); ++y) {
		const istd::CIntRanges* outputRangesPtr = resultMask.GetPixelRanges(y);
		if (outputRangesPtr != nullptr) {
			const InputPixelType* inputLinePtr = static_cast<const InputPixelType*>(inputImage.GetLinePtr(y));

			OutputPixelType* outputLinePtr = static_cast<OutputPixelType*>(outputImage.GetLinePtr(y));

			istd::CIntRanges::RangeList rangeList;
			outputRangesPtr->GetAsList(lineRange, rangeList);
			for (istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
				iter != rangeList.constEnd();
				++iter) {
				const istd::CIntRange& rangeH = *iter;

				for (int x = rangeH.GetMinValue(); x < rangeH.GetMaxValue(); ++x) {
					auto pixelValue = inputLinePtr[x];

					outputLinePtr[x] = f(pixelValue);
				}
			}
		}
	}

	return true;
}


/**
	Template copy function (same pixel type).
*/
template <typename PixelType>
bool DoFastCopyTemplate(
			const iimg::IBitmap& inputImage,
			const iimg::CScanlineMask& resultMask,
			iimg::IBitmap& outputImage)
{
	istd::CIndex2d imageSize = outputImage.GetImageSize();

	if (imageSize.IsSizeEmpty()){
		return false;
	}

	istd::CIntRange lineRange(0, imageSize.GetX());

	for (int y = 0; y < imageSize.GetY(); ++y){
		const istd::CIntRanges* outputRangesPtr = resultMask.GetPixelRanges(y);
		if (outputRangesPtr != nullptr){
			const PixelType* inputLinePtr = static_cast<const PixelType*>(inputImage.GetLinePtr(y));

			PixelType* outputLinePtr = static_cast<PixelType*>(outputImage.GetLinePtr(y));

			istd::CIntRanges::RangeList rangeList;
			outputRangesPtr->GetAsList(lineRange, rangeList);
			for (		istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter){
				const istd::CIntRange& rangeH = *iter;

				memcpy(&outputLinePtr[rangeH.GetMinValue()], &inputLinePtr[rangeH.GetMinValue()], rangeH.GetLength() * sizeof(PixelType));
			}
		}
	}

	return true;
}


// public static methods

bool CImageCopyProcessorComp::DoImageCopy(
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

	istd::CIndex2d imageSizeIn = inputImage.GetImageSize();
	if (!outputImage.CreateBitmap(outputPixelFormat, imageSizeIn)){
		if (loggerPtr != nullptr){
			loggerPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, QObject::tr("Could not create output image"), QObject::tr("ImageCopyProcessor"));
		}

		return false;
	}

	switch (inputPixelFormat){
	case iimg::IBitmap::PF_GRAY:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoFastCopyTemplate<quint8>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_RGB:
		case iimg::IBitmap::PF_RGBA:
			return DoCopyTemplate<quint8, iipr::CPixelManip::Rgba, iipr::CPixelManip::RgbCropAccum32<qint32, 16, true, true> >(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY16:
			return DoCopyTemplate<quint8, quint16, iipr::CPixelManip::GrayCropAccum32<qint32, 8, true, false> >(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY32:
			return DoCopyTemplate<quint8, quint32, iipr::CPixelManip::GrayCropAccum32<qint64, 16, true, false> >(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT32:
			return DoCopyTemplate<quint8, float, double>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoCopyTemplate<quint8, double, double>(inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_RGB:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoCopyTemplate<iipr::CPixelManip::Rgba, quint8, iipr::CPixelManip::RgbCropAccum32<qint32, 22, true, true> >(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_RGB:
		case iimg::IBitmap::PF_RGBA:
			return DoFastCopyTemplate<iipr::CPixelManip::Rgba>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY16:
			return DoCopyTemplate<iipr::CPixelManip::Rgba, quint16, iipr::CPixelManip::RgbCropAccum32<qint32, 14, true, false> >(inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_RGBA:
		if (outputPixelFormat == iimg::IBitmap::PF_RGBA){
			return DoFastCopyTemplate<iipr::CPixelManip::Rgba>(inputImage, resultMask, outputImage);
		}
		break;

	case iimg::IBitmap::PF_GRAY16:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoCopyTemplate<quint16, quint8, iipr::CPixelManip::GrayCropAccum32<qint32, 8, true, true> >(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY16:
			return DoFastCopyTemplate<quint16>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY32:
			return DoCopyTemplate<quint16, quint32, iipr::CPixelManip::GrayCropAccum32<qint64, 16, true, false> >(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT32:
			return DoCopyTemplate<quint16, float, double>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoCopyTemplate<quint16, double, double>(inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_GRAY32:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY32:
			return DoFastCopyTemplate<quint32>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT32:
			return DoCopyTemplate<quint32, float, double>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoCopyTemplate<quint32, double, double>(inputImage, resultMask, outputImage);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_FLOAT32:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_FLOAT32:
			return DoFastCopyTemplate<float>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoCopyTemplate<float, double, double>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_GRAY:
		{
			float x_min = std::numeric_limits<float>::max();
			float x_max = std::numeric_limits<float>::min();

			istd::CIndex2d imageSizeOut = outputImage.GetImageSize();
			if (imageSizeOut.IsSizeEmpty()) {
				return true;
			}

			istd::CIntRange lineRange(0, imageSizeOut.GetX());

			for (int y = 0; y < imageSizeOut.GetY(); ++y) {
				const istd::CIntRanges* outputRangesPtr = resultMask.GetPixelRanges(y);
				if (outputRangesPtr != nullptr) {
					auto inputLinePtr = static_cast<const float*>(inputImage.GetLinePtr(y));
					//auto outputLinePtr = static_cast<quint8*>(outputImage.GetLinePtr(y));

					istd::CIntRanges::RangeList rangeList;
					outputRangesPtr->GetAsList(lineRange, rangeList);
					for (istd::CIntRanges::RangeList::ConstIterator iter = rangeList.constBegin();
						iter != rangeList.constEnd();
						++iter) {
						const istd::CIntRange& rangeH = *iter;

						for (int x = rangeH.GetMinValue(); x < rangeH.GetMaxValue(); ++x) {
							auto pixelValue = inputLinePtr[x];

							if (!std::isnan(pixelValue)) {
								if (pixelValue > x_max) x_max = pixelValue;
								else if (pixelValue < x_min) x_min = pixelValue;
							}
						}
					}
				}
			}

			float d = x_max - x_min;
			if (d == 0) d = 1;

			return DoCopyTemplate<float, quint8>(inputImage, resultMask, outputImage, [&](float x)
			{
				return std::isnan(x) ? 255 : quint8((x - x_min) / d * 255.0);
			});
		}

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_FLOAT64:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_FLOAT32:
			return DoCopyTemplate<double, float, double>(inputImage, resultMask, outputImage);

		case iimg::IBitmap::PF_FLOAT64:
			return DoFastCopyTemplate<double>(inputImage, resultMask, outputImage);

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
						QObject::tr("ImageCopyProcessor"));
		}
		return false;
	}

	if (loggerPtr != nullptr){
		const iimg::CPixelFormatList& formatList = iimg::CPixelFormatList::GetInstance();
		loggerPtr->SendLogMessage(
					istd::IInformationProvider::IC_ERROR,
					0,
					QObject::tr("Cannot produce output image in format '%1' from input format '%2'").arg(formatList.GetOptionName(outputPixelFormat)).arg(formatList.GetOptionName(inputPixelFormat)),
					QObject::tr("ImageCopyProcessor"));
	}

	return false;
}


// protected methods

// reimplemented (iipr::CImageProcessorCompBase)

bool CImageCopyProcessorComp::ProcessImage(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& inputImage,
			iimg::IBitmap& outputImage) const
{
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

	return DoImageCopy(
				GetOutputPixelFormat(paramsPtr),
				inputImage,
				resultMask,
				outputImage,
				const_cast<CImageCopyProcessorComp*>(this));
}


} // namespace iipr

