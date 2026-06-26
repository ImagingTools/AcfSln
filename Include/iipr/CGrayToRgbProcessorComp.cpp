// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CGrayToRgbProcessorComp.h>


// Qt includes
#include <QtCore/QObject>

 // ACF includes
#include <istd/CChangeNotifier.h>
#include <ibase/CSize.h>


namespace iipr
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CGrayToRgbProcessorComp::DoProcessing(
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

bool CGrayToRgbProcessorComp::ConvertImage(
			const iimg::IBitmap& inputBitmap,
			iimg::IBitmap& outputBitmap) const
{
	if (inputBitmap.IsEmpty()){
		return true;
	}

	int inputFormat = inputBitmap.GetPixelFormat();
	if ((inputFormat == iimg::IBitmap::PF_RGB) || (inputFormat == iimg::IBitmap::PF_RGBA)){
		return outputBitmap.CopyFrom(inputBitmap);
	}
	else if (inputFormat != iimg::IBitmap::PF_GRAY){
		SendErrorMessage(0, QObject::tr("Image format must be Grayscale (8-Bit)"), "GrayToRgbProcessor");

		return false;
	}

	ibase::CSize imageSize = inputBitmap.GetImageSize();

	istd::CChangeNotifier resultNotifier(&outputBitmap);
	Q_UNUSED(resultNotifier);

	if (!outputBitmap.CreateBitmap(iimg::IBitmap::PF_RGB, imageSize)){
		return false;
	}

	for (int y = 0; y < imageSize.GetY(); ++y){
		quint8* inputImageLinePtr = (quint8*)inputBitmap.GetLinePtr(y);
		quint32* outputImageLinePtr = (quint32*)outputBitmap.GetLinePtr(y);

		for (int x = 0; x < imageSize.GetX(); ++x){
			quint8 value = *(inputImageLinePtr + x);

			outputImageLinePtr[x] = (value << 16) + (value << 8) + (value);
		}
	}

	const i2d::ICalibration2d* inputCalibrationPtr = inputBitmap.GetCalibration();
	if (inputCalibrationPtr != NULL){
		istd::TUniqueInterfacePtr<i2d::ICalibration2d> outputCalibrationPtr;
		outputCalibrationPtr.MoveCastedPtr(inputCalibrationPtr->CloneMe());

		outputBitmap.SetCalibration(outputCalibrationPtr.PopInterfacePtr(), true);
	}

	return true;
}


} // namespace iipr


