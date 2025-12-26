#include <iipr/CMorphologicalProcessorComp.h>


 // ACF includes
#include <istd/CChangeNotifier.h>
#include <istd/CIndex2d.h>
#include <ibase/CSize.h>
#include <iprm/TParamsPtr.h>
#include <iimg/CGeneralBitmap.h>
#include <iimg/CScanlineMask.h>
#include <imeas/INumericValue.h>
#include <iipr/CConvolutionKernel2d.h>
#include <iipr/CBitmapOperations.h>


namespace iipr
{


template <typename PixelComponentType>
inline void MinFunctor(PixelComponentType inputValue, PixelComponentType& outputValue)
{
	if (inputValue < outputValue){
		outputValue = inputValue;
	}
}

template <typename PixelComponentType>
inline void MaxFunctor(PixelComponentType inputValue, PixelComponentType& outputValue)
{
	if (inputValue > outputValue){
		outputValue = inputValue;
	}
}


static iipr::CConvolutionKernel2d GetCircleKernel(int kernelSize)
{
	Q_ASSERT((kernelSize % 2) != 0);

	iipr::CConvolutionKernel2d output(istd::CIndex2d(kernelSize, kernelSize), 1.0);

	int halfKernelSize = kernelSize / 2;
	double radius = kernelSize / 2.0;

	for (int y = -halfKernelSize; y <= halfKernelSize; ++y){
		for (int x = -halfKernelSize; x <= halfKernelSize; ++x){
			double distance = sqrt(double(x * x + y * y));
			if (distance > radius){
				istd::CIndex2d kernelIndex(x + halfKernelSize, y + halfKernelSize);

				output.SetKernelElement(kernelIndex, 0.0);
			}
		}
	}

	return output;
}


template <typename PixelComponentType, PixelComponentType OutputInitValue, void(*CalculateOutputValue)(PixelComponentType, PixelComponentType&)>
static void DoCircleFilter(
			int kernelWidth,
			int kernelHeight,
			const iimg::IBitmap& inputImage,
			const i2d::CRect& region,
			iimg::IBitmap& outputImage)
{
	int circleSize = qMax(kernelHeight, kernelWidth);
	if ((circleSize % 2) == 0){
		++circleSize;
	}

	int linesDifference = inputImage.GetLinesDifference();
	int pixelsDifference = inputImage.GetPixelsDifference();

	iipr::CConvolutionKernel2d kernel = GetCircleKernel(circleSize);

	int halfKernelSize = circleSize / 2.0;

	QVector<int> fastAccessElements;
	istd::CIndex2d kernelIndex;
	for (kernelIndex[1] = 0; kernelIndex[1] < circleSize; ++kernelIndex[1]){
		for (kernelIndex[0] = 0; kernelIndex[0] < circleSize; ++kernelIndex[0]){
			double value = kernel.GetKernelElement(kernelIndex);
			if (value != 0){
				int element;
				element = (kernelIndex[0] - halfKernelSize) * pixelsDifference + (kernelIndex[1] - halfKernelSize) * linesDifference;
				fastAccessElements.push_back(element);
			}
		}
	}

	int kernelElementsCount = int(fastAccessElements.size());

	const int regionTop = qMin(region.GetTop() + halfKernelSize, region.GetBottom() - halfKernelSize);
	const int regionBottom = qMax(region.GetTop() + halfKernelSize, region.GetBottom() - halfKernelSize);
	const int regionLeft = qMin(region.GetLeft() + halfKernelSize, region.GetRight() - halfKernelSize);
	const int regionRight = qMax(region.GetLeft() + halfKernelSize, region.GetRight() - halfKernelSize);

	int componentsCount = outputImage.GetComponentsCount();

	for (int componentIndex = 0; componentIndex < componentsCount; componentIndex++){
		for (int y = regionTop; y < regionBottom; ++y){

			const PixelComponentType* inputLinePtr = (PixelComponentType*)inputImage.GetLinePtr(y);
			PixelComponentType* outputLinePtr = (PixelComponentType*)outputImage.GetLinePtr(y);

			for (int x = regionLeft; x < regionRight; ++x){
				PixelComponentType outputValue = OutputInitValue;

				int outputComponentPosition = x * componentsCount + componentIndex;

				for (int i = 0; i < kernelElementsCount; ++i){
					const int& kernelElement = fastAccessElements[i];

					int componentPosition = (x + kernelElement) * componentsCount + componentIndex;

					CalculateOutputValue(inputLinePtr[componentPosition], outputValue);
				}

				*(outputLinePtr + outputComponentPosition) = outputValue;
			}
		}
	} // componentIndex
}


template <typename PixelComponentType, PixelComponentType OutputInitValue, void (*CalculateOutputValue)(PixelComponentType, PixelComponentType&)>
static void DoFilter(
			CMorphologicalProcessorComp::KernelType kernelType,
			int kernelWidth,
			int kernelHeight,
			const iimg::IBitmap& inputImage,
			const i2d::CRect& region,
			iimg::IBitmap& outputImage)
{
	if (kernelType == CMorphologicalProcessorComp::KT_CIRCLE){
		DoCircleFilter<PixelComponentType, OutputInitValue, CalculateOutputValue>(kernelWidth, kernelHeight, inputImage, region, outputImage);
	}
	else {

		int kernelHalfWidth = kernelWidth / 2.0;
		int kernelHalfHeight = kernelHeight / 2.0;

		int regionLeft = region.GetLeft();
		int regionRight = region.GetRight();
		int regionTop = region.GetTop();
		int regionBottom = region.GetBottom();

		int inputImageWidth = inputImage.GetImageSize().GetX();
		int inputImageHeight = inputImage.GetImageSize().GetY();

		int componentsCount = outputImage.GetComponentsCount();

		for (int componentIndex = 0; componentIndex < componentsCount; componentIndex++){
			for (int y = regionTop; y < regionBottom; ++y){
				Q_ASSERT(y >= 0);
				Q_ASSERT(y < inputImageHeight);

				const PixelComponentType* inputLinePtr = (PixelComponentType*)inputImage.GetLinePtr(y);
				PixelComponentType* outputLinePtr = (PixelComponentType*)outputImage.GetLinePtr(y);

				for(int x = regionLeft; x < regionLeft + kernelHalfWidth; ++x){
					PixelComponentType outputValue = OutputInitValue;

					int outputComponentPosition = x * componentsCount + componentIndex;

					for(int kernelX = -kernelHalfWidth; kernelX <= kernelHalfWidth; ++kernelX){
						int pixelIndex = x + kernelX;

						if (pixelIndex >= regionLeft && pixelIndex < inputImageWidth){
							int componentPosition = pixelIndex * componentsCount + componentIndex;

							CalculateOutputValue(inputLinePtr[componentPosition], outputValue);
						}
					}

					*(outputLinePtr + outputComponentPosition) = outputValue;
				}

				for(int x = regionLeft + kernelHalfWidth; x < regionRight - kernelHalfWidth; ++x){
					PixelComponentType outputValue = OutputInitValue;
					int outputComponentPosition = x * componentsCount + componentIndex;

					for(int kernelX = -kernelHalfWidth; kernelX <= kernelHalfWidth; ++kernelX){
						int pixelIndex = (x + kernelX) * componentsCount + componentIndex;

						CalculateOutputValue(inputLinePtr[pixelIndex], outputValue);
					}

					*(outputLinePtr + outputComponentPosition) = outputValue;
				}

				for(int x = regionRight - kernelHalfWidth; x < regionRight; ++x){
					PixelComponentType outputValue = OutputInitValue;
					int outputComponentPosition = x * componentsCount + componentIndex;

					for(int kernelX = -kernelHalfWidth; kernelX <= kernelHalfWidth; ++kernelX){
						int pixelIndex = x + kernelX;

						if (pixelIndex >= 0 && pixelIndex < regionRight){
							int componentPosition = pixelIndex * componentsCount + componentIndex;

							CalculateOutputValue(inputLinePtr[componentPosition], outputValue);
						}
					}

					*(outputLinePtr + outputComponentPosition) = outputValue;
				}
			}
		} // componentIndex

		for (int componentIndex = 0; componentIndex < componentsCount; componentIndex++){
			iimg::CGeneralBitmap tempBitmap;
			tempBitmap.CopyFrom(outputImage);

			int inputLineDifference = tempBitmap.GetLinesDifference();

			for (int y = regionTop; y < regionTop + kernelHalfHeight; ++y){
				Q_ASSERT(y >= 0);
				Q_ASSERT(y < inputImageHeight);

				const PixelComponentType* inputLinePtr = (PixelComponentType*)tempBitmap.GetLinePtr(y);
				PixelComponentType* outputLinePtr = (PixelComponentType*)outputImage.GetLinePtr(y);

				for(int x = regionLeft; x < regionRight; ++x){
					PixelComponentType outputValue = OutputInitValue;

					int componentPosition = x * componentsCount + componentIndex;

					for(int kernelY = -kernelHalfHeight; kernelY <= kernelHalfHeight; ++kernelY){
						int imageLineIndex = y + kernelY;

						if (imageLineIndex >= regionTop && imageLineIndex < inputImageHeight){
							PixelComponentType value = inputLinePtr[componentPosition + kernelY * inputLineDifference];

							CalculateOutputValue(value, outputValue);
						}
					}

					*(outputLinePtr + componentPosition) = outputValue;
				}
			}

			for (int y = regionTop + kernelHalfHeight; y < regionBottom - kernelHalfHeight; ++y){
				Q_ASSERT(y >= 0);
				Q_ASSERT(y < inputImageHeight);

				const PixelComponentType* inputLinePtr = (PixelComponentType*)tempBitmap.GetLinePtr(y);
				PixelComponentType* outputLinePtr = (PixelComponentType*)outputImage.GetLinePtr(y);

				for(int x = regionLeft; x < regionRight; ++x){
					PixelComponentType outputValue = OutputInitValue;

					int componentPosition = x * componentsCount + componentIndex;

					for(int kernelY = -kernelHalfHeight; kernelY <= kernelHalfHeight; ++kernelY){
						PixelComponentType value = inputLinePtr[componentPosition + kernelY * inputLineDifference];

						CalculateOutputValue(value, outputValue);
					}

					*(outputLinePtr + componentPosition) = outputValue;
				}
			}

			for (int y = regionBottom - kernelHalfHeight; y < regionBottom; ++y){
				Q_ASSERT(y >= 0);
				Q_ASSERT(y < inputImageHeight);

				const PixelComponentType* inputLinePtr = (PixelComponentType*)tempBitmap.GetLinePtr(y);
				PixelComponentType* outputLinePtr = (PixelComponentType*)outputImage.GetLinePtr(y);

				for(int x = regionLeft; x < regionRight; ++x){
					PixelComponentType outputValue = OutputInitValue;
					int componentPosition = x * componentsCount + componentIndex;

					for(int kernelY = -kernelHalfHeight; kernelY <= kernelHalfHeight; ++kernelY){
						int imageLineIndex = y + kernelY;

						if (imageLineIndex >= 0 && imageLineIndex < regionBottom){
							PixelComponentType value = inputLinePtr[componentPosition + kernelY * inputLineDifference];

							CalculateOutputValue(value, outputValue);
						}
					}

					*(outputLinePtr + componentPosition) = outputValue;
				}
			}
		} // componentIndex
	}
}


template <typename PixelComponentType, PixelComponentType InitMaxValue>
static void ProcessImage(
			CMorphologicalProcessorComp::ProcessingMode processingMode,
			CMorphologicalProcessorComp::KernelType kernelType,
			int kernelWidth,
			int kernelHeight,
			const iimg::IBitmap& inputImage,
			const i2d::CRect& regionRect,
			iimg::IBitmap& outputImage)
{	
	switch (processingMode){
		case CMorphologicalProcessorComp::PM_EROSION:
			DoFilter<PixelComponentType, InitMaxValue, MinFunctor<PixelComponentType> >(kernelType, kernelWidth, kernelHeight, inputImage, regionRect, outputImage);
			break;
				
		case CMorphologicalProcessorComp::PM_DILATATION:
			DoFilter<PixelComponentType, 0, MaxFunctor<PixelComponentType> >(kernelType, kernelWidth, kernelHeight, inputImage, regionRect, outputImage);
			break;

		case CMorphologicalProcessorComp::PM_OPENING:{
			iimg::CGeneralBitmap tempBitmap;
			tempBitmap.CopyFrom(outputImage);

			DoFilter<PixelComponentType, InitMaxValue, MinFunctor<PixelComponentType> >(kernelType, kernelWidth, kernelHeight, inputImage, regionRect, tempBitmap);
			DoFilter<PixelComponentType, 0, MaxFunctor<PixelComponentType> >(kernelType, kernelWidth, kernelHeight, tempBitmap, regionRect, outputImage);
			break;
		}

		case CMorphologicalProcessorComp::PM_CLOSING:{
			iimg::CGeneralBitmap tempBitmap;
			tempBitmap.CopyFrom(outputImage);

			DoFilter<PixelComponentType, 0, MaxFunctor<PixelComponentType> >(kernelType, kernelWidth, kernelHeight, inputImage, regionRect, tempBitmap);
			DoFilter<PixelComponentType, InitMaxValue, MinFunctor<PixelComponentType> >(kernelType, kernelWidth, kernelHeight, tempBitmap, regionRect, outputImage);
			break;
		}

		case CMorphologicalProcessorComp::PM_WHITE_TOP_HAT:{
			iimg::CGeneralBitmap tempBitmap;

			if (tempBitmap.CreateBitmap(
						inputImage.GetPixelFormat(),
						inputImage.GetImageSize(),
						inputImage.GetPixelBitsCount(),
						inputImage.GetComponentsCount())){

				ProcessImage<PixelComponentType, InitMaxValue>(
							CMorphologicalProcessorComp::PM_OPENING,
							kernelType,
							kernelWidth,
							kernelHeight,
							inputImage,
							regionRect,
							tempBitmap);

				iipr::CBitmapOperations::CaclulateBitmapDifference(tempBitmap, inputImage, outputImage);
			}

			break;
		}

		case CMorphologicalProcessorComp::PM_BLACK_TOP_HAT:{
			iimg::CGeneralBitmap tempBitmap;

			if (tempBitmap.CreateBitmap(
						inputImage.GetPixelFormat(),
						inputImage.GetImageSize(),
						inputImage.GetPixelBitsCount(),
						inputImage.GetComponentsCount())){

				ProcessImage<PixelComponentType, InitMaxValue>(
							CMorphologicalProcessorComp::PM_CLOSING,
							kernelType,
							kernelWidth,
							kernelHeight,
							inputImage,
							regionRect,
							tempBitmap);

				iipr::CBitmapOperations::CaclulateBitmapDifference(tempBitmap, inputImage, outputImage);
			}

			break;
		}

		case CMorphologicalProcessorComp::PM_MORPHO_GRADIENT:{
			iimg::CGeneralBitmap dilatedBitmap;
			iimg::CGeneralBitmap erodedBitmap;

			if (dilatedBitmap.CreateBitmap(
						inputImage.GetPixelFormat(),
						inputImage.GetImageSize(),
						inputImage.GetPixelBitsCount(),
						inputImage.GetComponentsCount())){
				ProcessImage<PixelComponentType, InitMaxValue>(
							CMorphologicalProcessorComp::PM_DILATATION,
							kernelType,
							kernelWidth,
							kernelHeight,
							inputImage,
							regionRect,
							dilatedBitmap);
			}

			if (erodedBitmap.CreateBitmap(
						inputImage.GetPixelFormat(),
						inputImage.GetImageSize(),
						inputImage.GetPixelBitsCount(),
						inputImage.GetComponentsCount())){
				ProcessImage<PixelComponentType, InitMaxValue>(
							CMorphologicalProcessorComp::PM_EROSION,
							kernelType,
							kernelWidth,
							kernelHeight,
							inputImage,
							regionRect,
							erodedBitmap);
			}

			iipr::CBitmapOperations::CaclulateBitmapDifference(dilatedBitmap, erodedBitmap, outputImage);

			break;
		}
	}
}


// protected methods

// reimplemented (CImageRegionProcessorCompBase)

bool CMorphologicalProcessorComp::ProcessImageRegion(
			const iimg::IBitmap& inputBitmap,
			const iprm::IParamsSet* paramsPtr,
			const i2d::IObject2d* aoiPtr,
			istd::IChangeable* outputPtr) const
{
	if (inputBitmap.IsEmpty()){
		SendWarningMessage(0, "Input bitmap is empty.");

		return false;
	}

	if (aoiPtr == nullptr){
		SendWarningMessage(0, "Filter region is not defined");

		return false;
	}

	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == nullptr){
		SendWarningMessage(0, "Output bitmap is not set");

		return false;
	}

	istd::CIndex2d inputBitmapSize = inputBitmap.GetImageSize();

	iimg::CScanlineMask bitmapRegion;
	i2d::CRect clipArea(inputBitmapSize);
	if (!bitmapRegion.CreateFromGeometry(*aoiPtr, &clipArea)){
		SendWarningMessage(0, "Cannot create the region");

		return false;
	}

	i2d::CRect regionRect = bitmapRegion.GetBoundingBox();
	if (regionRect.IsEmpty() || !regionRect.IsValid()){
		SendWarningMessage(0, "Cannot process an empty or invalid region");

		return false;
	}

	if (!outputBitmapPtr->CopyFrom(inputBitmap)){
		SendErrorMessage(0, "Data could not be copied from input bitmap to the output");

		return false;
	}

	iprm::TParamsPtr<imeas::INumericValue> filterSizePtr(paramsPtr, *m_filterSizeParamsIdAttrPtr);
	if (filterSizePtr == nullptr){
		SendErrorMessage(0, "No fiter dimension was set");

		return false;
	}

	imath::CVarVector filterLengths = filterSizePtr->GetValues();
	int filterDimensionsCount = filterLengths.GetElementsCount();
	if (filterDimensionsCount < 1){
		SendErrorMessage(0, "Processing filter can't have dimension smaller 1");

		return false;
	}

	regionRect.Intersection(i2d::CRect(inputBitmap.GetImageSize()));

	int imageWidth = inputBitmapSize.GetX();
	int imageHeight = inputBitmapSize.GetY();

	int kernelMaxWidth = qMax(1, qMin(int(filterLengths[0]), imageWidth));
	int kernelMaxHeight = qMax(1, qMin((filterDimensionsCount < 2)? kernelMaxWidth: int(filterLengths[1]), imageHeight));

	if (kernelMaxWidth == 1 && kernelMaxHeight == 1){
		return true; // do nothing
	}

	Q_ASSERT(kernelMaxWidth > 1);
	Q_ASSERT(kernelMaxHeight > 1);
	
	ProcessingMode processingMode = GetProcessingMode(paramsPtr);

	KernelType kernelType = GetKernelType(paramsPtr);

	int pixelFormat = inputBitmap.GetPixelFormat();
	switch (pixelFormat){
		case iimg::IBitmap::PF_GRAY:
		case iimg::IBitmap::PF_RGB:
		case iimg::IBitmap::PF_RGBA:
			ProcessImage<quint8, 255>(
							processingMode,
							kernelType,
							kernelMaxWidth,
							kernelMaxHeight,
							inputBitmap,
							regionRect,
							*outputBitmapPtr);
			break;

		case iimg::IBitmap::PF_GRAY16:
			ProcessImage<quint16, (1 << 16) - 1>(
							processingMode,
							kernelType,
							kernelMaxWidth,
							kernelMaxHeight,
							inputBitmap,
							regionRect,
							*outputBitmapPtr);
			break;

		case iimg::IBitmap::PF_GRAY32:
			ProcessImage<quint32, (quint64(1) << 32) - 1>(
							processingMode,
							kernelType,
							kernelMaxWidth,
							kernelMaxHeight,
							inputBitmap,
							regionRect,
							*outputBitmapPtr);
			break;

	default:
		return false;
	}

	return true;
}


// reimplemented (icomp::CComponentBase)

void CMorphologicalProcessorComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	int modeIndex = 0;
	if (!m_erosionEnableAttrPtr.IsValid() || *m_erosionEnableAttrPtr){
		m_processingModes.InsertOption(QObject::tr("Erosion"), "Erosion");
		m_processingModeIndexMap[modeIndex++] = PM_EROSION;
	}

	if (!m_dilatationEnableAttrPtr.IsValid() || *m_dilatationEnableAttrPtr){
		m_processingModes.InsertOption(QObject::tr("Dilation"), "Dilation");
		m_processingModeIndexMap[modeIndex++] = PM_DILATATION;
	}

	if (!m_openingEnableAttrPtr.IsValid() || *m_openingEnableAttrPtr){
		m_processingModes.InsertOption(QObject::tr("Opening"), "Opening");
		m_processingModeIndexMap[modeIndex++] = PM_OPENING;
	}

	if (!m_closingEnableAttrPtr.IsValid() || *m_closingEnableAttrPtr){
		m_processingModes.InsertOption(QObject::tr("Closing"), "Closing");
		m_processingModeIndexMap[modeIndex++] = PM_CLOSING;
	}

	if (!m_whiteTopEnableAttrPtr.IsValid() || *m_whiteTopEnableAttrPtr){
		m_processingModes.InsertOption(QObject::tr("White Top-Hat"), "WhiteTopHat");
		m_processingModeIndexMap[modeIndex++] = PM_WHITE_TOP_HAT;
	}

	if (!m_blackTopEnableAttrPtr.IsValid() || *m_blackTopEnableAttrPtr){
		m_processingModes.InsertOption(QObject::tr("Black Top-Hat"), "BlackTopHat");
		m_processingModeIndexMap[modeIndex++] = PM_BLACK_TOP_HAT;
	}

	if (!m_morthoGradientEnableAttrPtr.IsValid() || *m_morthoGradientEnableAttrPtr){
		m_processingModes.InsertOption(QObject::tr("Morphological Gradient"), "MorphologicalGradient");
		m_processingModeIndexMap[modeIndex] = PM_MORPHO_GRADIENT;
	}

	int formIndex = 0;
	if (!m_rectangleFormEnableAttrPtr.IsValid() || *m_rectangleFormEnableAttrPtr){
		m_filterForms.InsertOption(QObject::tr("Rectangle"), "Rectangle");
		m_filterFormIndexMap[formIndex++] = KT_RECT;
	}

	if (!m_circleFormEnableAttrPtr.IsValid() || *m_circleFormEnableAttrPtr){
		m_filterForms.InsertOption(QObject::tr("Circle"), "Circle");
		m_filterFormIndexMap[formIndex] = KT_CIRCLE;
	}
}


// private methods

CMorphologicalProcessorComp::ProcessingMode  CMorphologicalProcessorComp::GetProcessingMode(const iprm::IParamsSet* paramsPtr) const
{
	int mode = *m_defaultProcessingModeAttrPtr;

	if ((paramsPtr != nullptr) && m_processingModeIdAttrPtr.IsValid()){
		iprm::TParamsPtr<iprm::ISelectionParam> processingModeParamPtr(paramsPtr, *m_processingModeIdAttrPtr, false);
		if (processingModeParamPtr.IsValid()){
			int index = processingModeParamPtr->GetSelectedOptionIndex();
			mode = m_processingModeIndexMap.contains(index) ? 
				m_processingModeIndexMap[index] : PM_FIRST;
		}
	}

	if (mode < PM_FIRST || mode > PM_LAST){
		mode = m_processingModeIndexMap.contains(0) ? m_processingModeIndexMap[0] : PM_FIRST;
	}

	return ProcessingMode(mode);
}


CMorphologicalProcessorComp::KernelType CMorphologicalProcessorComp::GetKernelType(const iprm::IParamsSet* paramsPtr) const
{
	int formType = *m_defaultFilterFormTypeAttrPtr;

	if ((paramsPtr != nullptr) && m_filterFormTypeIdAttrPtr.IsValid()){
		iprm::TParamsPtr<iprm::ISelectionParam> filterFormTypeParamPtr(paramsPtr, *m_filterFormTypeIdAttrPtr, false);
		if (filterFormTypeParamPtr.IsValid()){
			int index = filterFormTypeParamPtr->GetSelectedOptionIndex();
			formType = m_filterFormIndexMap.contains(index) ? m_filterFormIndexMap[index] : KT_FIRST;
		}
	}

	if (formType < KT_FIRST || formType > KT_LAST){
		formType  = m_filterFormIndexMap.contains(0) ? m_filterFormIndexMap[0] :  KT_FIRST;
	}

	return KernelType(formType);
}


} // namespace iipr


