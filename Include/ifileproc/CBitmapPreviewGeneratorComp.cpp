// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CBitmapPreviewGeneratorComp.h>


// ACF includes
#include <iprm/TParamsPtr.h>
#include <iimg/CBitmap.h>


namespace ifileproc
{


// public methods

// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CBitmapPreviewGeneratorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	const iimg::CBitmap* inputBitmapPtr = dynamic_cast<const iimg::CBitmap*>(inputPtr);
	iimg::CBitmap* outputBitmapPtr = dynamic_cast<iimg::CBitmap*>(outputPtr);
	if ((inputBitmapPtr == NULL) || (outputBitmapPtr == NULL)){
		return TS_INVALID;
	}

	istd::CIndex2d size(256, 256);

	iprm::TParamsPtr<i2d::CRectangle> previewRectParamPtr(paramsPtr, "PreviewRect");
	if (previewRectParamPtr.IsValid()){
		size.SetX(previewRectParamPtr->GetWidth());
		size.SetY(previewRectParamPtr->GetHeight());
	}

	outputBitmapPtr->CopyImageFrom(inputBitmapPtr->GetQImage().scaled(size.GetX(), size.GetY(), Qt::KeepAspectRatioByExpanding));

	return TS_OK;
}


} // namespace ifileproc


