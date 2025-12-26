#include <iipr/CFastGaussianProcessorComp.h>


// Qt includes
#include <QtCore/QVector>

// ACF includes
#include <iimg/IBitmap.h>
#include <iimg/CScanlineMask.h>


namespace iipr
{


struct KernelElement
{
	int offset;
	int shift;
};


template <	typename PixelType,
			typename WorkingType,
			int PixelSize,
			int ChannelsCount,
			int AlphaIndex,
			int ValueShift>
bool DoConvolution(
			const iimg::IBitmap& inputBitmap,
			KernelElement fastAccessElements[9],
			iimg::IBitmap& outputBitmap)
{
	istd::CIndex2d imageSize = inputBitmap.GetImageSize();
	if (imageSize.IsSizeEmpty()){
		return false;
	}

	Q_ASSERT(imageSize == outputBitmap.GetImageSize());
	QVector<const quint8*> inputLines;
	QVector<quint8*> outputLines;

	int imageHeight = imageSize.GetY();

	for (int y = 1; y < imageHeight - 1; ++y){
		inputLines.push_back(static_cast<const quint8*>(inputBitmap.GetLinePtr(y)));
		outputLines.push_back(static_cast<quint8*>(outputBitmap.GetLinePtr(y)));
	}

	int imageWidth = imageSize.GetX();

	#pragma omp parallel for

	for (int y = 0; y < inputLines.count(); ++y){
		const quint8* inputPtr = inputLines[y] + PixelSize;
		quint8* outputPtr = outputLines[y] + PixelSize;

		for (int x = 1; x < imageWidth - 1; ++x){
			WorkingType sums[ChannelsCount] = {0};
			WorkingType alphaSum = 0;
			for (int i = 0; i < 9; ++i){
				const KernelElement& kernelElement = fastAccessElements[i];

				const PixelType* inputPixelPtr = (const PixelType*)(inputPtr + kernelElement.offset);
				for (int channelIndex = 0; channelIndex < ChannelsCount; ++channelIndex){
					WorkingType inputValue = inputPixelPtr[channelIndex];

					sums[channelIndex] += (inputValue << kernelElement.shift);
				}

				if (AlphaIndex >= 0){
					WorkingType inputValue = inputPixelPtr[AlphaIndex];

					alphaSum += inputValue;
				}
			}
			
			for (int channelIndex = 0; channelIndex < ChannelsCount; ++channelIndex){
				WorkingType outputValue = (sums[channelIndex] >> ValueShift);

				((PixelType*)outputPtr)[channelIndex] = PixelType(outputValue);
			}

			if (AlphaIndex >= 0){
				WorkingType outputValue = alphaSum >> ValueShift;

				((PixelType*)outputPtr)[AlphaIndex] = PixelType(outputValue);
			}

			inputPtr += PixelSize;
			outputPtr += PixelSize;
		}
	}

	return true;
}


bool DoGrayConvolution(
			const iimg::IBitmap& inputBitmap,
			const i2d::IObject2d* aoiPtr,
			iimg::IBitmap& outputBitmap)
{
	istd::CIndex2d imageSize = inputBitmap.GetImageSize();
	if (imageSize.IsSizeEmpty()){
		return false;
	}
	Q_ASSERT(imageSize == outputBitmap.GetImageSize());

	i2d::CRect regionRect(imageSize);
	iimg::CScanlineMask bitmapRegion;
	if (aoiPtr != nullptr){
		i2d::CRect clipArea(imageSize);
		if (!bitmapRegion.CreateFromGeometry(*aoiPtr, &clipArea)){
			return false;
		}
		
		regionRect = bitmapRegion.GetBoundingBox();
	}

	int regionLeft = qMax(regionRect.GetLeft(), 1);
	int regionRight = qMin(regionRect.GetRight(), imageSize.GetX() - 1);
	int regionTop = qMax(regionRect.GetTop(), 1);
	int regionBottom = qMin(regionRect.GetBottom(), imageSize.GetY() - 1);

	if ((regionRight <= regionLeft) || (regionBottom <= regionTop)){
		outputBitmap.ResetImage();

		return true;
	}

	int linesDifference = inputBitmap.GetLinesDifference();
	int pixelsDifference = inputBitmap.GetPixelsDifference();

//	#pragma omp parallel for

	for (int y = regionTop; y < regionBottom; ++y){
		const quint8* inputPtr = ((const quint8*)inputBitmap.GetLinePtr(y)) + regionLeft * pixelsDifference;
		quint8* outputPtr = ((quint8*)outputBitmap.GetLinePtr(y)) + regionLeft * pixelsDifference;

		const quint8* inputBuffer1 = (const quint8*)(inputPtr - pixelsDifference - linesDifference);
		const quint8* inputBuffer2 = (const quint8*)(inputPtr - linesDifference);
		const quint8* inputBuffer3 = (const quint8*)(inputPtr + pixelsDifference - linesDifference);

		const quint8* inputBuffer4 = (const quint8*)(inputPtr - pixelsDifference);
		const quint8* inputBuffer5 = (const quint8*)(inputPtr);
		const quint8* inputBuffer6 = (const quint8*)(inputPtr  + pixelsDifference);

		const quint8* inputBuffer7 = (const quint8*)(inputPtr - pixelsDifference + linesDifference);
		const quint8* inputBuffer8 = (const quint8*)(inputPtr + linesDifference);
		const quint8* inputBuffer9 = (const quint8*)(inputPtr + pixelsDifference + linesDifference);

		for (int x = regionLeft; x < regionRight; ++x){
			int sums = *inputBuffer1;
			sums += (*inputBuffer2) * 2;
			sums += *inputBuffer3;

			sums += (*inputBuffer4) * 2;
			sums += (*inputBuffer5) * 4;
			sums += (*inputBuffer6) * 2;

			sums += *inputBuffer7;
			sums += (*inputBuffer8) * 2;
			sums += *inputBuffer9;

			*outputPtr = quint8(sums >> 4);

			++inputBuffer1, ++inputBuffer2, ++inputBuffer3, ++inputBuffer4, ++inputBuffer5, ++inputBuffer6, ++inputBuffer7, ++inputBuffer8, ++inputBuffer9;
			outputPtr++;
		}
	}

	return true;
}


template <	typename PixelType,
			typename WorkingType>
bool DoFloatConvolution(
			const iimg::IBitmap& inputBitmap,
			const i2d::IObject2d* aoiPtr,
			iimg::IBitmap& outputBitmap)
{
	istd::CIndex2d imageSize = inputBitmap.GetImageSize();
	if (imageSize.IsSizeEmpty()){
		return false;
	}
	Q_ASSERT(imageSize == outputBitmap.GetImageSize());

	i2d::CRect regionRect(imageSize);
	iimg::CScanlineMask bitmapRegion;
	if (aoiPtr != nullptr){
		i2d::CRect clipArea(imageSize);
		if (!bitmapRegion.CreateFromGeometry(*aoiPtr, &clipArea)){
			return false;
		}
		
		regionRect = bitmapRegion.GetBoundingBox();
	}

	int regionLeft = qMax(regionRect.GetLeft(), 1);
	int regionRight = qMin(regionRect.GetRight(), imageSize.GetX() - 1);
	int regionTop = qMax(regionRect.GetTop(), 1);
	int regionBottom = qMin(regionRect.GetBottom(), imageSize.GetY() - 1);

	if ((regionRight <= regionLeft) || (regionBottom <= regionTop)){
		outputBitmap.ResetImage();

		return true;
	}

	const PixelType* inputPrevLinePtr = (const PixelType*)(inputBitmap.GetLinePtr(regionTop - 1));
	const PixelType* inputLinePtr = (const PixelType*)(inputBitmap.GetLinePtr(regionTop));

#pragma omp parallel for

	for (int y = regionTop; y < regionBottom; ++y){
		const PixelType* inputNextLinePtr = (const PixelType*)(inputBitmap.GetLinePtr(y + 1));

		PixelType* outputPtr = ((PixelType*)outputBitmap.GetLinePtr(y));

		for (int x = regionLeft; x < regionRight; ++x){
			WorkingType valueSum = inputPrevLinePtr[x - 1];
			valueSum += inputPrevLinePtr[x] * 2;
			valueSum += inputPrevLinePtr[x + 1];

			valueSum += inputLinePtr[x - 1] * 2;
			valueSum += inputLinePtr[x] * 4;
			valueSum += inputLinePtr[x + 1] * 2;

			valueSum += inputNextLinePtr[x - 1];
			valueSum += inputNextLinePtr[x] * 2;
			valueSum += inputNextLinePtr[x + 1];

			outputPtr[x] = PixelType(valueSum / 16);
		}
	}

	return true;
}


// reimplemented (iipr::CImageProcessorCompBase)

bool CFastGaussianProcessorComp::ProcessImageRegion(
				const iimg::IBitmap& inputBitmap,
				const iprm::IParamsSet* /*paramsPtr*/,
				const i2d::IObject2d* aoiPtr,
				istd::IChangeable* outputPtr) const
{
	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == nullptr){
		SendCriticalMessage(0, "Expected interface for the output object is not supported");

		return false;
	}

	istd::CIndex2d inputBitmapSize = inputBitmap.GetImageSize();
	if (inputBitmapSize != outputBitmapPtr->GetImageSize()){
		if (!outputBitmapPtr->CreateBitmap(inputBitmap.GetPixelFormat(), inputBitmapSize)){
			SendErrorMessage(0, "Output image could not be created");		

			return false;
		}
	}

	int linesDifference = inputBitmap.GetLinesDifference();
	int pixelsDifference = inputBitmap.GetPixelsDifference();

	switch (*m_kernelTypeAttrPtr){
		case KT_3x3:
			break;

		default:
			return false;
	}

	int pixelFormat = inputBitmap.GetPixelFormat();

	KernelElement fastAccessElements[9];

	// 1. filter row
	fastAccessElements[0].offset = -pixelsDifference - linesDifference;
	fastAccessElements[0].shift = 0;
	fastAccessElements[1].offset = -linesDifference;
	fastAccessElements[1].shift = 1;
	fastAccessElements[2].offset = pixelsDifference - linesDifference;
	fastAccessElements[2].shift = 0;

	// 2. filter row
	fastAccessElements[3].offset = -pixelsDifference;
	fastAccessElements[3].shift = 1;
	fastAccessElements[4].offset = 0;
	fastAccessElements[4].shift = 2;
	fastAccessElements[5].offset = pixelsDifference;
	fastAccessElements[5].shift = 1;

	// 3. filter row
	fastAccessElements[6].offset = -pixelsDifference + linesDifference;
	fastAccessElements[6].shift = 0;
	fastAccessElements[7].offset = linesDifference;
	fastAccessElements[7].shift = 1;
	fastAccessElements[8].offset = pixelsDifference + linesDifference;
	fastAccessElements[8].shift = 0;

	switch (pixelFormat){
		case iimg::IBitmap::PF_GRAY:
			return DoGrayConvolution(inputBitmap, aoiPtr, *outputBitmapPtr);

		case iimg::IBitmap::PF_RGB:
			return DoConvolution<quint8, qint32, 4, 4, -1, 4>(inputBitmap, fastAccessElements, *outputBitmapPtr);

		case iimg::IBitmap::PF_RGBA:
			return DoConvolution<quint8, qint32, 4, 4, 3, 4>(inputBitmap, fastAccessElements, *outputBitmapPtr);

		case iimg::IBitmap::PF_GRAY16:
			return DoConvolution<quint16, qint32, 1, 1, -1, 4>(inputBitmap, fastAccessElements, *outputBitmapPtr);

		case iimg::IBitmap::PF_GRAY32:
			return DoConvolution<quint32, qint64, 1, 1, -1, 4>(inputBitmap, fastAccessElements, *outputBitmapPtr);

		case iimg::IBitmap::PF_FLOAT32:
			return DoFloatConvolution<float, double>(inputBitmap, aoiPtr, *outputBitmapPtr);

		case iimg::IBitmap::PF_FLOAT64:
			return DoFloatConvolution<double, double>(inputBitmap, aoiPtr, *outputBitmapPtr);

		default:
			SendErrorMessage(0, "Unsupported image format");

			return false;
	}
}


} // namespace iipr


