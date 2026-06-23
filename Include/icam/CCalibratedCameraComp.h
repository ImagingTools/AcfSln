// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// ACF includes
#include <iimg/IBitmap.h>
#include <icam/CCameraDelegatorComp.h>


namespace icam
{


/**
	Camera provided bitmaps with integrated calibration information.
*/
class CCalibratedCameraComp: public icam::CCameraDelegatorComp
{
public:
	typedef icam::CCameraDelegatorComp BaseClass;

	I_BEGIN_COMPONENT(CCalibratedCameraComp);
		I_ASSIGN(m_scaleParamIdAttrPtr, "ScaleParamId", "ID of the scale parameter in the parameter set", true, "Scale");
	I_END_COMPONENT;

	static bool ReadImageResolution(const iimg::IBitmap& bitmap, double& resolutionX, double& resolutionY);
	static bool WriteImageResolution(iimg::IBitmap& bitmap, double resolutionX, double resolutionY);

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

private:
	I_ATTR(QByteArray, m_scaleParamIdAttrPtr);
};


} // namespace icam


