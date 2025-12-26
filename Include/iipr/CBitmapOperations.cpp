#include <iipr/CBitmapOperations.h>


// Qt includes
#include <QtCore/QCoreApplication>

// STL includes
#include <cstring>

// ACF includes
#include <ilog/CMessage.h>
#include <iimg/CScanlineMask.h>
#include <iimg/CPixelFormatList.h>


namespace iipr
{


// template functions

template <	typename FirstPixelType,
			typename SecondPixelType,
			typename OutputPixelType,
			typename WorkingType>
void CalculateDifferenceBitmap(
			WorkingType offset,
			WorkingType maxValue,
			const iimg::IBitmap& firstInputBitmap,
			const iimg::IBitmap& secondInputBitmap,
			iimg::IBitmap& result)
{
	istd::CIndex2d outputImageSize = result.GetImageSize();

	int componentsCount = firstInputBitmap.GetComponentsCount();

	for (int componentIndex = 0; componentIndex < componentsCount; componentIndex++){
		for (int y = 0; y < outputImageSize.GetY(); ++y){
			const FirstPixelType* firstLinePtr = (const FirstPixelType*)firstInputBitmap.GetLinePtr(y);
			const SecondPixelType* secondLinePtr = (const SecondPixelType*)secondInputBitmap.GetLinePtr(y);
			OutputPixelType* outputLinePtr = (OutputPixelType*)result.GetLinePtr(y);

			if (offset > 0){
				for (int x = 0; x < outputImageSize.GetX(); ++x){
					int pixelComponentIndex = x * componentsCount + componentIndex;

					WorkingType firstValue = firstLinePtr[pixelComponentIndex];
					WorkingType secondValue = secondLinePtr[pixelComponentIndex];

					outputLinePtr[pixelComponentIndex] = qMin(qAbs(offset + firstValue - secondValue), maxValue);
				}
			}
			else{
				for (int x = 0; x < outputImageSize.GetX(); ++x){
					int pixelComponentIndex = x * componentsCount + componentIndex;

					WorkingType firstValue = firstLinePtr[pixelComponentIndex];
					WorkingType secondValue = secondLinePtr[pixelComponentIndex];

					outputLinePtr[pixelComponentIndex] = qAbs(offset + firstValue - secondValue);
				}
			}
		}
	}
}


// public static methods

bool CBitmapOperations::ReduceBitmapToRegion(
			const iimg::IBitmap& inputBitmap,
			const i2d::IObject2d& region,
			iimg::IBitmap& outputBitmap)
{
	istd::CIndex2d inputBitmapSize = inputBitmap.GetImageSize();

	iimg::CScanlineMask bitmapRegion;
	i2d::CRect clipArea(inputBitmapSize);
	if (!bitmapRegion.CreateFromGeometry(region, &clipArea)){
		return false;
	}

	i2d::CRect regionRect = bitmapRegion.GetBoundingBox();

	int regionLeft = qMax(regionRect.GetLeft(), 0);
	int regionRight = qMin(regionRect.GetRight(), inputBitmapSize.GetX());
	int regionTop = qMax(regionRect.GetTop(), 0);
	int regionBottom = qMin(regionRect.GetBottom(), inputBitmapSize.GetY());

	if ((regionRight <= regionLeft) || (regionBottom <= regionTop)){
		outputBitmap.ResetImage();

		return true;
	}

	ibase::CSize outputBitmapSize(
				int(regionRight - regionLeft),
				int(regionBottom - regionTop));
	if (!outputBitmap.CreateBitmap(inputBitmap.GetPixelFormat(), outputBitmapSize)){
		return false;
	}

	int outputBitmapLineSize = outputBitmap.GetLineBytesCount();
	quint8* outputImageBufferPtr = (quint8*)outputBitmap.GetLinePtr(0);
	std::memset(outputImageBufferPtr, 0, outputBitmapLineSize * outputBitmap.GetImageSize().GetY()); 

	int pixelBits = outputBitmap.GetPixelBitsCount();
	int pixelBytes = pixelBits / 8;

	if (pixelBytes * 8 != pixelBits){
		return false;
	}
	
	for (int y = regionTop; y < regionBottom; y++){
		const quint8* inputLinePtr = (quint8*)inputBitmap.GetLinePtr(y);

		istd::CIntRanges::RangeList rangesList;
		const istd::CIntRanges* rangesPtr = bitmapRegion.GetPixelRanges(y);
		if (rangesPtr == nullptr){
			continue;
		}

		rangesPtr->GetAsList(clipArea.GetHorizontalRange(), rangesList);

		quint8* outputImageLinePtr = (quint8*)outputBitmap.GetLinePtr(y - regionTop);

		for (		istd::CIntRanges::RangeList::ConstIterator iter = rangesList.begin();
					iter != rangesList.end();
					++iter){
			const istd::CIntRange& pixelRange = *iter;

			int rangeStart = qMax(pixelRange.GetMinValue(), 0);
			Q_ASSERT(rangeStart >= regionLeft);
			int rangeEnd = qMin(pixelRange.GetMaxValue(), inputBitmap.GetImageSize().GetX());
			Q_ASSERT(rangeEnd <= regionRight);

			int bytesToCopy = (rangeEnd - rangeStart) * pixelBytes;

			if (bytesToCopy > 0){
				std::memcpy(	outputImageLinePtr + (rangeStart - regionLeft) * pixelBytes,
								inputLinePtr + rangeStart * pixelBytes,
								bytesToCopy);
			}
		}
	}

	return true;
}


bool CBitmapOperations::JoinBitmaps(
			const BitmapList& bitmapList,
			JoinMode joinMode,
			iimg::IBitmap& outputBitmap,
			istd::ILogger* operationLogPtr)
{
	int height = 0;
	int width = 0;
	int inputBitmapsCount = bitmapList.count();

	if (inputBitmapsCount <= 0){
		if (operationLogPtr != nullptr){
			operationLogPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, "The list of input bitmaps is empty", "BitmapOperations");
		}

		return false;
	}

	iimg::IBitmap::PixelFormat outputPixelFormat = bitmapList.at(0)->GetPixelFormat();

	// Calculate the size of the output bitmap:
	for (int i = 0; i < inputBitmapsCount; i++){
		istd::CIndex2d imageSize = bitmapList.at(i)->GetImageSize();
		if (joinMode == JM_VERTICAL){
			height += imageSize.GetY();
			width = qMax(width, imageSize.GetX());
		}
		else{
			width += imageSize.GetX();
			height = qMax(height, imageSize.GetY());
		}
	}

	if (!outputBitmap.CreateBitmap(outputPixelFormat, istd::CIndex2d(width, height))){
		if (operationLogPtr != nullptr){
			operationLogPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, "Output bitmap could not be created", "BitmapOperations");
		}

		return false;
	}

	int offsetX = 0;
	int offsetY = 0;

	for (int index = 0; index < inputBitmapsCount; index++){
		const iimg::IBitmap& inputSingleBitmap = *bitmapList.at(index);

		int singleBitmapHeight = inputSingleBitmap.GetImageSize().GetY();
		int singleBitmapWidth = inputSingleBitmap.GetImageSize().GetX();

		// If there is a bitmap in the list with different pixel format, cancel the calculation and exit:
		if (inputSingleBitmap.GetPixelFormat() != outputPixelFormat){
			if (operationLogPtr != nullptr){
				operationLogPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, "Input bitmaps have different pixel format", "BitmapOperations");
			}

			outputBitmap.ResetImage();

			return false;
		}

		int lineBytes = inputSingleBitmap.GetLineBytesCount();

		for (int y = 0; y < singleBitmapHeight; y++){
			void* outputLinePtr = nullptr;

			// Do vertical joining:
			if (joinMode == JM_VERTICAL){
				outputLinePtr = const_cast<void*>(outputBitmap.GetLinePtr(offsetY++));
			}
			// Do horizontal joining:
			else{
				outputLinePtr = (quint8*)(outputBitmap.GetLinePtr(y)) + offsetX;
			}

			const void* inputLinePtr = inputSingleBitmap.GetLinePtr(y);

			Q_ASSERT(outputLinePtr != nullptr);
			Q_ASSERT(inputLinePtr != nullptr);

			memcpy(outputLinePtr, inputLinePtr, lineBytes);
		}
		//horizontal joining
		offsetX += singleBitmapWidth;
	}

	return true;
}


bool CBitmapOperations::CombineBitmaps(
			const BitmapList& bitmapList,
			CombineMode combineMode,
			iimg::IBitmap& outputBitmap,
			imath::CVarVector* weghtsPtr,
			istd::ILogger* operationLogPtr)
{
	int inputBitmapsCount = bitmapList.count();

	if (inputBitmapsCount <= 0){
		if (operationLogPtr != nullptr){
			operationLogPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, "The list of input bitmaps is empty", "BitmapOperations");
		}

		return false;
	}

	iimg::IBitmap::PixelFormat outputPixelFormat = bitmapList.at(0)->GetPixelFormat();

	switch (outputPixelFormat){
		case iimg::IBitmap::PF_GRAY:
		case iimg::IBitmap::PF_RGB:
		case iimg::IBitmap::PF_RGBA:
			break;

		default:
			if (operationLogPtr != nullptr){
				operationLogPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, "Currently the image format is not supported for this operation", "BitmapOperations");
			}

			return false;
	}

	int width = bitmapList.at(0)->GetImageSize().GetX();
	int height = bitmapList.at(0)->GetImageSize().GetY();

	// Calculate the size of the output bitmap:
	for (int i = 0; i < inputBitmapsCount; i++){
		istd::CIndex2d imageSize = bitmapList.at(i)->GetImageSize();

		width = qMin(width, imageSize.GetX());
		height = qMin(height, imageSize.GetY());
	}

	if (!outputBitmap.CreateBitmap(outputPixelFormat, istd::CIndex2d(width, height))){
		if (operationLogPtr != nullptr){
			operationLogPtr->SendLogMessage(istd::IInformationProvider::IC_ERROR, 0, "Output bitmap could not be created", "BitmapOperations");
		}

		return false;
	}

	int pixelComponentsCount = outputBitmap.GetComponentsCount();

	for (int y = 0; y < height; y++){
		void* outputLinePtr = const_cast<void*>(outputBitmap.GetLinePtr(y));

		for (int x = 0; x < width; x++){
			for (int componentIndex = 0; componentIndex < pixelComponentsCount; componentIndex++){
				quint32 outputPixelComponentValue = 0;
				double weightSum = 0.0;

				switch (combineMode){
				case CM_MIN:
					weightSum = 1.0;
					outputPixelComponentValue = INT_MAX;
					break;

				case CM_MAX:
					weightSum = 1.0;
					break;

				default:
					break;
				}

				int pixelComponentIndex = x * pixelComponentsCount + componentIndex;

				for (int inputBitmapIndex = 0; inputBitmapIndex < inputBitmapsCount; inputBitmapIndex++){
					const iimg::IBitmap& inputBitmap = *bitmapList.at(inputBitmapIndex);

					const quint8* inputPixelPtr = ((quint8*)inputBitmap.GetLinePtr(y)) + pixelComponentIndex;

					switch (combineMode){
						case CM_AVERAGE:
							if (weghtsPtr == nullptr){
								outputPixelComponentValue += *inputPixelPtr;
								weightSum += 1.0;
							}
							else{
								double weight = weghtsPtr->GetElement(inputBitmapIndex);
								Q_ASSERT(weight >= 0);
								Q_ASSERT(weight <= 1.0);

								outputPixelComponentValue += *inputPixelPtr * weight;
							}
							break;

						case CM_MAX:
							outputPixelComponentValue = qMax<quint32>(*inputPixelPtr, outputPixelComponentValue);
							break;

						case CM_MIN:
							outputPixelComponentValue = qMin<quint32>(*inputPixelPtr, outputPixelComponentValue);
							break;
					}
				}

				quint8* outputPixelPtr = ((quint8*)outputLinePtr) + pixelComponentIndex;

				*outputPixelPtr = quint8(outputPixelComponentValue / weightSum);
			}
		}
	}

	return true;
}


bool CBitmapOperations::CaclulateBitmapDifference(
			const iimg::IBitmap& inputBitmap1,
			const iimg::IBitmap& inputBitmap2,
			iimg::IBitmap& outputBitmap,
			double offset,
			ilog::IMessageConsumer* messageConsumerPtr)
{
	iimg::IBitmap::PixelFormat pixelFormat = inputBitmap1.GetPixelFormat();

	if (pixelFormat != inputBitmap2.GetPixelFormat()){
		if (messageConsumerPtr != nullptr){
			messageConsumerPtr->AddMessage(
						ilog::IMessageConsumer::MessagePtr(
									new ilog::CMessage(
												ilog::CMessage::IC_ERROR,
												0,
												QCoreApplication::tr("Format of input images differs"),
												"Bitmap difference")));
		}

		return false;
	}

	istd::CIndex2d firstImageSize = inputBitmap1.GetImageSize();
	istd::CIndex2d secondImageSize = inputBitmap2.GetImageSize();

	istd::CIndex2d outputImageSize = istd::CIndex2d(qMin(firstImageSize.GetX(), secondImageSize.GetX()), qMin(firstImageSize.GetY(), secondImageSize.GetY()));
	if (!outputBitmap.CreateBitmap(pixelFormat, outputImageSize)){
		if (messageConsumerPtr != nullptr){
			messageConsumerPtr->AddMessage(
						ilog::IMessageConsumer::MessagePtr(
									new ilog::CMessage(
												ilog::CMessage::IC_ERROR,
												0,
												QCoreApplication::tr("Output bitmap could not be created"), 
												"Bitmap difference")));
		}

		return false;
	}

	switch (pixelFormat){
	case iimg::IBitmap::PF_RGB:
		CalculateDifferenceBitmap<quint8, quint8, quint8, int>(quint8(offset * 255), 255, inputBitmap1, inputBitmap2, outputBitmap);
		return true;
	case iimg::IBitmap::PF_GRAY:
		CalculateDifferenceBitmap<quint8, quint8, quint8, int>(quint8(offset * 255), 255, inputBitmap1, inputBitmap2, outputBitmap);
		return true;

	case iimg::IBitmap::PF_GRAY16:
		CalculateDifferenceBitmap<quint16, quint16, quint16, int>(quint16(offset * 0xffff), 0xffff, inputBitmap1, inputBitmap2, outputBitmap);
		return true;

	case iimg::IBitmap::PF_GRAY32:
		CalculateDifferenceBitmap<quint32, quint32, quint32, int>(quint16(offset * 0xffffffff), 0xffffffff, inputBitmap1, inputBitmap2, outputBitmap);
		return true;

	case iimg::IBitmap::PF_FLOAT32:
		CalculateDifferenceBitmap<float, float, float, float>(float(offset), 1.0f, inputBitmap1, inputBitmap2, outputBitmap);
		return true;

	case iimg::IBitmap::PF_FLOAT64:
		CalculateDifferenceBitmap<double, double, double, double>(offset, 1.0, inputBitmap1, inputBitmap2, outputBitmap);
		return true;

	default:
		if (messageConsumerPtr != nullptr){
			messageConsumerPtr->AddMessage(
						ilog::IMessageConsumer::MessagePtr(
									new ilog::CMessage(
												istd::IInformationProvider::IC_ERROR,
												0,
												QCoreApplication::tr("Input image format '%1' not supported").arg(iimg::CPixelFormatList::GetInstance().GetOptionName(pixelFormat)),
												QCoreApplication::tr("BitmapDifference"))));
		}
		return false;
	}
}


} // namespace iipr


