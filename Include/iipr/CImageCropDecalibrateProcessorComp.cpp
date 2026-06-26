// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CImageCropDecalibrateProcessorComp.h>


// Qt includes
#include <QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtCore/QtMath>
#else
#include <QtCore/qmath.h>
#endif

// ACF includes
#include <istd/TArray.h>
#include <ilog/CMessage.h>
#include <iprm/IParamsSet.h>
#include <iprm/TParamsPtr.h>
#include <iimg/IBitmap.h>
#include <iimg/CPixelFormatList.h>

// ACF-Solutions includes
#include <iipr/CPixelManip.h>


namespace iipr
{


/**
	Template filter funtion.
*/
template <	typename PixelType,
			typename WorkingType>
bool DoCropImageTemplate(
			int cellSize,
			bool useLinearInterpolation,
			const istd::TArray<i2d::CVector2d, 2>& cornerArray,
			const iimg::IBitmap& inputBitmap,
			iimg::IBitmap& outputBitmap)
{
	Q_ASSERT(inputBitmap.GetPixelFormat() == outputBitmap.GetPixelFormat());

	const PixelType* sourceBufferPtr = (const PixelType*)inputBitmap.GetLinePtr(0);
	int sourceLineDiff = inputBitmap.GetLinesDifference();

	PixelType* destBufferPtr = (PixelType*)outputBitmap.GetLinePtr(0);
	int destLineDiff = outputBitmap.GetLinesDifference();

	istd::CIndex2d inputImageSize = inputBitmap.GetImageSize();
	i2d::CRectangle inputPosRectangle = useLinearInterpolation?
				i2d::CRectangle(I_BIG_EPSILON, I_BIG_EPSILON, inputImageSize.GetX() - 2 * I_BIG_EPSILON - 1, inputImageSize.GetY() - 2 * I_BIG_EPSILON - 1):
				i2d::CRectangle(I_BIG_EPSILON, I_BIG_EPSILON, inputImageSize.GetX() - 2 * I_BIG_EPSILON, inputImageSize.GetY() - 2 * I_BIG_EPSILON);

	istd::CIndex2d outputImageSize = outputBitmap.GetImageSize();

	istd::CIndex2d gridSize = cornerArray.GetSizes();

	static i2d::CVector2d sourcePixelCenterPos(0.5, 0.5);

	istd::CIndex2d gridIndex;
	for (gridIndex[1] = 0; gridIndex[1] < gridSize[1] - 1; ++gridIndex[1]){
		for (gridIndex[0] = 0; gridIndex[0] < gridSize[0] - 1; ++gridIndex[0]){
			i2d::CVector2d posLeftTop = cornerArray.GetAt(gridIndex) - sourcePixelCenterPos;
			i2d::CVector2d posRightTop = cornerArray.GetAt(istd::CIndex2d(gridIndex.GetX() + 1, gridIndex.GetY())) - sourcePixelCenterPos;
			i2d::CVector2d posLeftBottom = cornerArray.GetAt(istd::CIndex2d(gridIndex.GetX(), gridIndex.GetY() + 1)) - sourcePixelCenterPos;
			i2d::CVector2d posRightBottom = cornerArray.GetAt(istd::CIndex2d(gridIndex.GetX() + 1, gridIndex.GetY() + 1)) - sourcePixelCenterPos;

			if ((posRightTop.GetX() == qInf()) || (posRightBottom.GetX() == qInf())){
				++gridIndex[0];
				continue;
			}

			if ((posLeftTop.GetX() == qInf()) || (posLeftBottom.GetX() == qInf())){
				continue;
			}

			int maxX = qMin(cellSize, outputImageSize.GetX() - gridIndex[0] * cellSize);
			int maxY = qMin(cellSize, outputImageSize.GetY() - gridIndex[1] * cellSize);

			if (		inputPosRectangle.Contains(posLeftTop) &&
						inputPosRectangle.Contains(posRightTop) &&
						inputPosRectangle.Contains(posLeftBottom) &&
						inputPosRectangle.Contains(posRightBottom)){
				if (useLinearInterpolation){
					for (int y = 0; y < maxY; ++y){
						PixelType* destLinePtr = (PixelType*)((quint8*)(destBufferPtr + gridIndex[0] * cellSize) + (gridIndex[1] * cellSize + y) * destLineDiff);

						double bottomFactor = y + 0.5;
						double topFactor = cellSize - bottomFactor;

						i2d::CVector2d normPosLeft = (posLeftTop * topFactor + posLeftBottom * bottomFactor) / (cellSize * cellSize);
						i2d::CVector2d normPosRight = (posRightTop * topFactor + posRightBottom * bottomFactor) / (cellSize * cellSize);

						for (int x = 0; x < maxX; ++x){
							double rightFactor = x + 0.5;
							double leftFactor = cellSize - rightFactor;
							i2d::CVector2d sourcePos = normPosLeft * leftFactor + normPosRight * rightFactor;

							int sourceX = int(sourcePos.GetX());
							double alphaX = sourcePos.GetX() - sourceX;
							int sourceY = int(sourcePos.GetY());
							double alphaY = sourcePos.GetY() - sourceY;

							const PixelType* pixelPtr = (const PixelType*)((const quint8*)(sourceBufferPtr + sourceX) + sourceY * sourceLineDiff);

							WorkingType pixel11Value = *pixelPtr;
							WorkingType pixel12Value = *(pixelPtr + 1);
							pixelPtr = (PixelType*)((quint8*)pixelPtr + sourceLineDiff);
							WorkingType pixel21Value = *pixelPtr;
							WorkingType pixel22Value = *(pixelPtr + 1);

							pixel11Value *= (1 - alphaX);
							pixel12Value *= alphaX;
							pixel12Value += pixel11Value;
							pixel12Value *= (1 - alphaY);
							pixel21Value *= (1 - alphaX);
							pixel22Value *= alphaX;
							pixel22Value += pixel21Value;
							pixel22Value *= alphaY;
							pixel22Value += pixel12Value;

							destLinePtr[x] = PixelType(pixel22Value);
						}
					}
				}
				else{
					for (int y = 0; y < maxY; ++y){
						PixelType* destLinePtr = (PixelType*)((quint8*)(destBufferPtr + gridIndex[0] * cellSize) + (gridIndex[1] * cellSize + y) * destLineDiff);

						double bottomFactor = y + 0.5;
						double topFactor = cellSize - bottomFactor;

						i2d::CVector2d normPosLeft = (posLeftTop * topFactor + posLeftBottom * bottomFactor) / (cellSize * cellSize);
						i2d::CVector2d normPosRight = (posRightTop * topFactor + posRightBottom * bottomFactor) / (cellSize * cellSize);

						for (int x = 0; x < maxX; ++x){
							double rightFactor = x + 0.5;
							double leftFactor = cellSize - rightFactor;
							i2d::CVector2d sourcePos = normPosLeft * leftFactor + normPosRight * rightFactor;

							int sourceX = int(sourcePos.GetX() + 0.5);
							int sourceY = int(sourcePos.GetY() + 0.5);

							destLinePtr[x] = PixelType(*(const PixelType*)((quint8*)(sourceBufferPtr + sourceX) + sourceY * sourceLineDiff));
						}
					}
				}
			}
			else{
				if (useLinearInterpolation){
					for (int y = 0; y < maxY; ++y){
						PixelType* destLinePtr = (PixelType*)((quint8*)(destBufferPtr + gridIndex[0] * cellSize) + (gridIndex[1] * cellSize + y) * destLineDiff);

						double bottomFactor = y + 0.5;
						double topFactor = cellSize - bottomFactor;

						i2d::CVector2d normPosLeft = (posLeftTop * topFactor + posLeftBottom * bottomFactor) / (cellSize * cellSize);
						i2d::CVector2d normPosRight = (posRightTop * topFactor + posRightBottom * bottomFactor) / (cellSize * cellSize);

						for (int x = 0; x < maxX; ++x){
							double rightFactor = x + 0.5;
							double leftFactor = cellSize - rightFactor;
							i2d::CVector2d sourcePos = normPosLeft * leftFactor + normPosRight * rightFactor;

							if (inputPosRectangle.Contains(sourcePos)){
								int sourceX = int(sourcePos.GetX());
								double alphaX = sourcePos.GetX() - sourceX;
								int sourceY = int(sourcePos.GetY());
								double alphaY = sourcePos.GetY() - sourceY;

								const PixelType* pixelPtr = (const PixelType*)((quint8*)(sourceBufferPtr + sourceX) + sourceY * sourceLineDiff);

								WorkingType pixel11Value = *pixelPtr;
								WorkingType pixel12Value = *(pixelPtr + 1);
								pixelPtr = (PixelType*)((quint8*)pixelPtr + sourceLineDiff);
								WorkingType pixel21Value = *pixelPtr;
								WorkingType pixel22Value = *(pixelPtr + 1);

								pixel11Value *= (1 - alphaX);
								pixel12Value *= alphaX;
								pixel12Value += pixel11Value;
								pixel12Value *= (1 - alphaY);
								pixel21Value *= (1 - alphaX);
								pixel22Value *= alphaX;
								pixel22Value += pixel21Value;
								pixel22Value *= alphaY;
								pixel22Value += pixel12Value;

								destLinePtr[x] = PixelType(pixel22Value);
							}
							else{
								destLinePtr[x] = WorkingType(0);
							}
						}
					}
				}
				else{
					for (int y = 0; y < maxY; ++y){
						PixelType* destLinePtr = (PixelType*)((quint8*)(destBufferPtr + gridIndex[0] * cellSize) + (gridIndex[1] * cellSize + y) * destLineDiff);

						double bottomFactor = y + 0.5;
						double topFactor = cellSize - bottomFactor;

						i2d::CVector2d normPosLeft = (posLeftTop * topFactor + posLeftBottom * bottomFactor) / (cellSize * cellSize);
						i2d::CVector2d normPosRight = (posRightTop * topFactor + posRightBottom * bottomFactor) / (cellSize * cellSize);

						for (int x = 0; x < maxX; ++x){
							double rightFactor = x + 0.5;
							double leftFactor = cellSize - rightFactor;
							i2d::CVector2d sourcePos = normPosLeft * leftFactor + normPosRight * rightFactor;

							if (inputPosRectangle.Contains(sourcePos)){
								int sourceX = int(sourcePos.GetX() + 0.5);
								int sourceY = int(sourcePos.GetY() + 0.5);

								destLinePtr[x] = PixelType(*(const PixelType*)((quint8*)(sourceBufferPtr + sourceX) + sourceY * sourceLineDiff));
							}
							else{
								destLinePtr[x] = WorkingType(0);
							}
						}
					}
				}
			}
		}
	}

	return true;
}


CImageCropDecalibrateProcessorComp::CImageCropDecalibrateProcessorComp()
{
	m_orientationModes.InsertOption(QObject::tr("Simple"), "Simple");
	m_orientationModes.InsertOption(QObject::tr("Visual"), "Visual");
	m_orientationModes.InsertOption(QObject::tr("No Reflexion"), "NoReflexion");

	m_interpolationModes.InsertOption(QObject::tr("Simple"), "Simple");
	m_interpolationModes.InsertOption(QObject::tr("Linear"), "Linear");
}


bool CImageCropDecalibrateProcessorComp::CropImage(
			const i2d::CRectangle& sourceAoi,
			int cellSize,
			const iimg::IBitmap& inputBitmap,
			iimg::IBitmap& outputBitmap,
			istd::CIndex2d* outputImageSizePtr,
			int interpolationMode,
			int orientationMode,
			ilog::IMessageConsumer* resultConsumerPtr)
{
	if (cellSize < 2){
		return false;
	}

	istd::CIndex2d outputImageSize;
	if (outputImageSizePtr != NULL){
		outputImageSize = *outputImageSizePtr;
	}
	else{
		if (!CalcOutputImageSize(sourceAoi, outputImageSize)){
			return false;
		}
	}

	iimg::IBitmap::PixelFormat pixelFormat = inputBitmap.GetPixelFormat();

	istd::CIndex2d gridSize((outputImageSize.GetX() + cellSize - 1) / cellSize + 1, (outputImageSize.GetY() + cellSize - 1) / cellSize + 1);
	if (!outputBitmap.CreateBitmap(pixelFormat, outputImageSize)){
		return false;
	}

	istd::TArray<i2d::CVector2d, 2> cornerArray;
	// MESF ACF-version compatibility
	cornerArray.SetSizes(gridSize);
	cornerArray.SetAllElements(i2d::CVector2d(qInf(), qInf()));

	const i2d::ICalibration2d* calibrationPtr = sourceAoi.GetCalibration();

	bool flipHorizontal = false;
	bool flipVertical = false;
	if ((orientationMode == OM_NO_REFLEXION) || (orientationMode == OM_VISUAL)){
		if (calibrationPtr != NULL){
			i2d::CAffine2d approxTransform;
			if (calibrationPtr->GetLocalTransform(i2d::CVector2d::GetZero(), approxTransform)){
				if (orientationMode == OM_VISUAL){
					if (approxTransform.GetDeformMatrix().GetAxisX().GetX() < 0){
						flipHorizontal = true;
						flipVertical = true;
					}
				}

				if (approxTransform.GetDeformMatrix().GetDet() < 0){
					flipVertical = !flipVertical;
				}
			}
		}
	}

	istd::CIndex2d gridIndex;
	for (gridIndex[1] = 0; gridIndex[1] < gridSize[1]; ++gridIndex[1]){
		double propY = double(gridIndex.GetY() * cellSize) / outputImageSize.GetY();
		if (flipVertical){
			propY = 1 - propY;
		}
		double cornerDestY = sourceAoi.GetVerticalRange().GetValueFromAlpha(propY);

		for (gridIndex[0] = 0; gridIndex[0] < gridSize[0]; ++gridIndex[0]){
			double propX = double(gridIndex.GetX() * cellSize) / outputImageSize.GetX();
			if (flipHorizontal){
				propX = 1 - propX;
			}
			double cornerDestX = sourceAoi.GetHorizontalRange().GetValueFromAlpha(propX);

			if (calibrationPtr != NULL){
				i2d::CVector2d originalPos;
				if (calibrationPtr->GetPositionAt(i2d::CVector2d(cornerDestX, cornerDestY), originalPos)){
					cornerArray.SetAt(gridIndex, originalPos);
				}
			}
			else{
				cornerArray.SetAt(gridIndex, i2d::CVector2d(cornerDestX, cornerDestY));
			}
		}
	}


	bool retVal = false;

	bool useLinearInterpolation = interpolationMode == IM_LINEAR;

	switch (pixelFormat){
	case iimg::IBitmap::PF_GRAY:
		retVal = DoCropImageTemplate<quint8, quint8>(cellSize, useLinearInterpolation, cornerArray, inputBitmap, outputBitmap);
		break;

	case iimg::IBitmap::PF_RGB:
	case iimg::IBitmap::PF_RGBA:
		retVal = DoCropImageTemplate< iipr::CPixelManip::Rgba, iipr::CPixelManip::RgbCropAccum32<qint32, 16, true, true> >(cellSize, useLinearInterpolation, cornerArray, inputBitmap, outputBitmap);
		break;

	case iimg::IBitmap::PF_GRAY16:
		retVal = DoCropImageTemplate<quint16, quint16>(cellSize, useLinearInterpolation, cornerArray, inputBitmap, outputBitmap);
		break;

	case iimg::IBitmap::PF_GRAY32:
		retVal = DoCropImageTemplate<quint32, quint32>(cellSize, useLinearInterpolation, cornerArray, inputBitmap, outputBitmap);
		break;

	case iimg::IBitmap::PF_FLOAT32:
		retVal = DoCropImageTemplate<float, double>(cellSize, useLinearInterpolation, cornerArray, inputBitmap, outputBitmap);
		break;

	case iimg::IBitmap::PF_FLOAT64:
		retVal = DoCropImageTemplate<double, double>(cellSize, useLinearInterpolation, cornerArray, inputBitmap, outputBitmap);
		break;

	default:
		if (resultConsumerPtr != NULL){
			ilog::CMessage* messagePtr = new ilog::CMessage(
						istd::IInformationProvider::IC_ERROR,
						0,
						QObject::tr("Input image format '%1' not supported").arg(iimg::CPixelFormatList::GetInstance().GetOptionName(pixelFormat)),
						QObject::tr("ImageCropDecalibrateProcessor"));
			resultConsumerPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(messagePtr));
		}
		return false;
	}

	return retVal;
}


bool CImageCropDecalibrateProcessorComp::CalcCalibration(
			const i2d::CRectangle& sourceAoi,
			icalib::CAffineCalibration2d& outputCalib,
			istd::CIndex2d* outputImageSizePtr,
			int orientationMode)
{
	istd::CIndex2d outputImageSize;
	if (outputImageSizePtr != NULL){
		outputImageSize = *outputImageSizePtr;
	}
	else{
		if (!CalcOutputImageSize(sourceAoi, outputImageSize)){
			return false;
		}
	}

	bool flipHorizontal = false;
	bool flipVertical = false;
	if ((orientationMode == OM_NO_REFLEXION) || (orientationMode == OM_VISUAL)){
		const i2d::ICalibration2d* calibrationPtr = sourceAoi.GetCalibration();
		if (calibrationPtr != NULL){
			i2d::CAffine2d approxTransform;
			if (calibrationPtr->GetLocalTransform(i2d::CVector2d::GetZero(), approxTransform)){
				if (orientationMode == OM_VISUAL){
					if (approxTransform.GetDeformMatrix().GetAxisX().GetX() < 0){
						flipHorizontal = true;
						flipVertical = true;
					}
				}

				if (approxTransform.GetDeformMatrix().GetDet() < 0){
					flipVertical = !flipVertical;
				}
			}
		}
	}

	double scaleX = outputImageSize.GetX() / sourceAoi.GetWidth();
	double scaleY = outputImageSize.GetY() / sourceAoi.GetHeight();

	i2d::CMatrix2d scaleMatrix(flipHorizontal? -scaleX: scaleX, 0, 0, flipVertical? -scaleY: scaleY);

	i2d::CAffine2d transformation;
	if (flipHorizontal){
		if (flipVertical){
			transformation.SetTranslation(-scaleMatrix.GetMultiplied(sourceAoi.GetRightBottom()));
		}
		else{
			transformation.SetTranslation(-scaleMatrix.GetMultiplied(sourceAoi.GetRightTop()));
		}
	}
	else{
		if (flipVertical){
			transformation.SetTranslation(-scaleMatrix.GetMultiplied(sourceAoi.GetLeftBottom()));
		}
		else{
			transformation.SetTranslation(-scaleMatrix.GetMultiplied(sourceAoi.GetLeftTop()));
		}
	}
	transformation.SetDeformMatrix(scaleMatrix);

	outputCalib.SetTransformation(transformation);

	return true;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CImageCropDecalibrateProcessorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	iprm::TParamsPtr<i2d::CRectangle> aoiParamPtr(paramsPtr, m_aoiParamIdAttrPtr, m_defaultAoiCompPtr, true);
	if (!aoiParamPtr.IsValid()){
		SendErrorMessage(MI_BAD_PARAMS, QObject::tr("No AOI rectangle found"));
		return TS_INVALID;
	}

	int orientationMode = OM_SIMPLE;
	iprm::TParamsPtr<iprm::ISelectionParam> orientationModePtr(paramsPtr, m_orientationModeParamIdAttrPtr, m_defaultOrientationModeCompPtr, false);
	if (orientationModePtr.IsValid()){
		orientationMode = orientationModePtr->GetSelectedOptionIndex();
	}

	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == NULL){
		icalib::CAffineCalibration2d* outputCalibPtr = dynamic_cast<icalib::CAffineCalibration2d*>(outputPtr);
		if (outputCalibPtr != NULL){
			return CalcCalibration(*aoiParamPtr, *outputCalibPtr, NULL, orientationMode)? TS_OK: TS_INVALID;
		}

		return TS_INVALID;
	}

	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	if (inputBitmapPtr == NULL){
		return TS_INVALID;
	}

	int interpolationMode = IM_LINEAR;
	iprm::TParamsPtr<iprm::ISelectionParam> interpolationModePtr(paramsPtr, m_interpolationModeParamIdAttrPtr, m_defaultInterpolationModeCompPtr, false);
	if (interpolationModePtr.IsValid()){
		interpolationMode = interpolationModePtr->GetSelectedOptionIndex();
	}

	return CropImage(
				*aoiParamPtr,
				*m_cellSizeAttrPtr,
				*inputBitmapPtr,
				*outputBitmapPtr,
				NULL,
				interpolationMode,
				orientationMode,
				GetLogPtr())? TS_OK: TS_INVALID;
}


// protected methods

bool CImageCropDecalibrateProcessorComp::CalcOutputImageSize(
			const i2d::CRectangle& sourceAoi,
			istd::CIndex2d& result)
{
	const i2d::ICalibration2d* calibrationPtr = sourceAoi.GetCalibration();

	double pixelSize = 1;

	i2d::CVector2d inputLeftTopPos = sourceAoi.GetLeftTop();
	i2d::CVector2d inputRightTopPos = sourceAoi.GetRightTop();
	i2d::CVector2d inputLeftBottomPos = sourceAoi.GetLeftBottom();
	i2d::CVector2d inputRightBottomPos = sourceAoi.GetRightBottom();
	if (calibrationPtr != NULL){
		double leftDist = 0;
		double rightDist = 0;
		double topDist = 0;
		double bottomDist = 0;
		if (		calibrationPtr->GetDistance(inputLeftTopPos, inputLeftBottomPos, leftDist) &&
					calibrationPtr->GetDistance(inputRightTopPos, inputRightBottomPos, rightDist) &&
					calibrationPtr->GetDistance(inputLeftTopPos, inputRightTopPos, topDist) &&
					calibrationPtr->GetDistance(inputLeftBottomPos, inputRightBottomPos, bottomDist)){
			pixelSize = 0.5 * (leftDist + rightDist + topDist + bottomDist) / (sourceAoi.GetWidth() + sourceAoi.GetHeight());
		}
	}

	result = istd::CIndex2d(qCeil(sourceAoi.GetWidth() * pixelSize),  qCeil(sourceAoi.GetHeight() * pixelSize));

	return true;
}


} //!namespace iipr


