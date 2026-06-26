// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef icam_CMemoryAcquisitionComp_included
#define icam_CMemoryAcquisitionComp_included


// ACF includes
#include <ifile/IFilePersistence.h>
#include <icomp/CComponentBase.h>
#include <ifile/IFileNameParam.h>
#include <iimg/CGeneralBitmap.h>
#include <iproc/TSyncProcessorWrap.h>

// ACF-Solutions includes
#include <icam/IBitmapAcquisition.h>


namespace icam
{


/**
	Bitmap acquisition from memory.
*/
class CMemoryAcquisitionComp:
			public icomp::CComponentBase,
			public iimg::CGeneralBitmap,
			virtual public iproc::TSyncProcessorWrap<IBitmapAcquisition>
{
public:
	typedef icomp::CComponentBase BaseClass;
	typedef iimg::CGeneralBitmap BaseClass2;

	I_BEGIN_COMPONENT(CMemoryAcquisitionComp);
		I_REGISTER_INTERFACE(IBitmapAcquisition);
		I_REGISTER_INTERFACE(iimg::IBitmap);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (icam::IBitmapAcquisition)
	virtual istd::CIndex2d GetBitmapSize(const iprm::IParamsSet* paramsPtr) const override;
};


} // namespace icam


#endif // !icam_CMemoryAcquisitionComp_included


