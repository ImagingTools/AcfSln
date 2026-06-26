// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CProcessedAcquisitionComp_included
#define iipr_CProcessedAcquisitionComp_included


// ACF includes
#include <iproc/TSyncProcessorCompBase.h>
#include <iimg/IBitmap.h>

// ACF-Solutions includes
#include <icam/IBitmapAcquisition.h>


namespace iipr
{


class CProcessedAcquisitionComp: public iproc::TSyncProcessorCompBase<icam::IBitmapAcquisition>
{
public:
	typedef iproc::TSyncProcessorCompBase<icam::IBitmapAcquisition> BaseClass;

	I_BEGIN_COMPONENT(CProcessedAcquisitionComp);
		I_ASSIGN(m_slaveAcquisitionCompPtr, "SlaveCamera", "Camera for real image acquisition", true, "SlaveCamera");
		I_ASSIGN(m_processorCompPtr, "ImageProcessor", "Image processor", false, "ImageProcessor");
		I_ASSIGN(m_bitmapFactCompPtr, "BitmapFactory", "Factory used for creation of the temporary image", true, "BitmapFactory");
	I_END_COMPONENT;

	// reimplemented (icam::IBitmapAcquisition)
	virtual istd::CIndex2d GetBitmapSize(const iprm::IParamsSet* paramsPtr) const override;

	// reimplemented iproc::TSyncProcessorWrap<icam::IBitmapAcquisition>
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual void InitProcessor(const iprm::IParamsSet* paramsPtr) override;

private:
	I_REF(icam::IBitmapAcquisition, m_slaveAcquisitionCompPtr);
	I_REF(iproc::IProcessor, m_processorCompPtr);
	I_FACT(iimg::IBitmap, m_bitmapFactCompPtr);
};


} // namespace iipr


#endif // !iipr_CProcessedAcquisitionComp_included


