#include <iipr/CRingPatternGeneratorComp.h>


// ACF includes
#include <istd/CIndex2d.h>


// IACF includes
#include <iipr/IRingPatternGeneratorParams.h>


namespace iipr
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CRingPatternGeneratorComp::DoProcessing(
			const iprm::IParamsSet* /*paramsPtr*/,
			const istd::IPolymorphic* /*inputPtr*/,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/)
{
	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == nullptr){
		SendErrorMessage(0, "Output bitmap is not defined");

		return TS_INVALID;
	}
						
	int outputImageWidth = 512;
	int outputImageHeight = 512;

	if (!outputBitmapPtr->CreateBitmap(iimg::IBitmap::PF_GRAY, istd::CIndex2d(outputImageWidth, outputImageHeight))){
		SendErrorMessage(0, "Output bitmap coud not be created");

		return TS_INVALID;
	}

	int halfHeight  = outputImageHeight >> 1;
	int halfWidth  = outputImageWidth >> 1;
	double waveLength = 8.0;

	double patternFrequency = 1.0 / waveLength;
	double minFrequency = 1.0 / halfHeight;
	int patternGenerationMode = iipr::IRingPatternGeneratorParams::PT_DYNAMIC_WAVELENGTH;	

	double maxRadius = halfHeight - waveLength / 2.0; // margin
	for ( int y = 0; y < outputImageHeight; y++){
		quint8* imageLinePtr = (quint8*)outputBitmapPtr->GetLinePtr(y);
		for (int x = 0; x < outputImageWidth; x++){
			int dx = x - halfWidth;
			int dy = y - halfHeight;

			double radius = sqrt(double(dx * dx + dy * dy));
			if (radius > maxRadius){
				*(imageLinePtr + x) = 0;
				
				continue;
			}

			double usedFrequency = patternFrequency;
			if (patternGenerationMode == iipr::IRingPatternGeneratorParams::PT_DYNAMIC_WAVELENGTH){
				double relativeRadius = radius / maxRadius;

				usedFrequency = (1.0 - relativeRadius) * minFrequency + relativeRadius * patternFrequency;
			}

			double outputValue = qCos(radius * usedFrequency * I_2PI);
				
			*(imageLinePtr + x) = quint8(127 * outputValue + 128);
		}
	}
	
	return TS_OK;
}


} // namespace iipr


