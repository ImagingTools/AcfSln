#include <iipr/CImageIntegralProcessorComp.h>


// ACF-Solutions includes
#include <iprm/TParamsPtr.h>
#include <iipr/CPixelManip.h>


namespace iipr
{


// template functions for several image modes

template <	typename InputPixelType,
			typename OutputPixelType,
			typename WorkingType>
bool DoIntegralFilter(
			const iimg::IBitmap& inputImage,
			iimg::IBitmap& outputImage,
			const iimg::CScanlineMask* maskPtr,
			bool setOutput)
{
	istd::CIndex2d imageSize = inputImage.GetImageSize();
	Q_ASSERT(outputImage.GetImageSize() == imageSize);

	QVector<WorkingType> prevLineBuffer(imageSize.GetX(), 0);

	if (maskPtr != nullptr){
		i2d::CRect commonRect = maskPtr->GetBoundingRect().GetIntersection(i2d::CRect(imageSize));

		int y = 0;
		if (setOutput){
			for (; y < commonRect.GetTop(); ++y){
				OutputPixelType* outputPtr = (OutputPixelType*)outputImage.GetLinePtr(y);

				for (int x = 0; x < imageSize.GetX(); ++x){
					outputPtr[x] = prevLineBuffer[x];
				}
			}
		}

		for (y = commonRect.GetTop(); y < commonRect.GetBottom(); ++y){
			const istd::CIntRanges* maskRangesPtr = maskPtr->GetPixelRanges(y);
			if (maskRangesPtr != nullptr){
				OutputPixelType* outputPtr = (OutputPixelType*)outputImage.GetLinePtr(y);

				const InputPixelType* inputLinePtr = (const InputPixelType*)inputImage.GetLinePtr(y);

				WorkingType lineSum = 0;
				int x = 0;

				istd::CIntRanges::RangeList maskRangeList;
				maskRangesPtr->GetAsList(commonRect.GetHorizontalRange(), maskRangeList);
				for (		istd::CIntRanges::RangeList::ConstIterator rangeIter = maskRangeList.constBegin();
							rangeIter != maskRangeList.constEnd();
							++rangeIter){
					const istd::CIntRange& maskRangeH = *rangeIter;

					if (setOutput){
						for (; x < maskRangeH.GetMinValue(); ++x){
							outputPtr[x] = prevLineBuffer[x];
						}
					}

					for (x = maskRangeH.GetMinValue(); x < maskRangeH.GetMaxValue(); ++x){
						lineSum += WorkingType(inputLinePtr[x]);

						WorkingType cummulatedValue = lineSum;
						cummulatedValue  += prevLineBuffer[x];

						prevLineBuffer[x] = cummulatedValue;

						outputPtr[x] = OutputPixelType(cummulatedValue);
					}
				}

				if (setOutput){
					for (; x < imageSize.GetX(); ++x){
						outputPtr[x] = prevLineBuffer[x];
					}
				}
			}
			else if (setOutput){
				OutputPixelType* outputPtr = (OutputPixelType*)outputImage.GetLinePtr(y);

				for (int x = 0; x < imageSize.GetX(); ++x){
					outputPtr[x] = prevLineBuffer[x];
				}
			}
		}

		if (setOutput){
			for (; y < imageSize.GetY(); ++y){
				OutputPixelType* outputPtr = (OutputPixelType*)outputImage.GetLinePtr(y);

				for (int x = 0; x < imageSize.GetX(); ++x){
					outputPtr[x] = prevLineBuffer[x];
				}
			}
		}
	}
	else{
		for (int y = 0; y < imageSize.GetY(); ++y){
			OutputPixelType* outputPtr = (OutputPixelType*)outputImage.GetLinePtr(y);

			const InputPixelType* inputLinePtr = (const InputPixelType*)inputImage.GetLinePtr(y);

			WorkingType lineSum = 0;
			for (int x = 0; x < imageSize.GetX(); ++x){
				lineSum += WorkingType(inputLinePtr[x]);

				WorkingType cummulatedValue = lineSum;
				cummulatedValue  += prevLineBuffer[x];

				prevLineBuffer[x] = cummulatedValue;

				outputPtr[x] = OutputPixelType(cummulatedValue);
			}
		}
	}

	return true;
}


template <typename PixelType>
bool DoIntegralInPlace(iimg::IBitmap& outputImage)
{
	istd::CIndex2d imageSize = outputImage.GetImageSize();
	if (imageSize.GetY() < 1){
		return true;
	}

	PixelType* prevLinePtr = (PixelType*)outputImage.GetLinePtr(0);

	PixelType firstLineSum = 0;
	for (int x = 0; x < imageSize.GetX(); ++x){
		firstLineSum += prevLinePtr[x];

		prevLinePtr[x] = firstLineSum;
	}

	for (int y = 1; y < imageSize.GetY(); ++y){
		PixelType* linePtr = (PixelType*)outputImage.GetLinePtr(y);

		PixelType lineSum = 0;
		for (int x = 0; x < imageSize.GetX(); ++x){
			lineSum += linePtr[x];

			linePtr[x] = lineSum + prevLinePtr[x];
		}
	}

	return true;
}


template <typename PixelType>
bool DoIntegralReference(const iimg::CScanlineMask& mask, iimg::IBitmap& outputImage)
{
	istd::CIndex2d imageSize = outputImage.GetImageSize();

	QVector<PixelType> prevLineBuffer(imageSize.GetX(), 0);

	i2d::CRect commonRect = mask.GetBoundingRect().GetIntersection(i2d::CRect(imageSize));

	int y = 0;
	for (; y < commonRect.GetTop(); ++y){
		PixelType* outputPtr = (PixelType*)outputImage.GetLinePtr(y);

		for (int x = 0; x < imageSize.GetX(); ++x){
			outputPtr[x] = prevLineBuffer[x];
		}
	}

	for (; y < commonRect.GetBottom(); ++y){
		const istd::CIntRanges* maskRangesPtr = mask.GetPixelRanges(y);
		if (maskRangesPtr != nullptr){
			PixelType* outputPtr = (PixelType*)outputImage.GetLinePtr(y);

			PixelType lineSum = 0;
			int x = 0;

			istd::CIntRanges::RangeList maskRangeList;
			maskRangesPtr->GetAsList(commonRect.GetHorizontalRange(), maskRangeList);
			for (		istd::CIntRanges::RangeList::ConstIterator rangeIter = maskRangeList.constBegin();
						rangeIter != maskRangeList.constEnd();
						++rangeIter){
				const istd::CIntRange& maskRangeH = *rangeIter;

				for (; x < maskRangeH.GetMinValue(); ++x){
					outputPtr[x] = prevLineBuffer[x];
				}

				for (; x < maskRangeH.GetMaxValue(); ++x){
					lineSum += 1;

					PixelType cummulatedValue = lineSum;
					cummulatedValue  += prevLineBuffer[x];

					prevLineBuffer[x] = cummulatedValue;

					outputPtr[x] = cummulatedValue;
				}
			}

			for (; x < imageSize.GetX(); ++x){
				outputPtr[x] = prevLineBuffer[x];
			}
		}
		else{
			PixelType* outputPtr = (PixelType*)outputImage.GetLinePtr(y);

			for (int x = 0; x < imageSize.GetX(); ++x){
				outputPtr[x] = prevLineBuffer[x];
			}
		}
	}

	for (; y < imageSize.GetY(); ++y){
		PixelType* outputPtr = (PixelType*)outputImage.GetLinePtr(y);

		for (int x = 0; x < imageSize.GetX(); ++x){
			outputPtr[x] = prevLineBuffer[x];
		}
	}

	return true;
}


bool CImageIntegralProcessorComp::CalculateIntegralImage(
			const iimg::IBitmap& inputImage,
			iimg::IBitmap& outputImage,
			const iimg::CScanlineMask* maskPtr,
			iimg::IBitmap::PixelFormat outputPixelFormat,
			bool fillOutOfMask)
{
	iimg::IBitmap::PixelFormat inputFormat = inputImage.GetPixelFormat();

	if (outputPixelFormat == iimg::IBitmap::PF_UNKNOWN){
		switch (inputFormat){
		case iimg::IBitmap::PF_GRAY:
		case iimg::IBitmap::PF_GRAY16:
		case iimg::IBitmap::PF_GRAY32:
			outputPixelFormat = iimg::IBitmap::PF_GRAY32;
			break;

		case iimg::IBitmap::PF_RGB:
		case iimg::IBitmap::PF_RGBA:
			outputPixelFormat = iimg::IBitmap::PF_GRAY32;

		case iimg::IBitmap::PF_FLOAT32:
			outputPixelFormat = iimg::IBitmap::PF_FLOAT32;
			break;

		case iimg::IBitmap::PF_FLOAT64:
			outputPixelFormat = iimg::IBitmap::PF_FLOAT64;
			break;

		default:
			return false;
		}
	}

	if (!outputImage.CreateBitmap(outputPixelFormat, inputImage.GetImageSize())){
		return false;
	}

	switch (inputFormat){
	case iimg::IBitmap::PF_GRAY:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoIntegralFilter<quint8, quint8, quint32>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_GRAY16:
			return DoIntegralFilter<quint8, quint16, quint32>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_GRAY32:
			return DoIntegralFilter<quint8, quint32, quint32>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT32:
			return DoIntegralFilter<quint8, float, float>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT64:
			return DoIntegralFilter<quint8, double, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_RGB:
	case iimg::IBitmap::PF_RGBA:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoIntegralFilter<iipr::CPixelManip::Rgba, quint8, iipr::CPixelManip::RgbCropAccum32<quint32, 0, false, true> >(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_RGB:
			return DoIntegralFilter<iipr::CPixelManip::Rgba, quint8, iipr::CPixelManip::RgbCropAccum32<quint32, 0, false, true> >(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_GRAY16:
			return DoIntegralFilter<iipr::CPixelManip::Rgba, quint16, iipr::CPixelManip::RgbCropAccum32<quint32, 0, false, true> >(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_GRAY32:
			return DoIntegralFilter<iipr::CPixelManip::Rgba, quint32, iipr::CPixelManip::RgbCropAccum32<quint32, 0, false, false> >(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT32:
			return DoIntegralFilter<iipr::CPixelManip::Rgba, float, iipr::CPixelManip::RgbCropAccum32<quint32, 0, false, false> >(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT64:
			return DoIntegralFilter<iipr::CPixelManip::Rgba, double, iipr::CPixelManip::RgbCropAccum32<quint32, 0, false, false> >(inputImage, outputImage, maskPtr, fillOutOfMask);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_GRAY16:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY16:
			return DoIntegralFilter<quint16, quint16, quint32>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_GRAY32:
			return DoIntegralFilter<quint16, quint32, quint32>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT32:
			return DoIntegralFilter<quint16, float, float>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT64:
			return DoIntegralFilter<quint16, double, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_GRAY32:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY32:
			return DoIntegralFilter<quint32, quint32, quint64>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT32:
			return DoIntegralFilter<quint32, float, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT64:
			return DoIntegralFilter<quint32, double, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_FLOAT32:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_FLOAT32:
			return DoIntegralFilter<float, float, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT64:
			return DoIntegralFilter<float, double, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		default:
			break;
		}
		break;

	case iimg::IBitmap::PF_FLOAT64:
		switch (outputPixelFormat){
		case iimg::IBitmap::PF_FLOAT32:
			return DoIntegralFilter<double, float, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		case iimg::IBitmap::PF_FLOAT64:
			return DoIntegralFilter<double, double, double>(inputImage, outputImage, maskPtr, fillOutOfMask);

		default:
			break;
		}
		break;

	default:
		break;
	}

	return false;
}


bool CImageIntegralProcessorComp::MakeIntegralImage(iimg::IBitmap& outputImage)
{
	switch (outputImage.GetPixelFormat()){
	case iimg::IBitmap::PF_GRAY16:
		return DoIntegralInPlace<quint16>(outputImage);

	case iimg::IBitmap::PF_GRAY32:
		return DoIntegralInPlace<quint32>(outputImage);

	case iimg::IBitmap::PF_FLOAT32:
		return DoIntegralInPlace<float>(outputImage);

	case iimg::IBitmap::PF_FLOAT64:
		return DoIntegralInPlace<double>(outputImage);

	default:
		return false;
	}
}


bool CImageIntegralProcessorComp::CalculateIntegralReference(
			const iimg::CScanlineMask& mask,
			iimg::IBitmap& outputImage)
{
	switch (outputImage.GetPixelFormat()){
	case iimg::IBitmap::PF_GRAY16:
		return DoIntegralReference<quint16>(mask, outputImage);

	case iimg::IBitmap::PF_GRAY32:
		return DoIntegralReference<quint32>(mask, outputImage);

	case iimg::IBitmap::PF_FLOAT32:
		return DoIntegralReference<float>(mask, outputImage);

	case iimg::IBitmap::PF_FLOAT64:
		return DoIntegralReference<double>(mask, outputImage);

	default:
		return false;
	}
}


// protected methods

// reimplemented (iipr::CImageProcessorCompBase)

bool CImageIntegralProcessorComp::ProcessImage(
			const iprm::IParamsSet* paramsPtr, 
			const iimg::IBitmap& inputImage,
			iimg::IBitmap& outputImage) const
{
	iprm::TParamsPtr<i2d::IObject2d> aoiObjectPtr(paramsPtr, m_aoiParamIdAttrPtr, m_defaultAoiCompPtr, false);
	if (aoiObjectPtr.IsValid()){
		iimg::CScanlineMask resultMask;
		resultMask.SetCalibration(inputImage.GetCalibration());

		i2d::CRect cliArea(inputImage.GetImageSize());

		if (!resultMask.CreateFromGeometry(*aoiObjectPtr.GetPtr(), &cliArea)){
			SendErrorMessage(0, QObject::tr("AOI type is not supported"));

			return false;
		}

		return CalculateIntegralImage(inputImage, outputImage, &resultMask);
	}
	else{
		return CalculateIntegralImage(inputImage, outputImage);
	}

}


} // namespace iipr


