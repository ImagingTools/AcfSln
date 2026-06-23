// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CCheckerboardPatternGeneratorComp.h>


// ACF includes
#include <istd/CIndex2d.h>


namespace iipr
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CCheckerboardPatternGeneratorComp::DoProcessing(
			const iprm::IParamsSet* /*paramsPtr*/,
			const istd::IPolymorphic* /*inputPtr*/,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == NULL){
		SendErrorMessage(0, "Output bitmap is not defined");

		return TS_INVALID;
	}
						
	int outputImageWidth = 512;
	int outputImageHeight = 512;

	if (!outputBitmapPtr->CreateBitmap(iimg::IBitmap::PF_GRAY, istd::CIndex2d(outputImageWidth, outputImageHeight))){
		SendErrorMessage(0, "Output bitmap coud not be created");

		return TS_INVALID;
	}

	int patternSize = 32;

	for ( int y = 0; y < outputImageHeight; y++){
		quint8* imageLinePtr = (quint8*)outputBitmapPtr->GetLinePtr(y);

		int yIndex = y / patternSize;
		for (int x = 0; x < outputImageWidth; x += patternSize){
	
			int xIndex = x / patternSize;
			qint8 currentPatternValue = 255 * (xIndex % 2);

			if (yIndex % 2){
				currentPatternValue = 255 - currentPatternValue;
			}
			
			memset(imageLinePtr + x, currentPatternValue, patternSize);
		}
	}
	
	return TS_OK;
}


} // namespace iipr


