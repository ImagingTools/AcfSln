#include <iipr/CImageFlipProcessorComp.h>


// Qt includes
#include <QtGui/QImage>

// ACF includes
#include <iimg/CBitmap.h>

#include <iprm/TParamsPtr.h>
#include <iprm/IEnableableParam.h>


namespace iipr
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CImageFlipProcessorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/)
{
	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	if (inputBitmapPtr == nullptr){
		return TS_INVALID;
	}

	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == nullptr){
		return TS_INVALID;
	}

	// check if postprocessing needed
	bool mirrorX = false, mirrorY = false;

	if (m_mirrorXParamIdAttrPtr.IsValid()){
		iprm::TParamsPtr<iprm::IEnableableParam> mirrorXParam(paramsPtr, *m_mirrorXParamIdAttrPtr);
		if (mirrorXParam.IsValid()){
			mirrorX = mirrorXParam->IsEnabled();
		}
	}

	if (m_mirrorYParamIdAttrPtr.IsValid()){
		iprm::TParamsPtr<iprm::IEnableableParam> mirrorYParam(paramsPtr, *m_mirrorYParamIdAttrPtr);
		if (mirrorYParam.IsValid()){
			mirrorY = mirrorYParam->IsEnabled();
		}
	}

	return ConvertImage(*inputBitmapPtr, *outputBitmapPtr, mirrorX, mirrorY) ? TS_OK : TS_INVALID;
}


// private methods

bool CImageFlipProcessorComp::ConvertImage(
			const iimg::IBitmap& inputBitmap,
			iimg::IBitmap& outputBitmap,
			bool mirrorX, bool mirrorY) const
{
	// TODO: Implement it more general and independent from QImage

	// simple (but inefficient) flip via QImage
	if (mirrorX || mirrorY){
		QImage tempImage;

		// check if we can take QImage directly
		const iimg::CBitmap* inputCBitmapPtr = dynamic_cast<const iimg::CBitmap*>(&inputBitmap);
		if (inputCBitmapPtr != nullptr){
			tempImage = inputCBitmapPtr->GetQImage().mirrored(mirrorX, mirrorY);
		}
		else{
			// else do this via copy...
			iimg::CBitmap tempBitmap;
			if (tempBitmap.CopyFrom(inputBitmap)){
				tempImage = tempBitmap.GetQImageRef().mirrored(mirrorX, mirrorY);
			}
		}

		// then put it back...
		iimg::CBitmap* outputCBitmapPtr = dynamic_cast<iimg::CBitmap*>(&outputBitmap);
		if (outputCBitmapPtr != nullptr){
			return outputCBitmapPtr->CopyImageFrom(tempImage);
		}
		else{
			iimg::CBitmap tempBitmap;
			if (tempBitmap.CopyImageFrom(tempImage)){
				return outputBitmap.CopyFrom(tempBitmap);
			}			
		}

		return false;
	}

	return outputBitmap.CopyFrom(inputBitmap);
}


} // namespace iipr


